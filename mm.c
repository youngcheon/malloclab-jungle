#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


team_t team = {
    /* Team name */
    "SWJungle",
    /* First member's full name */
    "Kimyoungchoen",
    /* First member's email address */
    "kimyoungcheon97@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4 // word and header footer 사이즈를 byte로. 
#define DSIZE 8 // double word size를 byte로
#define CHUNKSIZE (1<<12) // heap을 늘릴 때 얼만큼 늘릴거냐? 

#define MAX(x,y) ((x)>(y)? (x) : (y)) //MAX 구현

// size를 pack하고 개별 word 안의 bit를 할당 (size와 alloc을 비트연산), 헤더에서 써야하기 때문에 만듬.
#define PACK(size,alloc) ((size)| (alloc)) 
// alloc : 가용여부 (ex. 000) 
// size : block size를 의미
// 둘을 합치면 온전한 주소가 나옴

/* address p위치에 words를 read와 write를 한다. */
#define GET(p) (*(unsigned int*)(p))
// 포인터를 써서 p를 참조한다. 
// 주소와 값을 알 수 있다. 
// 다른 블록을 가리키거나 이동할 때 쓸 수 있다. 
#define PUT(p,val) (*(unsigned int*)(p)=(int)(val)) 
// 블록의 주소를 담는다. 

// address p위치로부터 size를 읽고 field를 할당
#define GET_SIZE(p) (GET(p) & ~0x7) 
// '~'은 역수니까 11111000이 됨. 비트 연산하면 000 앞에거만 가져올 수 있음. 
// 즉, 헤더에서 블록 size만 가져오겠다.
#define GET_ALLOC(p) (GET(p) & 0x1) 
// 00000001이 됨. //헤더에서 가용여부만 가져오겠다.

/* given block ptr bp, header와 footer의 주소를 계산*/
#define HDRP(bp) ((char*)(bp) - WSIZE) 
// bp가 어디에있던 상관없이 WSIZE 앞에 위치한다.
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) 
// 헤더의 끝 지점부터 GET SIZE(블록의 사이즈)만큼 더한 다음 
// word를 2번빼는게(그 뒤 블록의 헤드에서 앞으로 2번) footer의 시작 위치가 된다.

/* 이전 블록과 다음 블록의 주소를 계산*/
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp)-WSIZE))) 
// 그 다음 블록의 bp 위치로 이동한다.(bp에서 해당 블록의 크기만큼 이동 -> 그 다음 블록의 헤더 뒤로 감.)
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE))) 
// 그 전 블록의 bp위치로 이동.(이전 블록 footer로 이동하면 그 전 블록의 사이즈를 알 수 있으니 그만큼 그 전으로 이동.)
static char *heap_listp;
static void *extend_heap(size_t);
static void *coalesce(void *);
static void *find_fit(size_t);
static void place(void *, size_t);

/* 
 * mm_init - initialize the malloc package.
 */
// 할당기를 초기화하고 성공하면 0, 실패하면 -1을 리턴한다. 
// 가용리스트의 프롤로그헤더, 푸터, 에필로그 헤더를 만듬
int mm_init(void)
{
    /* 빈 heap*/
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
    // old brk에서 4만큼 늘려서 mem brk로 늘림
        return -1;
    PUT(heap_listp, 0); //padding생성
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); //prologue header
    //할당을(1) 할건데 8만큼 줄거다(DSIZE). -> 1 WSIZE 늘어난 시점부터 PACK에서 나온 사이즈를 줄거다.)
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); //prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2 * WSIZE);
    //prologue header와 footer 사이로 포인터를 옮긴다. header 뒤 위치
    //extend heap을 통해 한번 heap을 늘려줌.
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

