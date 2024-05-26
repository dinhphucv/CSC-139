#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "umem.h"

typedef void (*function)();

void printStatusOfUmeminit(int);
void printStatusOfUmalloc(void *);
void printStatusOfUfree(int);
void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
void test8();
void test9();
void test10();
void test11();

int main(void) {

    function tests[] = {test1, test2, test3, test4, test5, test6, test7, test8, test9, test10, test11};
    int numberOfTests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < numberOfTests; i++) {

        printf("\n");
        printf("------------------------------------------------------------------\n");
        printf("------------------------------------------------------------------\n");
        printf("\n");

        pid_t pid = fork();

        if (pid == 0) {
            tests[i]();
            exit(0);
        } else if (pid < 0) {

        } else {
            wait(NULL);
        }
    }

}

void printStatusOfUmeminit(int status) {

    if (status == 0) {
        printf("SUCCEEDED UMEMINIT\n");
    } else if (status == -1) {
        printf("FAILED UMEMINIT\n");
    }

}

void printStatusOfUmalloc(void *ptr) {

    if (ptr == NULL) {
        printf("FAILED UMALLOC\n");
    } else {
        printf("SUCCEEDED UMALLOC, the address: %p\n", ptr);
    }

}

void printStatusOfUfree(int status) {

    if (status == 0) {
        printf("SUCCEEDED UFREE\n");
    } else if (status == -1) {
        printf("FAILED UFREE\n");
    }

}

