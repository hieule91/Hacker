/*
 * CS 2110 Spring 2017
 * Author: HIEU LE
 */

/* we need this for uintptr_t */
#include <stdint.h>
/* we need this for memcpy/memset */
#include <string.h>
#include <stdio.h>
/* we need this for my_sbrk */
#include "my_sbrk.h"
/* we need this for the metadata_t struct and my_malloc_err enum definitions */
#include "my_malloc.h"

/* You *MUST* use this macro when calling my_sbrk to allocate the
 * appropriate size. Failure to do so may result in an incorrect
 * grading!
 */
#define SBRK_SIZE 2048

/* This is the size of the metadata struct and canary footer in bytes */
#define TOTAL_METADATA_SIZE (sizeof(metadata_t) + sizeof(int))

/* This is the minimum size of a block in bytes, where it can
 * store the metadata, canary footer, and at least 1 byte of data
 */
#define MIN_BLOCK_SIZE (TOTAL_METADATA_SIZE + 1)

/* Used in canary calcuations. See the "Block Allocation" section of the
 * homework PDF for details.
 */
#define CANARY_MAGIC_NUMBER 0xE629

/* Feel free to delete this (and all uses of it) once you've implemented all
 * the functions
 */
#define UNUSED_PARAMETER(param) (void)(param)

/* Our freelist structure - this is where the current freelist of
 * blocks will be maintained. failure to maintain the list inside
 * of this structure will result in no credit, as the grader will
 * expect it to be maintained here.
 * DO NOT CHANGE the way this structure is declared
 * or it will break the autograder.
 */
metadata_t *freelist;

/* Set on every invocation of my_malloc()/my_free()/my_realloc()/
 * my_calloc() to indicate success or the type of failure. See
 * the definition of the my_malloc_err enum in my_malloc.h for details.
 * Similar to errno(3).
 */
enum my_malloc_err my_malloc_errno;


// // if freelist available
// void *findBestBlock(size_t size) {
// 	UNUSED_PARAMETER(size);
// 	// while (freelist) {
// 	// 	metadata_t *userBlock = NULL;
// 	// 	if (userBlock->size == freelist->size) {

// 	// 	}  
// 	// }
// 	// return freelist;

// 	if (freelist == NULL) {
// 		return NULL;
// 	}
// 	metadata_t* currBlock = freelist;
// 	metadata_t* bestFitBlock = NULL;
// 	metadata_t* remainedBlock = NULL;

// 	while (currBlock) {
// 		if (currBlock->size < size)
// 	}
// }
	

/* MALLOC
 * See my_malloc.h for documentation
 */
void *my_malloc(size_t size) {
	// UNUSED_PARAMETER(size);
    // return NULL;
    unsigned int request_size = size + TOTAL_METADATA_SIZE;
    if (request_size > SBRK_SIZE) {
    	my_malloc_errno = SINGLE_REQUEST_TOO_LARGE;
    	return NULL;
    }
    if (size == 0) {
    	my_malloc_errno = NO_ERROR;
    	return NULL;
    }

	// check freelist
	if (freelist) {
		// find the best fit block
		metadata_t* bestFitBlock = findBestBlock(size);
		
		// split the block: canary, size, ....

		return (void*) (((char*) bestFitBlock) + sizeof(int) + sizeof(metadata_t)); 
		// (void *)(((char *)bestFitBlock) + sizeof(metadata_t));
	} else {
		freelist = my_sbrk(SBRK_SIZE);
		if (!freelist) {
			my_malloc_errno = OUT_OF_MEMORY;
			return NULL;
		}
		freelist->size = size; 
		return my_malloc(size);
	}
}




void *my_malloc2(size_t size) {
    // UNUSED_PARAMETER(size);
    // return NULL;
    unsigned int request_size = size + TOTAL_METADATA_SIZE + sizeof(int);
    if (request_size > SBRK_SIZE) {
    	my_malloc_errno = SINGLE_REQUEST_TOO_LARGE;
    	return NULL;
    }
    if (size == 0) {
    	my_malloc_errno = NO_ERROR;
    	return NULL;
    }

    // loop through the freelist to find the best fit block
  	metadata_t *currBlock = freelist;
  	metadata_t *bestFitBlock = NULL;
  	while (currBlock != NULL) {
  		// check if the current block is large enough
  		if (currBlock->size >= request_size) {
  			// check if the it is the smallest fit?
  			if (bestFitBlock == NULL) {
  				bestFitBlock = currBlock;
  			} else {
  				// check if the current block can offer us a better (smaller)
  				if (bestFitBlock->size > currBlock->size) {
  					bestFitBlock = currBlock; 
  				}
  			}
  		}
  		printf("%d\n",bestFitBlock->size);
   		currBlock = currBlock->next;
  	}

  	//If the freelist is either NULL or does not contain any big enough block, the bestMatch will be still NULL.
  	//then we need to call my_sbrk() to give us a new fresh 2KB free junk of memory
  	if (bestFitBlock == NULL) {
  		bestFitBlock = my_sbrk(SBRK_SIZE);
  		//check if my_sbrk return failure
  		if (bestFitBlock == NULL) {
  			my_malloc_errno = OUT_OF_MEMORY;
  			return NULL;
  		}

  		// OOPS: you forget to set up the metadata for this 2KB free block. Do we need to care it
  		// because at the end before we return the block to the user we will do it?
  		// ANS: yes
  		bestFitBlock->size = SBRK_SIZE;
  		// bestFitBlock->request_size = 0;
  		// bestFitBlock->prev = NULL;
  		bestFitBlock->next = freelist;

  		//add it into the free list
  		if (freelist == NULL) {
  			freelist = bestFitBlock;
  		} else {
	  		// freelist->prev = bestFitBlock;
	  		freelist = bestFitBlock;
  		}
  		//now the 2KB free block is added to the front of the freelist
  	}
  	printf("%p\n", (void *)bestFitBlock);
  	printf("%s\n", "----------------------------------");
  	return bestFitBlock;
}


/* REALLOC
 * See my_malloc.h for documentation
 */
void *my_realloc(void *ptr, size_t size) {
    UNUSED_PARAMETER(ptr);
    UNUSED_PARAMETER(size);
    return NULL;
}

/* CALLOC
 * See my_malloc.h for documentation
 */
void *my_calloc(size_t nmemb, size_t size) {
    UNUSED_PARAMETER(nmemb);
    UNUSED_PARAMETER(size);
    return NULL;
}

/* FREE
 * See my_malloc.h for documentation
 */
void my_free(void *ptr) {
    UNUSED_PARAMETER(ptr);
}
