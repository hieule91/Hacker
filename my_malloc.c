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

	// static metadata_t* merge(metadata_t* node, metadata_t* next);

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


void* findBestBlock(size_t request_size) {
    if (freelist == NULL) {
        return NULL;
    }
    metadata_t* prev_block_ptr = NULL;
    metadata_t* curr_block_ptr = freelist;
    metadata_t* best_fit_ptr = NULL;

    // looking for the block that is either greater than or equal to the user requested size
    // look for the block that best fit in the freelist and set prev and next blocks
    while (curr_block_ptr->next != NULL && curr_block_ptr->size < request_size ) {
        prev_block_ptr = curr_block_ptr;
        curr_block_ptr = curr_block_ptr->next;

    }

    //a)when the block is exactly the same size
    if (request_size == curr_block_ptr->size) {
        best_fit_ptr = curr_block_ptr;

        if (prev_block_ptr == NULL) {
            freelist = curr_block_ptr->next;
        } else {
            prev_block_ptr->next = curr_block_ptr->next;
            curr_block_ptr->next = NULL;

        }
        curr_block_ptr->canary = ((uintptr_t)curr_block_ptr^CANARY_MAGIC_NUMBER)-curr_block_ptr->size;
        unsigned int* tail_carany = (unsigned int*)((uint8_t*)(curr_block_ptr) + curr_block_ptr->size 
        	- sizeof(curr_block_ptr->canary));
        *tail_carany = curr_block_ptr->canary;
        // best_fit_ptr = (metadata_t*) ((uintptr_t)curr_block_ptr + sizeof(metadata_t));
        my_malloc_errno = NO_ERROR;
        return (void*) (best_fit_ptr + 1);

    }

    //b)when the block is big enough to house a new block
    unsigned int block_space_left_over = curr_block_ptr->size - request_size;
    if (curr_block_ptr->size > request_size && block_space_left_over >= MIN_BLOCK_SIZE) {
        best_fit_ptr = curr_block_ptr;
        if (prev_block_ptr == NULL) {
            freelist = curr_block_ptr->next;
        } else {
            prev_block_ptr->next = curr_block_ptr->next;
            curr_block_ptr->next = NULL;
        }
        //split the block size and get the block size that we want out of the freelist
        //left_over_block_ptr points to the new block that we want to split
        metadata_t* left_over_block_ptr = (metadata_t*)((uint8_t*)curr_block_ptr + request_size);
        
        //set the canaries current block pointer
        best_fit_ptr->size = request_size;
        best_fit_ptr->next = NULL;
        curr_block_ptr->canary = ((uintptr_t)curr_block_ptr^CANARY_MAGIC_NUMBER)-curr_block_ptr->size;
        unsigned int* tail_carany_curr = (unsigned int*)((uint8_t*)(curr_block_ptr) + curr_block_ptr->size 
        	- sizeof(curr_block_ptr->canary));
        *tail_carany_curr = curr_block_ptr->canary;
        
        //set the canaries for the new block
        left_over_block_ptr->size = block_space_left_over;
        left_over_block_ptr->next = NULL;

        left_over_block_ptr->canary = ((uintptr_t)left_over_block_ptr^CANARY_MAGIC_NUMBER)-left_over_block_ptr->size;
        unsigned int* tail_carany_left_over = (unsigned int*)((uint8_t*)(left_over_block_ptr) + curr_block_ptr->size 
        	- sizeof(left_over_block_ptr->canary));
        *tail_carany_left_over = left_over_block_ptr->canary;
        curr_block_ptr->next = left_over_block_ptr;

        //add the left over space back to the freelist
        if (freelist == NULL) {
            freelist = left_over_block_ptr;
            my_malloc_errno = NO_ERROR;

            return (void*)(best_fit_ptr + 1);
        }
        metadata_t* new_freelist_ptr = freelist;
        metadata_t* prev_newfreelist_ptr = NULL;
        int keep_track = 0;

        while (new_freelist_ptr != NULL) {
            if (left_over_block_ptr->size < new_freelist_ptr->size) {
                if (prev_newfreelist_ptr != NULL) {
                    prev_newfreelist_ptr->next = left_over_block_ptr;
                    left_over_block_ptr->next = new_freelist_ptr;
                    keep_track = 1;
                } else {
                    left_over_block_ptr->next = new_freelist_ptr;
                    freelist = left_over_block_ptr;
                    keep_track = 1;
                }
                my_malloc_errno = NO_ERROR;
                return (void*) (best_fit_ptr + 1);
            }
            prev_newfreelist_ptr = new_freelist_ptr;
            new_freelist_ptr = new_freelist_ptr->next;
        }

        if (keep_track == 0) {
            prev_newfreelist_ptr->next = left_over_block_ptr;
        }
        my_malloc_errno = NO_ERROR;
        return (void*) (best_fit_ptr + 1);
    }
    my_malloc_errno = NO_ERROR;
    return (void*) (best_fit_ptr + 1);
}

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

    // metadata_t* bestFitBlock = findBestBlock(request_size);

    // //d)create the new block 
    // if (bestFitBlock == NULL) {
    //     metadata_t* newBlock = my_sbrk(SBRK_SIZE);
    //     if (newBlock == NULL) {
    //         my_malloc_errno = OUT_OF_MEMORY;
    //         return NULL;
    //     } else {
    //         newBlock->size = SBRK_SIZE;
    //         newBlock->next = NULL;
    //         if (freelist == NULL) {
    //             freelist = newBlock;
    //         } else {
    //             metadata_t* curr_block = freelist;
    //             while(curr_block != NULL) {
    //                 if ((metadata_t*) ((uintptr_t)curr_block + curr_block->size) == newBlock) {
    //                     curr_block->size += newBlock->size;
    //                 }
    //                 curr_block = curr_block->next;
    //             }
    //         }
    //         bestFitBlock = findBestBlock(request_size);
    //     }
    // }
    // my_malloc_errno = NO_ERROR;
    // return bestFitBlock;

    // create and return a new block or the best fit block
    if (freelist) {
		// find the best fit block
		metadata_t* bestFitBlock = (metadata_t*)findBestBlock(request_size);
		
		// split the block: canary, size, ....
		my_malloc_errno = NO_ERROR;
		return bestFitBlock; 
		// (void *)(((char *)bestFitBlock) + sizeof(metadata_t));
	} else {
		freelist = my_sbrk(SBRK_SIZE);
		if (!freelist) {
			my_malloc_errno = OUT_OF_MEMORY;
			return NULL;
		}
		freelist->size = request_size; 
		return my_malloc(request_size);
	}
  	// return bestFitBlock;
}

