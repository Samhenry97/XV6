//cow.c: handler for copy-on-write by keeping track of how many references there are to a page
#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "cow.h"

#define COUNTPART 0xFFF
#define DATA_PER_PAGE 1023

//struct that should be size 4096, fitting perfectly in one page of memory
//each uint in the data is organized as follows
// +----------------20---------------+---------12----------+
// |      Upper Address of Page      | References to this  |
// |                                 |         page        |
// +----------------+----------------+---------------------+
// This data structure is a linked list of cowRefs with each cowRef
// holding an array of uints described above. An empty spot that used 
// to be filled is filled with a zero, and the end of the list is marked with 0xFFFFFFFF
struct cowRefs {
    uint data[DATA_PER_PAGE];
    struct cowRefs* next;
};

//The structure is organized as a linked list, starting at firstPage
struct cowRefs* firstPage = 0;

//find the address where page is within the linked list
uint* searchPage(uint page) {
    if (firstPage != 0) {
        struct cowRefs* curPage = firstPage;
        do {
            for (uint i = 0; i < DATA_PER_PAGE; i++) {
                uint cur = curPage->data[i];
                if (cur == 0xFFFFFFFF) {
                    //end marker
                    return 0;
                } else if (cur == 0) {
                    continue;
                } else if ((cur & ~COUNTPART) == page) {
                    return curPage->data + i;
                }
            }
            curPage = curPage->next;
        } while (curPage->next);
    }
    return 0;
}

//allocate a new page to store a cowRef
struct cowRefs* allocCow() {
    struct cowRefs* ans = (struct cowRefs*) kalloc();
    if (ans == 0) {
        panic("Not enough memory to implement COW");
    }
    ans->next = 0;
    for (uint i = 0; i < DATA_PER_PAGE; i++) {
        ans->data[i] = 0;
    }
    return ans;
}

// find the next open block, whether it's one that was used before or if we have to create a new one
uint* openBlock() {
    if (firstPage != 0) {
        struct cowRefs* curPage = firstPage;
        do {
            for (uint i = 0; i < DATA_PER_PAGE; i++) {
                uint cur = curPage->data[i];
                if (cur == 0xFFFFFFFF) {
                    if (i == DATA_PER_PAGE - 1) {
                        curPage->next = allocCow();
                        curPage->next->data[0] = 0xFFFFFFFF;
                    } else {
                        curPage->data[i + 1] = 0xFFFFFFFF;
                    }
                    return curPage->data + i;
                } else if (cur == 0) {
                    return curPage->data + i;
                }
            }
            curPage = curPage->next;
        } while (curPage->next);
    } else {
        firstPage = allocCow();
        firstPage->data[0] = 0xFFFFFFFF;
        return firstPage->data;
    }
    panic("There's something very wrong with the COW structure.\n");
}

//increase the references to the block for this page
void cowUp(uint page) {
    uint* data = searchPage(page);
    if (data == 0) {
        panic("CowUp: Non-existant COW reference\n");
    }
    int count = *data & COUNTPART;
    if (count >= COUNTPART - 1) {
        panic("Too many COW references to the same block\n");
    }
    (*data) ++;
}

//decrease the references to the block for this page
//if there's only one reference left, remove the block
void cowDown(uint page) {
    uint* data = searchPage(page);
    if (data == 0) {
        cprintf(ALL_DEVS, "Cow: bad     page %x\n", page);
        panic("CowDown: Non-existant COW reference\n");
    }
    int count = *data & COUNTPART;
    if (count == 0) {
        panic("Cannot subtract references from 0-reference COW block\n");
    } else if (count == 0) {
        *data = 0;
        kfree((char*) page);
    } else {
        (*data) --;
    }
}

//get the number of references to this block
int getCowRefs(uint page) {
    uint* data = searchPage(page);
    if (data == 0) {
        panic("CowRefs: Non-existant COW reference\n");
    }
    return (*data) & COUNTPART;
}

//set the number of references to this page
void cowSet(uint page, uint value) {
    cprintf(ALL_DEVS, "Cow: setting page %x\n", page);
    if (value >= COUNTPART) {
        panic("Too many COW references to the same block\n");
    }
    uint* data = searchPage(page);
    if (data == 0) {
        data = openBlock();
    }
    *data = page + value;
}