// Author: Logan Tillman

#include <unistd.h>
#include <string.h>
#include "mymalloc.h"

struct chunk {
    int size;
    struct chunk *next;
    struct chunk *prev;
};

struct chunk *head = NULL;

// int main(int argc, char **argv) {

//     my_malloc(atoi(argv[1]));
//     my_malloc(atoi(argv[2]));

//     return 0;
// }

void *my_malloc(size_t size) {
    // printf("Called mymalloc\n");
    if (size < 1) {
        fprintf(stderr, "Invalid size\n");
        exit(1);
    }

    size = (size + 7 + 8) & -8;
    // printf("Size: %d\n", size);

    struct chunk *allocPtr = NULL;
    void *ptr = NULL;

    void *listIt = free_list_begin();
    int isOpenMemory = 0;
    int isHead = 0;
    struct chunk *openMemory = NULL;

    while (listIt != NULL) {
        if (isOpenMemory == 1) {
            break;
        }

        struct chunk *tmpIt = (struct chunk *) listIt;
        if (tmpIt->size >= size) {
            // printf("Found chunk\n");
            if (tmpIt == head) {
                // printf("Found chunk is the head\n");
                isHead = 1;
            }
            isOpenMemory = 1;
            openMemory = tmpIt;
        }
        listIt = free_list_next(listIt);
    }

    if (head == NULL || isOpenMemory == 0) {
        if (size > 8192) {
            ptr = sbrk(size);
            allocPtr = (struct chunk *) ptr;
            allocPtr->size = size;
        }
        else {
            ptr = sbrk(8192);
            allocPtr = (struct chunk *) ptr;
            allocPtr->size = size;

            struct chunk *tmpHead = (void *) allocPtr + size;
            // printf("allocPtr: %08x tmpHead: %08x\n", allocPtr, tmpHead);
            tmpHead->size = 8192 - size;
            // printf("allocPtr->size %d tmpHead->size %d\n", allocPtr->size, tmpHead->size);
            if (head == NULL) {
                // printf("HEAD WAS NULL, SETTING TO ");
                head = tmpHead;
                head->next = NULL;
                head->prev = NULL;
                // printf("%08x size is %d\n", head, head->size);
            }
            else {
                // printf("Allocated memory, adding %d bytes to free list ", tmpHead->size);
                // printf("old head %08x ", free_list_begin());
                head->prev = tmpHead;
                tmpHead->next = head;
                head = tmpHead;
                // printf("new head %08x\n", free_list_begin());
            }
        }
    }
    else if (isOpenMemory == 1) {
        // printf("Head isn't null. Carve off the free list\n");
        // if (size <= head->size) {
            // allocPtr = openMemory;
            // printf("size: %d open: %08x alloc: %08x\n", size, openMemory, allocPtr);
            if (openMemory->size == size) {
                struct chunk *tmpChunk = NULL;
                if (isHead == 1) {
                    if (head->next != NULL) {
                        tmpChunk = head->next;
                    }
                }

                // printf("Carving %d bytes from mem chunk %08x of size %d\n", size, openMemory, openMemory->size);
                if (openMemory->prev != NULL) {
                    // printf("1: Linking %08x to %08x ", openMemory->prev, openMemory->next);
                    openMemory->prev->next = openMemory->next;
                }
                if (openMemory->next != NULL) {
                    // printf("2: Linking %08x to %08x\n", openMemory->next, openMemory->prev);
                    openMemory->next->prev = openMemory->prev;
                }
                allocPtr = openMemory;

                if (isHead == 1) {
                    head = tmpChunk;
                }
            }
            else {
                // printf("Carving %d bytes off free list ", size);
                struct chunk *tmpPrev = openMemory->prev;
                struct chunk *tmpNext = openMemory->next;

                allocPtr = openMemory;
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

                // printf("Moved open memory from %08x to %08x new size is %d\n", allocPtr, openMemory, openMemory->size);
                if (isHead == 1) {
                    head = openMemory;
                    // printf("Set head to %08x\n", head);
                }
            }
            allocPtr->size = size;
            // printf("head addr: %08x head size: %d tmpAddr: %08x tmpSize: %d\n", head, head->size, allocPtr, allocPtr->size);

        // }
    }

    // printf("HEAD: addr: %08x size: %d endofheap: %08x\n", head, head->size, sbrk(0));

    // printf("mymalloc returning %08x\n", (void *) allocPtr + 8);

    void *setWord = (void *) allocPtr + 4;

    strcpy((char *) setWord, "Til");
    // printf("Made it to return allocPtr %08x size %d\n", allocPtr, allocPtr->size);

    // struct chunk *listTrav = free_list_begin();
    // while (listTrav != NULL) {
    //     printf("%08x -> ", listTrav);
    //     listTrav = free_list_next(listTrav);
    // }
    // printf("\n");

    return (void *) allocPtr + 8;
}

void my_free(void *ptr) {
    // printf("Current head: %08x Ptr: %08x\n", head, ptr);

    char *word = ptr - 4;

    if (strcmp(word, "Til") != 0) {
        fprintf(stderr, "Error with my free\n");
        exit(1);
    }

    ptr -= 8;

    struct chunk *tmpPtr = (struct chunk *) ptr;

    if (head == NULL) {
        head = (struct chunk *) ptr;
        head->next = NULL;
        head->prev = NULL;
    }
    else {
        // printf("Adding free'd chunk of size %d to head of free list ", tmpPtr->size);
        // printf("old head %08x tmpPtr %08x ", head, tmpPtr);
        // struct chunk *tmp = (struct chunk *) ptr;
        head->prev = tmpPtr;
        tmpPtr->next = head;
        tmpPtr->prev = NULL;
        head = tmpPtr;
        // printf("new head %08x next %08x\n", free_list_begin(), free_list_next(free_list_begin()));
        // printf("next %08x\n", free_list_next(free_list_begin()));
    }

    /* Printing out the free list */
    // printf("    Free List:\n");
    // void *it = free_list_begin();
    // while (it != NULL) {
    //     struct chunk *fin = (struct chunk *) it;
    //     printf("    %08x it->size %d\n", it, fin->size);
    //     it = free_list_next(it);
    // }
}

void *free_list_begin() {
    // printf("Made it to free list begin\n");
    if (head == NULL) {
        // printf("Returned NULL\n");
        return NULL;
    }

    // printf("returned %08x\n", (void *) head);
    return (void *) head;
}

void *free_list_next(void *node) {
    // printf("Made it to free list next\n");
    struct chunk *nodeChunk = (struct chunk *) node;
    if (nodeChunk->next == NULL) {
        // printf("Returned NULL\n");
        return NULL;
    }
    else {
        // printf("returned %08x\n", (void *) nodeChunk->next);
        return (void *) nodeChunk->next;
    }
}

void coalesce_free_list() {
    // printf("Made it to coalesce_free_list\n");

}