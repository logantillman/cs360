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

void *my_malloc(size_t size) {
    if (size < 1) {
        fprintf(stderr, "Invalid size\n");
        exit(1);
    }

    size = (size + 7 + 8) & -8;

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
            if (tmpIt == head) {
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
            tmpHead->size = 8192 - size;
            if (head == NULL) {
                head = tmpHead;
                head->next = NULL;
                head->prev = NULL;
            }
            else {
                head->prev = tmpHead;
                tmpHead->next = head;
                head = tmpHead;
            }
        }
    }
    else if (isOpenMemory == 1) {
            if (openMemory->size == size) {
                struct chunk *tmpChunk = NULL;
                if (isHead == 1) {
                    if (head->next != NULL) {
                        tmpChunk = head->next;
                    }
                }

                if (openMemory->prev != NULL) {
                    openMemory->prev->next = openMemory->next;
                }
                if (openMemory->next != NULL) {
                    openMemory->next->prev = openMemory->prev;
                }
                allocPtr = openMemory;

                if (isHead == 1) {
                    head = tmpChunk;
                }
            }
            else {
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

                if (isHead == 1) {
                    head = openMemory;
                }
            }
            allocPtr->size = size;
    }

    void *setWord = (void *) allocPtr + 4;

    strcpy((char *) setWord, "Til");

    return (void *) allocPtr + 8;
}

void my_free(void *ptr) {

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
        head->prev = tmpPtr;
        tmpPtr->next = head;
        tmpPtr->prev = NULL;
        head = tmpPtr;
    }
}

void *free_list_begin() {
    if (head == NULL) {
        return NULL;
    }

    return (void *) head;
}

void *free_list_next(void *node) {
    struct chunk *nodeChunk = (struct chunk *) node;
    if (nodeChunk->next == NULL) {
        return NULL;
    }
    else {
        return (void *) nodeChunk->next;
    }
}

void coalesce_free_list() {
    // printf("Made it to coalesce_free_list\n");

}