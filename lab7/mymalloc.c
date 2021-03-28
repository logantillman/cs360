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
    struct chunk *openMemory = NULL;

    while (listIt != NULL) {
        if (isOpenMemory == 1) {
            break;
        }

        struct chunk *tmpIt = (struct chunk *) listIt;
        listIt = free_list_next(listIt);
    }

    if (head == NULL || size > head->size) {
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
//                printf("HEAD WAS NULL, SETTING TO ");
                head = tmpHead;
                head->next = NULL;
                head->prev = NULL;
//                printf("%08x size is %d\n", head, head->size);
            }
            else {
//                printf("Allocated memory, adding %d bytes to free list ", tmpHead->size);
//                printf("old head %08x ", free_list_begin());
                head->prev = tmpHead;
                tmpHead->next = head;
                head = tmpHead;
//                printf("new head %08x\n", free_list_begin());
            }
        }
    }
    else {
        // printf("Head isn't null. Carve off the free list\n");
        // if (size <= head->size) {
            allocPtr = head;
            if (head->size == size) {
                head = NULL;
            }
            else {
//                printf("Carving %d bytes off free list ", size);

                head = (void *) head + size;
                head->size = allocPtr->size - size;
//                printf("Moved head from %08x to %08x\n", allocPtr, head);
            }
            
            allocPtr->size = size;
            // printf("head addr: %08x head size: %d tmpAddr: %08x tmpSize: %d\n", head, head->size, allocPtr, allocPtr->size);

        // }
    }

    // printf("HEAD: addr: %08x size: %d endofheap: %08x\n", head, head->size, sbrk(0));

    // printf("mymalloc returning %08x\n", (void *) allocPtr + 8);

    void *setWord = (void *) allocPtr + 4;

    strcpy((char *) setWord, "Til");

    return (void *) allocPtr + 8;
}

void my_free(void *ptr) {
    // printf("called my free\n");

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
//        printf("Adding free'd chunk of size %d to head of free list ", tmpPtr->size);
//        printf("old head %08x ", free_list_begin());
        struct chunk *tmp = (struct chunk *) ptr;
        head->prev = tmp;
        tmp->next = head;
        head = tmp;
//        printf("new head %08x\n", free_list_begin());
        // printf("next %08x\n", free_list_next(free_list_begin()));
    }

    /* Printing out the free list */
//    printf("    Free List:\n");
    void *it = free_list_begin();
    while (it != NULL) {
        struct chunk *fin = (struct chunk *) it;
//        printf("    %08x it->size %d\n", it, fin->size);
        it = free_list_next(it);
    }
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