/*
 * Helper function to append to merge two nodes together
 */
static metadata_t* merge(metadata_t* node, metadata_t* next) {
    if (next == freelist) {
        freelist = node;
    }
	if ((uintptr_t)((char*)node + node->size) == (uintptr_t)next) {
        node->size = node->size + next->size;
        node->next = next->next;
        unsigned int *tail_canary = (unsigned int *)((uint8_t *)node + node->size - sizeof(int));
    	node->canary = *tail_canary = ((uintptr_t)node ^ CANARY_MAGIC_NUMBER) - node->size;
        // *(unsigned long*)(move_ptr(node, node->size - 1)) =
        //     (uintptr_t)node^((metadata_t*)node)->size;
        // (((uintptr_t)node^CANARY_MAGIC_NUMBER)-node->size) = (uintptr_t)node^((metadata_t*)node)->size; 
   	} else if ((((uintptr_t)node ^ CANARY_MAGIC_NUMBER) - node->size) < (uintptr_t)next) {
		node->next = next;
	}
    return node;

}

/* REALLOC
 * See my_malloc.h for documentation
 */
void *my_realloc(void *ptr, size_t size) {
    // UNUSED_PARAMETER(ptr);
    // UNUSED_PARAMETER(size);
    // return NULL;

    metadata_t* newblock = my_malloc(size);
    if (newblock == NULL) {
        if (ptr != NULL) {
            my_free(ptr);
        }
        my_malloc_errno = NO_ERROR;
        return NULL;
    }

    if (ptr != NULL) {
        size_t new_size;
        if ((newblock - 1)->size > ((metadata_t*)ptr - 1)->size) {
            new_size = (((metadata_t*)ptr - 1)->size - TOTAL_METADATA_SIZE);
        } else {
            new_size = ((newblock - 1)->size - TOTAL_METADATA_SIZE);
        }
        memcpy(newblock, ptr, new_size);
        my_free(ptr);
    }
    my_malloc_errno = NO_ERROR;
    return newblock;

}

/* CALLOC
 * See my_malloc.h for documentation
 */
void *my_calloc(size_t nmemb, size_t size) {
    // UNUSED_PARAMETER(nmemb);
    // UNUSED_PARAMETER(size);
    // return NULL;
    size_t totalSize = nmemb * size;
    metadata_t* newblock = my_malloc(totalSize);
    if (newblock == NULL) {
    	my_malloc_errno = NO_ERROR;
        return NULL;
    }
    memset(newblock, 0, totalSize);
    my_malloc_errno = NO_ERROR;
    return newblock;
}

/* FREE
 * See my_malloc.h for documentation
 */
void my_free(void *ptr) {
    // UNUSED_PARAMETER(ptr);

	// check free null
	if (ptr == NULL) {
		my_malloc_errno = NO_ERROR;
		return;
	}

	// proper address of the block
	metadata_t* block = (metadata_t*) ptr - 1;
	// check canaries
	unsigned int original_canary = (uintptr_t)block^block->size;
	unsigned int canary = block->canary;

	if (original_canary != canary) {
		my_malloc_errno = CANARY_CORRUPTED;
		return;
	} else {
		canary = ((uintptr_t)block ^ CANARY_MAGIC_NUMBER) - block->size;
		if (original_canary != canary) {
			my_malloc_errno = CANARY_CORRUPTED;
			return;
		}
	}

	if (!freelist) {
		my_malloc_errno = NO_ERROR;
		freelist = block;
		return;
	}

    metadata_t dummy;
	metadata_t* prev_block = &dummy;
	prev_block->next = freelist;
	while(prev_block->next != NULL
		&& (((uintptr_t)block^CANARY_MAGIC_NUMBER) - block->size) > (uintptr_t)prev_block->next) {
		prev_block = prev_block->next;
	}

    if (prev_block->next != NULL) {
        merge(block, prev_block->next);
    }

    if (prev_block != &dummy) {
        merge(prev_block, block);
    }
    my_malloc_errno = NO_ERROR;
}
