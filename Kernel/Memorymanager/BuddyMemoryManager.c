#ifdef BUDDY
#include <stdio.h>
#include <stdint.h>
#include "memoryManager.h"
#include <defs.h>
#include <string.h>

enum NodeState { FREE, SPLIT, USED };

#define POW_TWO(x) ((uint64_t) 1 << (x))
#define MIN_EXPONENT 6
#define MIN_ALLOC POW_TWO(MIN_EXPONENT)
#define MAX_EXPONENT 28
#define MAX_ALLOC POW_TWO(MAX_EXPONENT)
#define NODES (POW_TWO(MAX_EXPONENT - MIN_EXPONENT + 1) - 1)

typedef struct {
    enum NodeState use;
} Node;

typedef struct MemoryManagerCDT {
    uint8_t *firstAddress;
    Node *tree;
    uint64_t totalMemory;
    uint64_t usedMemory;
} MemoryManagerCDT;

static MemoryManagerADT memoryManager = NULL;

static uint64_t getNode(uint8_t exponent);
static int64_t lookFreeNode(uint8_t exponent);
static uint8_t *address(uint64_t node, uint8_t exponent);
static uint8_t getExponent(uint64_t request);
static int64_t searchNode(uint8_t *ptr, uint8_t *exponent);
static void setSplit(uint64_t node);
static void releaseNode(uint64_t node);
static int isAllocatable(uint64_t node);

void createMemoryManager(void *startAddress, size_t memorySize) {
    memoryManager = startAddress;

    if (memorySize < MIN_ALLOC) {
        return;
    }

    memoryManager->firstAddress = (uint8_t*)HEAP_ADDRESS;
    memoryManager->tree = (Node *)(memoryManager->firstAddress + HEAP_SIZE);

    memoryManager->totalMemory = HEAP_SIZE;
    memoryManager->usedMemory = 0;

    
    memset(memoryManager->tree, 0, NODES * sizeof(Node));
}

void *allocMemory(const size_t memoryToAllocate) {
    if (memoryManager == NULL || memoryToAllocate == 0) {
        return NULL;
    }

    if (memoryToAllocate > memoryManager->totalMemory) {
        return NULL;
    }

    uint8_t exponent = getExponent(memoryToAllocate + 1);
    if (exponent > MAX_EXPONENT) {
        return NULL;
    }

    int64_t node = lookFreeNode(exponent);
    if (node == -1) {
        return NULL;
    }

    memoryManager->tree[node].use = USED;
    setSplit(node);

    memoryManager->usedMemory += POW_TWO(exponent);

    uint8_t *block = address(node, exponent);
    block[0] = exponent;
    return (void *)(block + 1);
}

void freeMemory(void *address) {
    if (memoryManager == NULL || address == NULL) {
        return;
    }
    
    uint8_t *real_ptr = (uint8_t *)address - 1;
    uint8_t exponent = real_ptr[0];
    uint64_t node = searchNode(real_ptr, &exponent);

    if (memoryManager->tree[node].use != USED) {
        return;
    }
    
    releaseNode(node);
    memoryManager->usedMemory -= POW_TWO(exponent);
}


static inline uint64_t getNode(uint8_t exponent) {
    return POW_TWO(MAX_EXPONENT - exponent) - 1;
}

static uint8_t *address(uint64_t node, uint8_t exponent) {
    return memoryManager->firstAddress + ((node - getNode(exponent)) << exponent);
}

static int64_t searchNode(uint8_t *ptr, uint8_t *exponent) {
    uint64_t node = ((ptr - memoryManager->firstAddress) >> *exponent) + getNode(*exponent);
    return node;
}

static uint8_t getExponent(uint64_t request) {
    uint8_t exponent = MIN_EXPONENT;
    uint64_t size = MIN_ALLOC;
    while (size < request) {
        exponent++;
        size <<= 1;
    }
    return exponent;
}

static int isAllocatable(uint64_t node) {
    if (memoryManager->tree[node].use != FREE) {
        return 0;  // Node is not free
    }
    
    uint64_t current = node;
    while (current != 0) {
        current = (current - 1) >> 1;
        if (memoryManager->tree[current].use == USED) {
            return 0;
        }
    }
    return 1;
}

static int64_t lookFreeNode(uint8_t exponent) {
    uint64_t node = getNode(exponent);
    uint64_t lastnode = getNode(exponent - 1);
    
    while (node < lastnode) {
        if (isAllocatable(node)) {
            return node;
        }
        node++;
    }
    return -1;
}

static void setSplit(uint64_t node) {
    while (node != 0) {
        node = ((node - 1) >> 1);
        if (memoryManager->tree[node].use == FREE) {
            memoryManager->tree[node].use = SPLIT;
        } else {
            break;  
        }
    }
}

static void releaseNode(uint64_t node) {
    memoryManager->tree[node].use = FREE;

    if (node == 0) {
        return;
    }
    
    while (node != 0) {
        uint64_t parent = (node - 1) >> 1;
        uint64_t left = (parent << 1) + 1;
        uint64_t right = (parent << 1) + 2;

        if (memoryManager->tree[left].use == FREE && memoryManager->tree[right].use == FREE) {
            memoryManager->tree[parent].use = FREE;
            node = parent;
        } else {
            break;
        }
    }
}

#endif