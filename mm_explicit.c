/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "team 3",
    /* First member's full name */
    "yeopto",
    /* First member's email address */
    "gunyeop0530@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*
    매크로
*/

/* 기본 단위인 word, double word, 새로 할당받는 힙의 크기 CHUNKSIZE를 정의한다 */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define MINIMUM     16      /* Initial Prologue block size, header, footer, PREC, SUCC */
#define CHUNKSIZE   (1<<12) /* Extend heap by this amount : 4096bytes -> 4kib */

#define MAX(x, y) ((x) > (y) ? (x) : (y))  // 최댓값 구하는 함수 매크로

/* header 및 footer 값(size + allocated) 리턴 */
#define PACK(size, alloc)   ((size) | (alloc))   

/* 주소 p에서의 word를 읽어오거나 쓰는 함수 */
#define GET(p)          (*(unsigned int*)(p))
#define PUT(p, val)     (*(unsigned int*)(p) = (val))

/* header or footer에서 블록의 size, allocated field를 읽어온다 */
#define GET_SIZE(p)     (GET(p) & ~0x7) 
#define GET_ALLOC(p)    (GET(p) & 0x1)    

/* 블록 포인터 bp를 인자로 받아 블록의 header와 footer의 주소를 반환한다 */
#define HDRP(bp)    ((char*)(bp) - WSIZE) 
#define FTRP(bp)    ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* 블록 포인터 bp를 인자로 받아 이후, 이전 블록의 주소를 리턴한다 */
#define NEXT_BLKP(bp)   ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE))) // (char*)(bp) + GET_SIZE(지금 블록의 헤더값)
#define PREV_BLKP(bp)   ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE))) // (char*)(bp) - GET_SIZE(이전 블록의 풋터값)

/* free list 상에서의 이전, 이후 블록의 포인터를 리턴한다. */
#define PREC_FREEP(bp)  (*(void**)(bp))             // 이전 블록의 bp
#define SUCC_FREEP(bp)  (*(void**)(bp + WSIZE))     // 이후 블록의 bp

static char* heap_listp = NULL; // heap공간 첫 주소를 가리키는 정적 전역 변수 설정
static char* free_listp = NULL; // free list의 맨 첫 블록을 가리키는 포인터

static void* extend_heap(size_t words);
static void* coalesce(void* bp);
static void* find_fit(size_t asize);
static void place(void* bp, size_t newsize);

int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *bp);
void *mm_realloc(void *ptr, size_t size);

int mm_init(void) {
    /* 메모리에서 6words를 가져오고 이걸로 빈 가용 리스트 초기화 */
    /* padding, prol_header, prol_footer, PREC, SUCC, epil_header */
    if ((heap_listp = mem_sbrk(6 * WSIZE)) == (void*)-1)
        return -1;

    PUT(heap_listp, 0);  // Alignment padding. 더블 워드 경계로 정렬된 미사용 패딩.
    PUT(heap_listp + (1 * WSIZE), PACK(MINIMUM, 1));  // prologue header
    PUT(heap_listp + (2 * WSIZE), NULL);  // prologue block안의 PREC 포인터 NULL로 초기화
    PUT(heap_listp + (3 * WSIZE), NULL);  // prologue block안의 SUCC 포인터 NULL로 초기화
    PUT(heap_listp + (4 * WSIZE), PACK(MINIMUM, 1));  // prologue footer
    PUT(heap_listp + (5 * WSIZE), PACK(0, 1));      // epliogue header

    free_listp = heap_listp + 2 * WSIZE; // free_listp를 탐색의 시작점으로 둔다. 


    /* 그 후 CHUNKSIZE만큼 힙을 확장해 초기 가용 블록을 생성한다. */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) {
        return -1; // 실패하면 -1 리턴
    } 

    return 0;
}

// extend_heap : 워드 단위 메모리를 인자로 받아 힙을 늘려준다.
static void* extend_heap(size_t words){ // 워드 단위로 받는다.
    char* bp;
    size_t size;
    
    /* 더블 워드 정렬에 따라 메모리를 mem_sbrk 함수를 이용해 할당받는다. */
    size = (words % 2) ? (words + 1) * WSIZE : (words) * WSIZE; // size를 짝수 word && byte 형태로 만든다.
    if ((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
    } /* mem_brk는 heap의 첫 주소, mem_sbrk 함수는 그 주소값을 기억하기 위해 임시변수 old_brk를 만들어주고
    전역변수인 mem_brk를 늘려주고 리턴은 old_brk를 반환 */
    
    PUT(HDRP(bp), PACK(size, 0)); // 가용 블록 헤더 만들기
    PUT(FTRP(bp), PACK(size, 0)); // 가용 블록 푸터 만들기
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));  // 에필로그 헤더 위치 변경

    /* 세팅해주고 오류 단편화가 발생할 수 있으니까 free한 가용공간 
    합칠거 있으면 합쳐주기 위해 return을 coalesc로*/
    return coalesce(bp);
}

