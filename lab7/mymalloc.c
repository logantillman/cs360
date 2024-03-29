// Author: Logan Tillman

#include <unistd.h>
#include <string.h>
#include "mymalloc.h"

/* Struct for storing the chunk data */
struct chunk {
    int size;
    struct chunk *next;
    struct chunk *prev;
};

/* Global variable for the head of the free list */
struct chunk *head = NULL;

/* Compare function for qsort */
int compare (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

/* Function that allocates memory given a size */
void *my_malloc(size_t size) {
    if (size < 1) {
        fprintf(stderr, "Invalid size\n");
        exit(1);
    }

    /* Making the size a multiple of 8 and padding another 8 bytes */
    size = (size + 7 + 8) & -8;

    struct chunk *allocPtr = NULL;
    void *ptr = NULL;

    void *listIt = free_list_begin();
    int isOpenMemory = 0;
    int isHead = 0;
    struct chunk *openMemory = NULL;

    /* Checking to see if there's an open chunk of free memory big enough for the requested size */
    while (listIt != NULL) {

        /* Tracks if we found an open chunk of memory */
        if (isOpenMemory == 1) {
            break;
        }

        /* Checking to see if the size is big enough */
        struct chunk *tmpIt = (struct chunk *) listIt;
        if (tmpIt->size >= size) {
            
            /* Keeping track if the open memory is the head of the free list */
            if (tmpIt == head) {
                isHead = 1;
            }
            isOpenMemory = 1;
            openMemory = tmpIt;
        }
        listIt = free_list_next(listIt);
    }

    /* If the free list is empty or there is no free memory chunks big enough, we allocate more memory */
    if (head == NULL || isOpenMemory == 0) {

        /* Allocating given size if greater than 8192 */
        if (size > 8192) {
            ptr = sbrk(size);
            allocPtr = (struct chunk *) ptr;
            allocPtr->size = size;
        }

        /* Allocating 8192 if the size is smaller than 8185 */
        else {
            ptr = sbrk(8192);

            /* Allocating the requested size */
            allocPtr = (struct chunk *) ptr;
            allocPtr->size = size;

            /* Putting the rest of the memory on the free list */
            struct chunk *tmpHead = (void *) allocPtr + size;
            tmpHead->size = 8192 - size;

            /* If the free list is empty, set this chunk as the head */
            if (head == NULL) {
                head = tmpHead;
                head->next = NULL;
                head->prev = NULL;
            }

            /* If the free list is not empty, add this chunk to the top of the list */
            else {
                head->prev = tmpHead;
                tmpHead->next = head;
                head = tmpHead;
            }
        }
    }

    /* If there is a free memory chunk with sufficient size */
    else if (isOpenMemory == 1) {

            /* If the requested size will take up the whole chunk */
            if (openMemory->size == size) {
                struct chunk *tmpChunk = NULL;

                /* If we're replacing the head, we need to store the new head for after allocation */
                if (isHead == 1) {
                    if (head->next != NULL) {
                        tmpChunk = head->next;
                    }
                }

                /* Updating the next and previous chunk pointers */
                if (openMemory->prev != NULL) {
                    openMemory->prev->next = openMemory->next;
                }
                if (openMemory->next != NULL) {
                    openMemory->next->prev = openMemory->prev;
                }

                /* Allocate the memory */
                allocPtr = openMemory;

                /* If we replaced the head, update the head variable */
                if (isHead == 1) {
                    head = tmpChunk;
                }
            }

            /* If the requested size will only take up a portion of the chunk */
            else {

                /* Storing the previous and next chunks */
                struct chunk *tmpPrev = openMemory->prev;
                struct chunk *tmpNext = openMemory->next;

                /* Allocate the memory */
                allocPtr = openMemory;

                /* Move the free memory chunk and update the links */
                openMemory = (void *) openMemory + size;
                openMemory->size = allocPtr->size - size;
                openMemory->prev = tmpPrev;
                openMemory->next = tmpNext;
                
                if (openMemory->prev != NULL) {
                    openMemory->prev->next = openMemory;
                }
                if (openMemory->next != NULL) {
                    openMemory->next->prev = openMemory;
                }

                /* If we allocated some of the head, move the head pointer */
                if (isHead == 1) {
                    head = openMemory;
                }
            }

            /* Set the allocated size */
            allocPtr->size = size;
    }

    /* Allocating a word in the last 4 bytes of the padding for error checking */
    void *setWord = (void *) allocPtr + 4;

    strcpy((char *) setWord, "Til");

    return (void *) allocPtr + 8;
}

/* Function that frees the given chunk of memory at the pointer */
void my_free(void *ptr) {

    /* Error checking the chunk */
    char *word = ptr - 4;
    if (strcmp(word, "Til") != 0) {
        fprintf(stderr, "Error with my free\n");
        exit(1);
    }

    /* Decrementing to the padding bytes */
    ptr -= 8;

    struct chunk *tmpPtr = (struct chunk *) ptr;

    /* If free list is empty, this chunk is the head */
    if (head == NULL) {
        head = tmpPtr;
        head->next = NULL;
        head->prev = NULL;
    }

    /* If the list is not empty, move the current head back and set this chunk as the head */
    else {
        head->prev = tmpPtr;
        tmpPtr->next = head;
        tmpPtr->prev = NULL;
        head = tmpPtr;
    }
}

/* Returns the beginning of the free list */
void *free_list_begin() {
    if (head == NULL) {
        return NULL;
    }

    return (void *) head;
}

/* Returns the next value in the free list */
void *free_list_next(void *node) {
    struct chunk *nodeChunk = (struct chunk *) node;
    if (nodeChunk->next == NULL) {
        return NULL;
    }
    else {
        return (void *) nodeChunk->next;
    }
}

/* Function that combines contiguous chunks of memory */
void coalesce_free_list() {
    int size = 0;
    void *numIt = free_list_begin();

    /* Grabbing the number of chunks in the free list */
    while (numIt != NULL) {
        size++;
        numIt = free_list_next(numIt);
    }

    /* Allocating memory for an array of the chunk pointers */
    void **nodeArray = malloc(size * sizeof(void *));
    int i;

    /* Populating the chunk pointer array */
    numIt = free_list_begin();
    for (i = 0; i < size; i++) {
        nodeArray[i] = numIt;
        numIt = free_list_next(numIt);
    }

    /* Sorting the chunk pointers */
    qsort(nodeArray, size, 4, compare);

    /* Setting the free list links to match the sorted pointer array */
    for (i = 0; i < size; i++) {
        struct chunk *curr = (struct chunk *) nodeArray[i];

        /* Setting the previous node on the free list to the closest chunk of memory that comes before the current chunk */
        if ((i - 1) >= 0) {
            struct chunk *prev = (struct chunk *) nodeArray[i - 1];
            curr->prev = prev;
        }
        else {
            curr->prev = NULL;
            head = curr;
        }

        /* Setting the next node on the free list to the closest chunk of memory that comes after the current chunk */
        if ((i + 1) < size) {
            struct chunk *next = (struct chunk *) nodeArray[i + 1];
            curr->next = next;
        }
        else {
            curr->next = NULL;
        }
    }

    /* Freeing the node array because we don't need it anymore */
    free(nodeArray);

    void *firstNode = free_list_begin();

    /* Finding which chunks are contiguous and combining them */
    while (firstNode != NULL) {
        struct chunk *firstNodeChunk = (struct chunk *) firstNode;
        void *nextNode = free_list_next(firstNode);

        /* Combining chunks until we come across a chunk that isn't contiguous */
        while (nextNode != NULL) {
            if (firstNode + firstNodeChunk->size == nextNode) {
                struct chunk *nextNodeChunk = (struct chunk *) nextNode;
                firstNodeChunk->size += nextNodeChunk->size;
                firstNodeChunk->next = nextNodeChunk->next;
            }
            else {
                break;
            }

            nextNode = free_list_next(nextNode);
        }

        firstNode = free_list_next(firstNode);
    }
}