// extend_heap의 용도
// 힙이 초기화될때 앞으로 블록을 넣기 위해 사이즈를 한번 늘리는것
// malloc을 통해 블록을 넣을 영역을 알아봤지만 적당한 맞춤 영역을 찾지 못해 늘리는 것

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* alignment 유지를 위해 짝수 개수의 words를 allocate */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    // sbrk로 size로 늘려서 long 형으로 반환한다.(한번 쫙 미리 늘려놓는 것) 
    // mem_sbrk가 반환되면 bp는 현재 만들어진 블록의 끝에 가있음.
    /* free block 헤더와 푸터를 init하고 epilogue 헤더를 init */
    PUT(HDRP(bp), PACK(size, 0));  // free block header 생성, 
    PUT(FTRP(bp), PACK(size, 0)); //free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); 
    // 블록 추가했으니 epilogue header을 새롭게 위치 조정해줌
    // 처음 세팅의 의미 = bp를 헤더에서 읽은 사이즈만큼 이동하고, 앞으로 한칸 간다.
    // 그 위치가 결국 늘린 블록 끝에서 한칸 간거라 거기가 epilogue header 위치
    // 만약 prev block이 free였다면, 합쳐라
    return coalesce(bp);
}

/* 
 * mm_malloc -  데이터 할당 함수, 가용리스트에서 블록 할당하기
 */
void *mm_malloc(size_t size)
{
    size_t asize;      /* 블록 사이즈 조정 */
    size_t extendsize; /* heap에 맞는 fit이 없으면 확장하기 위한 size */
    char *bp;

    /* 거짓된 요청 무시 */
    if (size == 0){
        return NULL;
    }

    /* overhead, alignment 요청 포함해서 블록 사이즈 조정 */
    if (size <= DSIZE){
        //헤더와 푸터를 포함해서 블록 사이즈를 다시 조정해야하니까 DSIZE의 두배를 준다.
        asize = 2 * DSIZE;
    }else{
        //size보다 클때, 블록이 가질 수 있는 크기중에 최적화된 크기로 재조정
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }
    /* fit에 맞는 free리스트를 찾는다. */
    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp; //place마친 블록의 위치를 리턴
    }

    /* fit맞는게 없으면 메모리를 더 가져와 block을 리턴시킨다. */
    extendsize = MAX(asize, CHUNKSIZE); // asize와 첫 세팅값중에 큰걸로
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL){
        return NULL;
    }
    place(bp, asize); //확장된 상태에서 asize를 넣어라.
    return bp;
    //할당기가 요청한 크기를 조정하면 할당할 가용블록이 가용리스트에 있는지 탐색한다.(find_fit)
    //맞는 블록을 찾으면 요청한 블록을 배치한다 (place)
    // 필요하면 기존블록을 분할한다 -> 맞는 블록을 찾지못하면 힙을 늘리고 다시 배치
}