void *mm_malloc(size_t size) {
    size_t asize;
    size_t extendsize;
    char* bp;

    if (size == 0) return NULL;

    if (size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        /* 말록할당은 payload부터. 필요한 공간은 
        헤더푸터를 포함해야되기에 더블워드(DSIZE)하나는 필수
        만약 size가 9라면 16만큼만 할당하면 되니까 더블 워드 두개가 필요한건데
        9에서 DSIZE - 1 만큼 더해주면 17이겠지? 8로나누면 2고 더블워드 두개는 즉 16이 된다.
        그럼 총 24바이트가 asize가 되어야하는 것.
        */
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }  

    // fit에 맞는 free리스트 찾기
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize); // 필요하다면 분할하여 할당한다.
        return bp;
    }

    // 만약 맞는 크기의 가용 블록이 없다면 새로 힙을 늘려서 새 힙에 메모리를 할당한다.
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) {
        return NULL;
    } 
    place(bp, asize);
    return bp;
}

// coalesce(bp) : 해당 가용 블록을 앞뒤 가용 블록과 연결하고 연결된 가용 블록의 주소를 리턴한다.
static void* coalesce(void* bp) {
    // 직전 블록의 footer, 직후 블록의 header를 보고 가용 여부를 확인.
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));  // 직전 블록 가용 여부
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));  // 직후 블록 가용 여부
    size_t size = GET_SIZE(HDRP(bp)); // 현재 사이즈

    // case 1 : 직전, 직후 블록이 모두 할당 -> 해당 블록만 free list에 넣어주면 된다.

    // case 2 : 직전 블록 할당, 직후 블록 가용
    if (prev_alloc && !next_alloc) {
        removeBlock(NEXT_BLKP(bp));    // free 상태였던 직후 블록을 free list에서 제거한다.
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    } else if (!prev_alloc && next_alloc) { // case 3 : 직전 블록 가용, 직후 블록 할당
        removeBlock(PREV_BLKP(bp));    // 직전 블록을 free list에서 제거한다.
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp); 
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));  
    } else if (!prev_alloc && !next_alloc) { // case 4 : 직전, 직후 블록 모두 가용
        removeBlock(PREV_BLKP(bp));
        removeBlock(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));  
        PUT(FTRP(bp), PACK(size, 0));  
    }
    
    // 연결된 새 가용 블록을 free list에 추가한다.
    putFreeBlock(bp);

    return bp;
}


// first_fit : 힙의 맨 처음부터 탐색하여 요구하는 메모리 공간보다 큰 가용 블록의 주소를 반환한다.
static void* find_fit(size_t asize){
    /* First-fit */
    void* bp;

    // free list의 맨 뒤는 프롤로그 블록이다. Free list에서 유일하게 할당된 블록이므로 얘를 만나면 탐색 종료.
    for (bp = free_listp; GET_ALLOC(HDRP(bp)) != 1; bp = SUCC_FREEP(bp)){
        if(asize <= GET_SIZE(HDRP(bp))){
            return bp;
        }
    }
    // 못 찾으면 NULL을 리턴한다.
    return NULL;
}

// place(bp, size): 요구 메모리를 할당할 수 있는 가용 블록을 할당한다. 이 때 분할이 가능하면 분할한다.
static void place(void* bp, size_t asize){
    // 현재 할당할 수 있는 후보 가용 블록의 주소
    size_t csize = GET_SIZE(HDRP(bp));

    // 할당될 블록이므로 free list에서 없애준다.
    removeBlock(bp);

    // 분할이 가능한 경우
    if ((csize - asize) >= (2*DSIZE)) {
        // 앞의 블록은 할당 블록으로
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        // 뒤의 블록은 가용 블록으로 분할한다.
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
        // free list 첫번째에 분할된 블럭을 넣는다.
        putFreeBlock(bp);
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

// putFreeBlock(bp) : 새로 반환되거나 생성된 가용 블록을 free list의 첫 부분에 넣는다.
void putFreeBlock(void* bp) { 
    SUCC_FREEP(bp) = free_listp;
    PREC_FREEP(bp) = NULL;
    PREC_FREEP(free_listp) = bp;
    free_listp = bp;
}

// removeBlock(bp) : 할당되거나 연결되는 가용 블록을 free list에서 없앤다.
void removeBlock(void *bp) {
    // free list의 첫번째 블록을 없앨 때
    if (bp == free_listp) {
        PREC_FREEP(SUCC_FREEP(bp)) = NULL;
        free_listp = SUCC_FREEP(bp);
    } else { // free list 안에서 없앨 때
        SUCC_FREEP(PREC_FREEP(bp)) = SUCC_FREEP(bp);
        PREC_FREEP(SUCC_FREEP(bp)) = PREC_FREEP(bp);
    }
}

// free되면 가용블록끼리 합쳐주고 헤더, 푸터 갱신
void mm_free(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0)); // 헤더 갱신
    PUT(FTRP(bp), PACK(size, 0)); // 푸터 갱신
    coalesce(bp); // 합치기
}

// realloc은 말록으로 새로 할당하고 그 전에 있는 것은 프리해줌
void *mm_realloc(void *ptr, size_t size) {
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}