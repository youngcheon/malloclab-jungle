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
    "ateam",
    /* First member's full name */
    "Youngcheon Kim",
    /* First member's email address */
    "kimyoungcheon97@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 매크로정의 */
#define WSIZE 4 //word and header footer사이즈를 byte로
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define MAX(x,y) ((x)>(y)? (x):(y)) //max

/* address p위치에 words를 read와 write 를 한다.*/
#define GET(p) (*(unsigned int*)(p))
#define PUT(p,val) (*(unsigned int*)(p)=(int)(val))

// adress p위치로부터 size를 읽고 field를 할당
// GETSIZE 
// ~는 역수니까 11111000이 됨.
// 비트연산하면 000 앞부분만 가져올 수 있는데, 블록 사이즈만 가져오겠다는 뜻이다.
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1) //헤더에서 가용여부만 가져오겠다는 뜻

/* given block ptr bp, header와 footer의 주소를 계산*/
#define HDRP(bp) ((char*)(bp) * GET_SIZE((char*)(bp)-WSIZE))
// HDRP => bp가 어디있던 상관없이 WSIZE 앞에 위치한다.
#define FTRP(bp) ((char*)(bp) + GET_SIZE((HDRP(bp)) - DSIZE))
// FTRP => 헤더의 끝 지점부터 블록사이즈만큼 더한 다음 word를 두번 빼는게
// footer의 시작 위치가 된다.

/* given block ptr bp, 이전 블록과 다음 블록의 주소를 계산 */
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))
/* The only global variable is a pointer to the first block */
static char *heap_listp;
static void *extend_heap(size_t);
static void *coalesce(void *);
static void *find_fit(size_t);
static void place(void *, size_t)

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1){
        return -1;
    }
    PUT(heap_listp, 0);
    PUT(extend  _heap + (1*WSIZE), PACK(DSIZE, 1));
    PUT(extend_heap + (2*WSIZE), PACK(DSIZE, 1));
    PUT(extend_heap + (3*WSIZE), PACK(DSIZE, 1));
    heap_listp += (2*WSIZE);
    //Extend the empty heap with a free block of CHUNKSIZE bytes 
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL){
        return -1;
    }
    return 0;
}

static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    //Allocate an even number of words to maintain alignment
    size = (word%2) ? (words+1)*WSIZE : words*WSIZE;
    if ((long)(bp=mem_sbrk(size))==-1){
        return NULL;
    }
    // Initialie free block header/footer and the epilogue heaer
    PUT(HDRP(bp), PACK(size,0));
    PUT(HDRP(bp), PACK(size,0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));

    return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














