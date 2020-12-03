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

/* Read the size and allocation status of block at address a */
#define GET_SIZE(a) (READ(a) & ~0x7)
#define GET_ALLOC(a) (READ(a) & 0x1)

/* Calculate address of given block's header/footer */
#define HEAD(ptr) ((char *)(ptr) - WSIZE)
#define FOOT(ptr) ((char *)(ptr) + GET_SIZE(HEAD(ptr)) - DSIZE)

/* Calculate address of next/previous blocks */
#define NEXT(ptr) ((char *)(ptr) + GET_SIZE(((char *)(ptr) - HFSIZE)))
#define PREV(ptr) ((char *)(ptr) - GET_SIZE(((char *)(ptr) - DWORD)))

/************************
// /* rounds up to the nearest multiple of ALIGNMENT */
// #define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
// #define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
/************************

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{   
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
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

//Used example in CSAPP chapter 9.9 for reference.