void test1() {
    printf("------------------TEST 1: FAILED MEMORY UMEMINIT------------------\n");
    printf("Request -1 bytes of memory from the OS using umeminit(-1, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(-1, BEST_FIT));
    printf("\n");
    printf("Request 0 bytes of memory from the OS using umeminit(0, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(0, BEST_FIT));
    printf("\n");
    printf("Request 1 bytes of memory from the OS using umeminit(1, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(1, BEST_FIT));
    umemdump();
    printf("\n");
    printf("Request another 1 bytes of memory from the OS using umeminit(1, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(1, BEST_FIT));
    umemdump();
    printf("\n");
    printf("There is only 1 free block of memory!\n");
    printf("The 4th call of UMEMINIT is failed as expected!\n");
    printf("The free block's size should be equal to the round up (to multiple of page) size - header size!\n");
    printf("----------------------------END TEST 1----------------------------\n");
}

void test2() {
    printf("-------TEST 2 (BEST_FIT): ALLOCATE ONE HUGE CHUNK OF MEMORY-------\n");
    printf("Request 1 bytes of memory from the OS using umeminit(1, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(1, BEST_FIT));
    umemdump();
    printf("\n");
    printf("Allocate one huge chunk of memory that equal to the size of the free block.\n");
    void *ptr = umalloc(4080);
    printStatusOfUmalloc(ptr);
    umemdump();
    printf("When all memory is allocated, there isn't a free memory block in the memory dump!\n");
    printf("The address of the memory dump before umalloc and the address return from umalloc is the SAME!\n");
    printf("\n");
    printf("There is no free memory node. Let's try to allocate some memory (400) to see if it successfully.\n");
    printStatusOfUmalloc(umalloc(400));
    printf("It's expected to have failed UMALLOC as there is no free node.\n");
    printf("\n");
    printf("Free allocated memory using ufree.\n");
    ufree(ptr);
    umemdump();
    printf("\n");
    printf("We now has one big node, let's request a node of memory that bigger than the currently free one (allocate 4096).\n");
    printStatusOfUmalloc(umalloc(4096));
    printf("And print out the memory dump\n");
    umemdump();
    printf("\n");
    printf("Eventhough we have a big free node but it doesn't fit the requested memory so the UMALLOC is failed.\n");
    printf("The free node is still remain in the free list.\n");
    printf("----------------------------END TEST 2----------------------------\n");
}

void test3() {
    printf("---TEST 3 (BEST_FIT): ALLOCATE 2 CHUNKS OF MEMORY AND FREE THEM---\n");
    printf("Request 5000 bytes of memory from the OS using umeminit(5000, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(5000, BEST_FIT));
    umemdump();
    printf("\n");
    printf("Allocate 2 chunks of memory with different sizes.\n");
    int memorySizes[] = {28, 16};
    void *ptrs[2];
    for (int i = 0; i < 2; i++) {
        printStatusOfUmalloc(ptrs[i] = umalloc(memorySizes[i]));
    }
    printf("\n");
    printf("Now free the first memory to test.\n");
    printStatusOfUfree(ufree(ptrs[0]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("After free the first memory, there are 2 node of free memory, the one just got freed and the rest\n");
    printf("\n");
    printf("Now free the second memory to test.\n");
    printStatusOfUfree(ufree(ptrs[1]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("After free the second memory, the memory is coalesced into one big free chunk, which has size equal to the original!\n");
    printf("----------------------------END TEST 3----------------------------\n");
}

void test4() {
    printf("----------TEST 4 (BEST_FIT): ALLOCATE 10 CHUNKS OF MEMORY----------\n");
    printf("Request 5000 bytes of memory from the OS using umeminit(5000, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(5000, BEST_FIT));
    umemdump();
    printf("\n");
    printf("Allocate 10 chunks of memory with different sizes.\n");
    int memorySizes[] = {28, 16, 80, 32, 50, 100, 25, 120, 90, 45};
    void *ptrs[10];
    for (int i = 0; i < 10; i++) {
        printStatusOfUmalloc(ptrs[i] = umalloc(memorySizes[i]));
    }
    printf("\n");
    printf("Now free some (4) memory to test.\n");
    printStatusOfUfree(ufree(ptrs[4]));
    printStatusOfUfree(ufree(ptrs[2]));
    printStatusOfUfree(ufree(ptrs[6]));
    printStatusOfUfree(ufree(ptrs[8]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("After free 4 memory node, without coalescing, ptrs index 4, 2, 6, 8 not next to each other.\n");
    printf("There are 5 nodes of free memory, 4 freed nodes and the rest node.\n");
    printf("\n");
    printf("Test if BEST_FIT is working properly, allocate 30 bytes of memory\n");
    printStatusOfUmalloc(umalloc(30));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("When allocate 30 bytes of memory, the free node with 32 bytes is removed from free list.\n");
    printf("BEST_FIT is working properly in this case.\n");
    printf("\n");
    printf("Now, allocate 24 bytes of memory.\n");
    printStatusOfUmalloc(umalloc(24));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("Node with 56 bytes is the best fit in this case, since the size of it is greater than requested size + 16 bytes header.\n");
    printf("It's expected for this node to be split, the new node should have a size of 56 - 24 - 16 = 16 bytes.\n");
    printf("This new node should replace the spot of 56 bytes node.\n");
    printf("BEST_FIT is working properly in this case.\n");
    printf("----------------------------END TEST 4----------------------------\n");
}

void test5() {
    printf("----------TEST 5 (WORST_FIT): ALLOCATE 10 CHUNKS OF MEMORY----------\n");
    printf("Request 5000 bytes of memory from the OS using umeminit(5000, WORST_FIT).\n");
    printStatusOfUmeminit(umeminit(5000, WORST_FIT));
    umemdump();
    printf("\n");
    printf("Allocate 10 chunks of memory with different sizes.\n");
    int memorySizes[] = {28, 16, 80, 32, 50, 100, 25, 120, 530, 6500};
    void *ptrs[10];
    for (int i = 0; i < 10; i++) {
        printStatusOfUmalloc(ptrs[i] = umalloc(memorySizes[i]));
    }
    printf("\n");
    printf("Now free some memory to test.\n");
    printStatusOfUfree(ufree(ptrs[4]));
    printStatusOfUfree(ufree(ptrs[2]));
    printStatusOfUfree(ufree(ptrs[6]));
    printStatusOfUfree(ufree(ptrs[8]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("After free 4 memory node, without coalescing, ptrs index 4, 2, 6, 8 not next to each other.\n");
    printf("There are 5 nodes of free memory, 4 freed nodes and the rest node.\n");
    printf("\n");
    printf("Test if WORST_FIT is working properly, allocate 30 bytes of memory\n");
    printStatusOfUmalloc(umalloc(30));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("When allocate 30 bytes of memory, the free node with 32 bytes (best fit) is remained in the free list.\n");
    printf("The first free node with 80 bytes (first fit) is also remained in the free list.\n");
    printf("The node with the largest size (536) is replaced with another node has smaller size (536 - 32 - 16 = 488 bytes).\n");
    printf("The free list still has 5 nodes.\n");
    printf("WORST_FIT is working properly in this example.\n");
    printf("\n");
    printf("Let's allocate another 20 bytes, which all current free nodes are suitable.\n");
    printf("The best fit is the 32 bytes node, the first fit is the 80 bytes node.\n");
    printf("The algorithm should pick 504 bytes node to split, as it is the largest node.\n");
    printStatusOfUmalloc(umalloc(20));
    umemdump();
    printf("\n");
    printf("504 bytes node is split as expected, the new node size is 504 - 16(new header) - 24 (round up of 20) = 464.\n");
    printf("WORST_FIT is working properly in this example.\n");
    printf("----------------------------END TEST 5----------------------------\n");
}

void test6() {
    printf("----------TEST 6 (FIRST_FIT): ALLOCATE 10 CHUNKS OF MEMORY----------\n");
    printf("Request 5000 bytes of memory from the OS using umeminit(5000, FIRST_FIT).\n");
    printStatusOfUmeminit(umeminit(5000, FIRST_FIT));
    umemdump();
    printf("\n");
    printf("Allocate 10 chunks of memory with different sizes.\n");
    int memorySizes[] = {280, 160, 400, 320, 750, 1000, 820, 120, 700, 3050};
    void *ptrs[10];
    for (int i = 0; i < 10; i++) {
        printStatusOfUmalloc(ptrs[i] = umalloc(memorySizes[i]));
    }
    printf("\n");
    printf("Now free some memory to test.\n");
    printStatusOfUfree(ufree(ptrs[4]));
    printStatusOfUfree(ufree(ptrs[2]));
    printStatusOfUfree(ufree(ptrs[6]));
    printStatusOfUfree(ufree(ptrs[8]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("After free 4 memory node, without coalescing, ptrs index 4, 2, 6, 8 not next to each other.\n");
    printf("There are 5 nodes of free memory, 4 freed nodes and the rest node.\n");
    printf("\n");
    printf("Test if FIRST_FIT is working properly, allocate 700 bytes of memory\n");
    printStatusOfUmalloc(umalloc(700));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("When allocate 700 bytes of memory, the free node with 752 bytes is replaced with a smaller size node in the free list.\n");
    printf("The new node after replacing has size of 752 - 704 - 16 = 32 bytes.\n");
    printf("The algorithm doesn't choose 824 bytes (WORST_FIT) or 704 bytes (BEST_FIT) node.\n");
    printf("The free list has 5 nodes (752 bytes node is replaced with a smaller size 32 bytes node).\n");
    printf("FIRST_FIT is working properly as 752 bytes is the first fit node in the list.\n");
    printf("\n");
    printf("Let's allocate 32 bytes of memory.");
    printStatusOfUmalloc(umalloc(32));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("The free list head (400 bytes) is expected to be split as it suitable for the requested memory and it's the first node.\n");
    printf("32 bytes node (best fit) or 824 bytes node (worst fit) should not be choose.\n");
    printf("The size of the new node should be 400 - 32 - 16 = 352 and it is should be the free list head.\n");
    printf("FIRST_FIT is working properly.\n");
    printf("----------------------------END TEST 6----------------------------\n");
}

void test7() {
    printf("----------TEST 7 (NEXT_FIT): ALLOCATE 15 CHUNKS OF MEMORY----------\n");
    printf("Request 5000 bytes of memory from the OS using umeminit(5000, NEXT_FIT).\n");
    printStatusOfUmeminit(umeminit(5000, NEXT_FIT));
    umemdump();
    printf("\n");
    printf("Allocate 15 chunks of memory with different sizes.\n");
    int memorySizes[] = {280, 160, 400, 320, 550, 1000, 620, 120, 700, 750, 1000, 500, 350, 470, 300};
    void *ptrs[15];
    for (int i = 0; i < 15; i++) {
        printStatusOfUmalloc(ptrs[i] = umalloc(memorySizes[i]));
    }
    printf("\n");
    printf("Now free some memory to test.\n");
    printStatusOfUfree(ufree(ptrs[4]));
    printStatusOfUfree(ufree(ptrs[2]));
    printStatusOfUfree(ufree(ptrs[6]));
    printStatusOfUfree(ufree(ptrs[8]));
    printStatusOfUfree(ufree(ptrs[10]));
    printStatusOfUfree(ufree(ptrs[12]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("Now allocate 450 bytes of memory.\n");
    printStatusOfUmalloc(umalloc(450));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("The previous next free node has size 392 which doesn't fit 450 requested memory.\n");
    printf("The previous next free node is also the last node, so it will continue searching from the free list head.\n");
    printf("The first node fit 450 bytes is the node with 552 bytes free, so splitting.\n");
    printf("Now, the next free node is the new node we just split from 552 bytes node.\n");
    printf("450 requested memory is round up to 456 + 16 bytes header for new node, so the new node now has 80 bytes free (552 - 456 - 16 = 80).\n");
    printf("\n");
    printf("Now, let allocate 400 bytes from the memory.\n");
    printStatusOfUmalloc(umalloc(400));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("The algorithm doesn't choose 400 bytes node (first or best fit) or 1000 bytes node (worst fit).\n");
    printf("Instead, it choose the node with 624 bytes to splits, this is the first fit node when start from the nextFreeNode, the 80 bytes node.\n");
    printf("The next free node is now the new 208 bytes node split from the 624 bytes node.\n");
    printf("NEXT_FIT is working properly up until now.\n");
    printf("\n");
    printf("Now, let allocate another 80 bytes from the memory.\n");
    printStatusOfUmalloc(umalloc(80));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("The algorithm doesn't choose 400 bytes node (first fit) or 1000 bytes node (worst fit) or 80 bytes node (best fit).\n");
    printf("Instead, it choose the node with 208 bytes to split, this node is the next free node from last allocation and it does fit the request.\n");
    printf("Now, the next free node is now the new 112 (208 - 80 - 16) bytes node split from the 208 bytes node.\n");
    printf("NEXT_FIT is working properly up until now.\n");
    printf("----------------------------END TEST 7----------------------------\n");
}

void test8() {
    printf("--------------TEST 8 (BEST_FIT): UFREE AND COALESCING--------------\n");
    printf("Request 5000 bytes of memory from the OS using umeminit(5000, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(5000, BEST_FIT));
    umemdump();
    printf("\n");
    printf("Allocate 12 chunks of memory with different sizes.\n");
    int memorySizes[] = {280, 400, 320, 550, 1000, 620, 700, 750, 500, 350, 470, 300};
    void *ptrs[12];
    for (int i = 0; i < 12; i++) {
        printStatusOfUmalloc(ptrs[i] = umalloc(memorySizes[i]));
    }
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("Now free the first memory node (280 bytes).\n");
    printStatusOfUfree(ufree(ptrs[0]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("There is 2 node of memory in the free list, 280 bytes node and 1720 node, which is expected.\n");
    printf("\n");
    printf("Now free the second memory node (400 bytes).\n");
    printStatusOfUfree(ufree(ptrs[1]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("The first memory node and the second memory node is next to each other, so they are coalescing.\n");
    printf("The first node in the free list is has a bigger size now.\n");
    printf("280 (first node) + 16 (header of second node) + 400 (second node) = 696 bytes.\n");
    printf("Coalescing is working properly for now.\n");
    printf("\n");
    printf("Let's free the last allocated memory (saved at ptrs[11], requested size is 300).\n");
    printStatusOfUfree(ufree(ptrs[11]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("The last allocated memory node and the last free node in the free list are next to each other, so they are coalescing.\n");
    printf("The last node in the free list is has a bigger size now.\n");
    printf("304 (300 round up to multiple of 8 - last allocated node) + 16 (header of last free node) + 1720 (size of last free node) = 2040 bytes.\n");
    printf("Coalescing is working properly for now.\n");
    printf("\n");
    printf("Let's free ptrs[5], requested size is 620.\n");
    printStatusOfUfree(ufree(ptrs[5]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("ptrs[5] is allocated after ptrs[0] (already coalescing with ptrs[1]) and ptrs[11] (merged to one big chunk at the last free list).\n");
    printf("620 round up to multiple of 8 is 624, it's expected to have a 624 bytes node that in between the first free node and the last free node.\n");
    printf("Up until now, UFREE is working properly in term of put back the freed node and coalescing.\n");
    printf("\n");
    printf("Let's free ptrs[7], requested size is 750.\n");
    printStatusOfUfree(ufree(ptrs[7]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("Let's free ptrs[6], requested size is 700.\n");
    printStatusOfUfree(ufree(ptrs[6]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("After free ptrs[7], there are 4 nodes in the free list, the middle 2 is ptrs[5] and ptrs[7] just free.\n");
    printf("The free ptrs[6], as ptrs[5] and ptrs[6] and ptrs[7] are continuous, they should be coalescing.\n");
    printf("ptrs[5] has size 624, ptrs[7] was requested with 750 and was round up to 752, ptrs[6] is 700 and round to 704.\n");
    printf("The new node after coalescing has size of 624 + 16 + 752 + 16 + 704 = 2112 bytes.\n");
    printf("There are 3 nodes in free list, coalescing still work well.\n");
    printf("\n");
    printf("Let's free ptrs[3] (550) and print out the memory dump.\n");
    printf("It's expected to have a node of size 550 round up after 696 node and before 2112 node.\n");
    printStatusOfUfree(ufree(ptrs[3]));
    umemdump();
    printf("\n");
    printf("Everything is working properly up until now.\n");
    printf("----------------------------END TEST 8----------------------------\n");
}

void test9() {
    printf("------TEST 9 (BEST_FIT): UFREE COALESCING AND UPDATE NEXT FREE NODE------\n");
    printf("Request 4000 bytes of memory from the OS using umeminit(4000, NEXT_FIT).\n");
    printStatusOfUmeminit(umeminit(4000, NEXT_FIT));
    umemdump();
    printf("\n");
    printf("Allocate 5 chunks of memory with different sizes.\n");
    int memorySizes[] = {280, 160, 400, 320, 550};
    void *ptrs[5];
    for (int i = 0; i < 5; i++) {
        printStatusOfUmalloc(ptrs[i] = umalloc(memorySizes[i]));
    }
    printf("\n");
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("Let's free the last allocated memory at ptrs[4]\n");
    printf("The current next free node pointer is now point to the only free node, which is the rest memory after allocated.\n");
    printf("Since ptrs[4] and the next free node is next to each other, the next free node pointer should point to the ptrs[4] after coalescing.\n");
    printf("Address of ptrs[4]: %p.\n", ptrs[4]);
    printStatusOfUfree(ufree(ptrs[4]));
    umemdump();
    printf("\n");
    printf("Let's free ptrs[2], there should be a new free node as the just freed node is not next to the other node.\n");
    printStatusOfUfree(ufree(ptrs[2]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("As there is no coalescing with next free node from last allocation pointer, this pointer keep as is\n.");
    printf("\n");
    printf("Let's free ptrs[3], It's expected to have only 1 free node of memory.");
    printf("Since ptrs[2] is free, ptrs[3] is just freed, and the rest free node are neighbor.\n");
    printf("It's expected to see them coalescing and the next free node pointer should have same address as ptrs[2].\n");
    printf("Address of ptrs[2]: %p.\n", ptrs[2]);
    printStatusOfUfree(ufree(ptrs[3]));
    umemdump();
    printf("\n");
    printf("The result comes out as expected!\n");
    printf("----------------------------END TEST 9----------------------------\n");
}

void test10() {
    printf("----------------TEST 10 (BEST_FIT): UFREE SAME PTRS----------------\n");
    printf("Request 1 bytes of memory from the OS using umeminit(1, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(1, BEST_FIT));
    umemdump();
    printf("\n");
    printf("Allocate one huge chunk of memory that equal to the size of the free block.\n");
    void *ptr = umalloc(4080);
    printStatusOfUmalloc(ptr);
    umemdump();
    printf("When all memory is allocated, there isn't a free memory block in the memory dump!\n");
    printf("The address of the memory dump before umalloc and the address return from umalloc is the SAME!\n");
    printf("\n");
    printf("Now let's try to free that big node two times.\n");
    printStatusOfUfree(ufree(ptr));
    printStatusOfUfree(ufree(ptr));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("----------------------------END TEST 10----------------------------\n");
}

void test11() {
    printf("-------------TEST 11 (BEST_FIT): UFREE SAME PTRS (CONT)-------------\n");
    printf("Request 1 bytes of memory from the OS using umeminit(1, BEST_FIT).\n");
    printStatusOfUmeminit(umeminit(1, BEST_FIT));
    umemdump();
    printf("\n");
    printf("Allocate 5 chunks of memory with different sizes.\n");
    int memorySizes[] = {280, 160, 400, 320, 550};
    void *ptrs[5];
    for (int i = 0; i < 5; i++) {
        printStatusOfUmalloc(ptrs[i] = umalloc(memorySizes[i]));
    }
    printf("\n");
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("Now let's try to free ptrs[1] and ptrs[3].\n");
    printStatusOfUfree(ufree(ptrs[1]));
    printStatusOfUfree(ufree(ptrs[3]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("Now let's try to free these just freed node multiples times.\n");
    printStatusOfUfree(ufree(ptrs[1]));
    printStatusOfUfree(ufree(ptrs[3]));
    printStatusOfUfree(ufree(ptrs[1]));
    printStatusOfUfree(ufree(ptrs[3]));
    printf("And print out the memory dump.\n");
    umemdump();
    printf("\n");
    printf("It's not possible to free a node that already free.\n");
    printf("The memory dump is still has 3 nodes.\n");
    printf("----------------------------END TEST 11----------------------------\n");
}