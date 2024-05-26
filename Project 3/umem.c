#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "umem.h"

typedef struct __node_t {
    int size;
    struct __node_t *next;
} node_t;

int initialized = 0;
int allocationAlgorithm = 0;
node_t *freeListHead = NULL;
node_t *nextFreeNodeFromLastAllocation = NULL;

size_t roundUpToMultipleOf(size_t originalSize, size_t multipleOfSize) {
    return ((originalSize + multipleOfSize - 1) / multipleOfSize) * multipleOfSize;
}

int umeminit(size_t sizeOfRegion, int allocationAlgo) {

    if (initialized || sizeOfRegion <= 0) {
        return -1;
    }

    sizeOfRegion = roundUpToMultipleOf(sizeOfRegion, getpagesize());

    if (sizeOfRegion <= 0) {
        return -1;
    }

    int fd = open("/dev/zero", O_RDWR);

    if (fd == -1) {
        return -1;
    }

    void *ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) {
        //perror("mmap");
        //exit(1);
        return -1;
    }

    close(fd);

    freeListHead = (node_t *) ptr;
    freeListHead->size = (int) (sizeOfRegion - sizeof(node_t));
    freeListHead->next = NULL;

    allocationAlgorithm = allocationAlgo;
    initialized = 1;

    return 0;
}

void *umalloc(size_t size) {

    size = roundUpToMultipleOf(size, (size_t) 8);

    if (size <= 0) {
        return NULL;
    }

    node_t *currentNode = freeListHead;
    node_t *bestNode = NULL;

    if (currentNode == NULL) {
        return NULL;
    }

    if (allocationAlgorithm == BEST_FIT) {

        while (currentNode != NULL) {

            if (currentNode->size >= (int) size) {

                if (bestNode == NULL || currentNode->size < bestNode->size) {
                    bestNode = currentNode;
                }

            }

            currentNode = currentNode->next;

        }

    } else if (allocationAlgorithm == WORST_FIT) {

        while (currentNode != NULL) {

            if (currentNode->size >= (int) size) {

                if (bestNode == NULL || currentNode->size > bestNode->size) {
                    bestNode = currentNode;
                }

            }

            currentNode = currentNode->next;

        }

    } else if (allocationAlgorithm == FIRST_FIT) {

        while (currentNode != NULL) {

            if (currentNode->size >= (int) size) {
                bestNode = currentNode;
                break;
            }

            currentNode = currentNode->next;

        }

    } else if (allocationAlgorithm == NEXT_FIT) {

        if (nextFreeNodeFromLastAllocation != NULL) {
            currentNode = nextFreeNodeFromLastAllocation;
        }

        while (currentNode != NULL) {

            if (currentNode->size >= (int) size) {
                bestNode = currentNode;
                break;
            }

            currentNode = currentNode->next;

        }

        if (bestNode == NULL) {

            currentNode = freeListHead;

            while (currentNode != NULL) {

                if (currentNode->size >= (int) size) {
                    bestNode = currentNode;
                    break;
                }

                if (currentNode->next == nextFreeNodeFromLastAllocation) {
                    break;
                }

                currentNode = currentNode->next;

            }

        }

    }

    if (bestNode == NULL) {
        return NULL;
    }

    if (bestNode->size > (int) (size + sizeof(node_t))) {

        node_t *newNode = (node_t *) ((char *) bestNode + sizeof(node_t) + size);
        newNode->size = bestNode->size - (int) (size + sizeof(node_t));
        newNode->next = bestNode->next;

        bestNode->size = (int) size;
        bestNode->next = newNode;

    }

    if (freeListHead == bestNode) {
        freeListHead = bestNode->next;
    } else {

        currentNode = freeListHead;

        while (currentNode != NULL) {

            if (currentNode->next == bestNode) {
                currentNode->next = bestNode->next;
                break;
            }

            currentNode = currentNode->next;

        }

    }

    if (allocationAlgorithm == NEXT_FIT) {
        nextFreeNodeFromLastAllocation = bestNode->next;
    }

    return (void *) ((char *) bestNode + sizeof(node_t));

}

int ufree(void *ptr) {

    if (ptr == NULL || !initialized) {
        return -1;
    }

    node_t *nodeToFree = (node_t *) ((char *) ptr - sizeof(node_t));

    node_t *previousNode = NULL;
    node_t *currentNode = freeListHead;

    while (currentNode != NULL && currentNode < nodeToFree) {
        previousNode = currentNode;
        currentNode = currentNode->next;
    }

    if (currentNode == nodeToFree) {
        return -1;
    }

    if (previousNode != NULL && (char *) previousNode + sizeof(node_t) + previousNode->size == (char *) nodeToFree) {

        previousNode->size = previousNode->size + (int) sizeof(node_t) + nodeToFree->size;
        nodeToFree = previousNode;

    } else {

        nodeToFree->next = currentNode;

        if (previousNode != NULL) {
            previousNode->next = nodeToFree;
        } else {
            freeListHead = nodeToFree;
        }

    }

    if (nodeToFree->next != NULL) {

        if ((char *) nodeToFree + sizeof(node_t) + nodeToFree->size == (char *) nodeToFree->next) {

            if (allocationAlgorithm == NEXT_FIT && nodeToFree->next == nextFreeNodeFromLastAllocation) {
                nextFreeNodeFromLastAllocation = nodeToFree;
            }

            nodeToFree->size = nodeToFree->size + (int) sizeof(node_t) + nodeToFree->next->size;
            nodeToFree->next = nodeToFree->next->next;
        }

    }

    return 0;

}

void umemdump() {

    node_t *currentNode = freeListHead;

    if (allocationAlgorithm == NEXT_FIT) {

        if (nextFreeNodeFromLastAllocation == NULL) {

            nextFreeNodeFromLastAllocation = freeListHead;

        }

        printf("------------------------------------\n");
        printf("The address of the next free node from last allocation: %p, Size: %zu\n", (void *)((char *)nextFreeNodeFromLastAllocation + sizeof(node_t)), (size_t) nextFreeNodeFromLastAllocation->size);

    }

    printf("--------BEGIN OF MEMORY DUMP--------\n");

    while (currentNode != NULL) {

        printf("Address: %p, Size: %zu\n", (void *)((char *)currentNode + sizeof(node_t)), (size_t) currentNode->size);
        currentNode = currentNode->next;

    }

    printf("---------END OF MEMORY DUMP---------\n");

}
