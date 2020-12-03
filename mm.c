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
    /* bu username : eg. jappavoo */
    "en",
    /* full name : eg. jonathan appavoo */
    "Eamon Niknafs",
    /* email address : jappavoo@bu.edu */
    "en@bu.edu",
    "",
    ""
};
/* header/footer size (bytes) */
#define HFSIZE 4       

/* double word size (bytes) */
#define DWORD 8

/* Default size to extend heap (bytes) */
#define CHUNKSIZE (1<<12)

/* Combines the size and allocated bit into a word (for the header/footer) */
#define HF(size, alloc) ((size) | (alloc))

/* Read and write a word at address a */
#define READ(a) (*(unsigned int *)(a))
#define WRITE(a, val) (*(unsigned int *)(a) = (val))

/* Read the size and allocation status of header/footer block at address a */
#define GET_SIZE(a) (READ(a) & ~0x7)
#define GET_ALLOC(a) (READ(a) & 0x1)

/* Calculate address of given block's header/footer */
#define HEAD(ptr) ((char *)(ptr) - WSIZE)
#define FOOT(ptr) ((char *)(ptr) + GET_SIZE(HEAD(ptr)) - DSIZE)

/* Calculate address of next/previous blocks */
#define NEXT(ptr) ((char *)(ptr) + GET_SIZE(((char *)(ptr) - HFSIZE)))
#define PREV(ptr) ((char *)(ptr) - GET_SIZE(((char *)(ptr) - DWORD)))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

static char *trav; //trav for next fit

/************************
// #define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
/************************

/* 
 * mm_init - initialize the malloc package.
 *  - creates a free heap list of size 16 bytes
 *  - adds a start header/footer
 *  - adds an end header
 *  - moves the pointer to the start of the free space
 *  - extends the heap by CHUNKSIZE bytes
 */
int mm_init(void)
{   
    /* Create the initial free heap list */
    heapL = mem_sbrk(2*DWORD);
    if (heapL == (void *)-1) return -1;
    WRITE(heapL, 0); //padding
    WRITE(heapL + (1*HFSIZE), HF(DWORD, 1)); //start header
    WRITE(heapL + (2*HFSIZE), HF(DWORD, 1)); //start footer
    WRITE(heapL + (3*HFSIZE), HF(0, 1)); //end header 
    heapL += (2*HFSIZE); //move heapL pointer after start header/footer

    /* Extend the empty heap list CHUNKSIZE bytes */
    if (grow_heap(CHUNKSIZE/HFSIZE) == NULL) return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block of (at least) size bytes
 * - searches for fit of size bytes
 * - if found, puts the block
 * - if not found, grows heap then puts the block
 */
void *mm_malloc(size_t size)
{
    if (heapL == 0) mm_init(); //if no heap list, init
    if (size == 0) return NULL; //if request is useless, return NULL
    ptr = fit(ALIGN(size));
    if (ptr != NULL){ //if fit
        put(ptr, ALIGN(size)); //puts block
        return ptr;
    }
    else //only runs of no fit
    {
        growsize = (ALIGN(size)>CHUNKSIZE) ? ALIGN(size) : CHUNKSIZE; //grows heap by CHUNKSIZE or block size, whichever is larger
        if ((ptr = grow_heap(growsize/HFSIZE)) == NULL) return NULL;
        put(ptr, ALIGN(size)); //puts block
        return ptr;
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

/* 
 * *grow_heap - grows the heap size.
 *  - ensures alignment
 *  - adds header/footer for new free block
 *  - moves end header to the end of the grown heap
 *  - coalesces
 */
static void *grow_heap(size_t size) 
{
    char *ptr = mem_sbrk(size); //set pointer to start of grown block
    if ((long)ptr == -1)  return NULL;

    /* Initialize new block's header/footer and end header */
    PUT(HEAD(ptr), HF(ALLIGN(size)*HFSIZE, 0));
    PUT(FOOT(ptr), HF(ALLIGN(size)*HFSIZE, 0));
    PUT(HEAD(NEXT(ptr)), HF(0, 1)); //new end header

    //return coalesce(ptr);  TO BE IMPLEMENTED
}

/* 
 * fit - Find a fit for a block with size bytes
 * - uses next fit method to search for a fit
 * - first traverses the next part of the heap (not already traversed)
 * - next, it traverses the first half
 * - if still no fit, returns NULL, if any then returns pointer to start of fit
 */
static void *fit(size_t size)
{
    char *last_trav = trav; //next fit
    /* search from the trav to the end of list */
    for (; GET_SIZE(HEAD(trav)) > 0; trav = NEXT(trav)){
        if (!GET_ALLOC(HEAD(trav)) && (size <= GET_SIZE(HEAD(trav)))) return trav;
    }
    /* search from start of list to last trav */
    for (trav = heapL; trav < last_trav; rover = NEXT(trav)){
        if (!GET_ALLOC(HEAD(trav)) && (size <= GET_SIZE(HEAD(trav)))) return trav;
    }
    return NULL; //no fit
}

/* 
 * put - Puts size byte block at the free block at ptr, splitting
 * if required.
 */
static void put(void *ptr, size_t size)
{
    size_t free_size = GET_SIZE(HEAD(ptr));   

    if ((free_size - size) >= (2*DWORD)) { 
        WRITE(HEAD(ptr), HF(size, 1));
        WRITE(FOOT(ptr), HF(size, 1));
        ptr = NEXT(ptr);
        WRITE(HEAD(ptr), HF(free_size-size, 0));
        WRITE(FOOT(ptr), HF(free_size-size, 0));
    }
    else { 
        WRITE(HEAD(ptr), HF(free_size, 1));
        WRITE(FOOT(ptr), HF(free_size, 1));
    }
}
//Used example in CSAPP chapter 9.9 for reference.