/*
 * mm_free - 블록을 반환하고 경계태그 연결 사용
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp)); //얼만큼 free를 해야 하는지
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    //가용상태로 만들기.
    coalesce(bp);
}


//블록 연결하는 함수 
// 힙을 확장하면서 가용블록이 생긴다. 가용 블록이 생기면, 생기는 시점에 현재 블록 주변에도
// 가용 블록이 있는지 살피고 합칠 수 있는 블록은 합쳐야 한다.
// 합치지 않으면 작은 가용 블록들로 쪼개져 있는 경우에는 작은 블록들의 합이 충분히 커서
// 큰 메모리가 들어갈 수 있어보이지만, 구역이 나뉘어져 있어서 못들어가는 경우가 
// 생기기 때문이다. -> 오류 단편화 (false fragmentation)
// extend_heap, free 함수에서 쓰임
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    // 그전 블록으로 가서 블록의 가용여부 확인 
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    // 다음 블록으로 가서 블록의 가용여부 확인
    size_t size = GET_SIZE(HDRP(bp)); // 현재 블록 사이즈 확인


    /* CASE 1*/
    // 이전과 다음 블록이 모두 할당 되어있는 경우, 현재 블록의 상태는 할당->가용으로 변경
    if (prev_alloc && next_alloc){ 
        return bp; //이미 free에서 가용이 되어있으니 여기선 따로 free할 필요없다.
    }
    /* CASE 2*/
    // 이전 블록은 할당상태, 다음 블록은 가용상태.
    // 현재 블록은 다음 블록과 통합됨.
    else if (prev_alloc && !next_alloc){ 
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))); 
        // 다음블록의 헤더를 보고 그 블록의 크기만큼 현재 블록의 사이즈에 추가함.
        PUT(HDRP(bp), PACK(size, 0)); //헤더갱신
        PUT(FTRP(bp), PACK(size, 0)); //푸터갱신
        // 헤더갱신하면서 푸터에도 똑같은 정보가 들어가기 때문에
        // 이동할 필요없이 그냥 갱신해주면됨.
    }
    /* CASE 3 */
    // 이전 블록 가용, 다음 블록 할당 상태. 
    else if (!prev_alloc && next_alloc)
    { 
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0)); //푸터에 먼저 조정하려는 크기로 상태 변경
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        // 현재 헤더에서 그 앞블록의 헤더 위치로 이동한 다음에, 조정한 size를 넣는다.
        bp = PREV_BLKP(bp);
        //bp를 그 앞블록의 헤더로 이동 (늘린 블록의 헤더니까)
    }
    /* Case 4 */
    //이전 블록과 다음 블록 모두 가용상태.
    //3개의 블록을 하나로 합쳐야함
    else
    { 
        // 사이즈 늘리기
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        // 앞블록으로 가서 헤더 갱신
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        // 앞블록으로 가서 푸터 갱신 (앞블록이든 뒷블록이든 헤더랑 똑같기때문에 상관없음)
        PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0));
        // 헤더는 그 전 블록으로 이동.
        bp = PREV_BLKP(bp);
    }
    // 적용된bp리턴
    // bp는 항상 블록의 헤더 뒤에 위치하는게 좋기 때문에 연결이 끝나면 bp는 블록의 헤더에
    // 위치해야한다.
    return bp;
}

/*
 * mm_realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    copySize = GET_SIZE(HDRP(oldptr));

    if (size < copySize)
    {
        copySize = size;
    }

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}


//find fit
//가용리스트에 적절한 블록을 찾는 함수이다. 탐색은 크게 3가지 방법으로 나뉜다.
//first-fit => 가용블록리스트를 처음부터 검색해서 크기가 맞는 첫번째 가용블록을 선택.
//next-fit => 검색을 리스트의 처음에서 시작하는 대신, 이전 검색이 종료된 지점에서 검색시작
//best-fit => 모든 가용블록을 검사해서 크기가 맞는 가장 작은 블록을 선택

//first-fit으로 구현함
static void *find_fit(size_t asize)
{
    void *bp;
    // 위치를 처음세팅했던 위치 (heap_listp)로 잡으면 
    // 처음부터 모든 블록을 다 탐색 가능
    // NEXT_BLKP로 다음 블록 이동
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        //for 문을 돌면서 넣을 수 있는 블록을 찾는다.
        //asize가 GET_SIZE이하면 넣을 수 있기 때문에
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            // If a fit is found, return the address the of block pointer
            return bp;
        }
    }

    return NULL; /* No fit 상태*/
}

//place
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp)); //현재 블록 사이즈
    if ((csize - asize) >= (2 * DSIZE))
    //현재 블록에서 asize만큼 넣어도 2*DSIZE만큼 남는가?
    //남으면 다른 data를 넣을 수 있다.
    {
        PUT(HDRP(bp), PACK(asize, 1));
        //헤더 위치에 asize만큼 넣고 1로 변환.
        PUT(FTRP(bp), PACK(asize, 1));
        //푸터도 동일
        bp = NEXT_BLKP(bp);
        // 다음 블록으로 이동해서 bp 갱신

        //나머지 블록은 다 가용할수 있다 라는걸 다음 헤더에 표시
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    }
    else
    {
        // 추가 데이터 넣을 공간이 없다면
        // 혼자만 들어감
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}