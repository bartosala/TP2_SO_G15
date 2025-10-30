#ifdef BUDDY
#include <stdio.h>
#include <stdint.h>
#include "memoryManager.h"
#include <defs.h>
#include <textModule.h>

// Match reference implementation - use defines and uint8_t
#define FREE 0
#define SPLIT 1
#define USED 2

#define POW_TWO(x) ((uint64_t) 1 << (x))
#define MIN_EXPONENT 6
#define MIN_ALLOC POW_TWO(MIN_EXPONENT)
#define MAX_EXPONENT 28
#define MAX_ALLOC POW_TWO(MAX_EXPONENT)
#define NODES (POW_TWO(MAX_EXPONENT - MIN_EXPONENT + 1) - 1)

typedef struct {
    uint8_t use;  // uint8_t instead of enum!
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
    
    printStr("Creating memory manager at: 0x", 0x00FFFFFF);
    printInt((uint64_t)startAddress, 0x00FFFFFF);
    printStr("\n", 0x00FFFFFF);

    if (memorySize < MIN_ALLOC) {
        printStr("Tamaño de memoria insuficiente\n", 0x00FF0000);
        return;
    }

    memoryManager->firstAddress = startAddress + sizeof(MemoryManagerCDT);
    memoryManager->tree = (Node *)(memoryManager->firstAddress + HEAP_SIZE);

    memoryManager->totalMemory = HEAP_SIZE;
    memoryManager->usedMemory = 0;

    printStr("Heap starts at: 0x", 0x00FFFFFF);
    printInt((uint64_t)memoryManager->firstAddress, 0x00FFFFFF);
    printStr("\nTree starts at: 0x", 0x00FFFFFF);
    printInt((uint64_t)memoryManager->tree, 0x00FFFFFF);
    printStr("\nTotal memory: ", 0x00FFFFFF);
    printInt(memoryManager->totalMemory, 0x00FFFFFF);
    printStr(" bytes\n", 0x00FFFFFF);
    
    memset(memoryManager->tree, 0, NODES * sizeof(Node));
    printStr("Memory manager initialized!\n", 0x00FFFFFF);
}

void *allocMemory(const size_t memoryToAllocate) {
    if (memoryManager == NULL || memoryToAllocate == 0) {
        printStr("ERROR: manager NULL or size 0\n", 0x00FF0000);
        return NULL;
    }

    if (memoryToAllocate > memoryManager->totalMemory) {
        printStr("ERROR: Requested size exceeds total memory\n", 0x00FF0000);
        return NULL;
    }

    // Don't add +1 - metadata is stored INSIDE the block
    uint8_t exponent = getExponent(memoryToAllocate);
    if (exponent > MAX_EXPONENT) {
        printStr("ERROR: Exponent too large: ", 0x00FF0000);
        printInt(exponent, 0x00FF0000);
        printStr("\n", 0x00FF0000);
        return NULL;
    }

    int64_t node = lookFreeNode(exponent);
    if (node == -1) {
        printStr("ERROR: No free node for size ", 0x00FF0000);
        printInt(memoryToAllocate, 0x00FF0000);
        printStr(" (exponent: ", 0x00FF0000);
        printInt(exponent, 0x00FF0000);
        printStr(", used: ", 0x00FF0000);
        printInt(memoryManager->usedMemory, 0x00FF0000);
        printStr(")\n", 0x00FF0000);
        return NULL;
    }

    printStr("Before marking: node ", 0x00FFFF00);
    printInt(node, 0x00FFFF00);
    printStr(" state=", 0x00FFFF00);
    printInt(memoryManager->tree[node].use, 0x00FFFF00);
    printStr("\n", 0x00FFFF00);

    memoryManager->tree[node].use = USED;
    
    printStr("After marking: node ", 0x00FFFF00);
    printInt(node, 0x00FFFF00);
    printStr(" state=", 0x00FFFF00);
    printInt(memoryManager->tree[node].use, 0x00FFFF00); // Aca se podira imprimir USED en vez de 2 , osea el estado en palabras. Igual creo
                                                            // que ni hace falta poner nada 
    printStr("\n", 0x00FFFF00);
    
    setSplit(node);

    memoryManager->usedMemory += POW_TWO(exponent);

    uint8_t *block = address(node, exponent);
    block[0] = exponent;
    
    printStr("Allocated: size=", 0x0000FF00);
    printInt(memoryToAllocate, 0x0000FF00);
    printStr(", block=0x", 0x0000FF00);
    printInt((uint64_t)(block + 1), 0x0000FF00);
    printStr(", used=", 0x0000FF00);
    printInt(memoryManager->usedMemory, 0x0000FF00);
    printStr("\n", 0x0000FF00);
    
    return (void *)(block + 1);
}

void freeMemory(void *address) {
    if (memoryManager == NULL || address == NULL) {
        printStr("Free: NULL pointer\n", 0x00FFFF00);
        return;
    }
    
    uint8_t *real_ptr = (uint8_t *)address - 1;
    uint8_t exponent = real_ptr[0];
    uint64_t node = searchNode(real_ptr, &exponent);

    if (memoryManager->tree[node].use != USED) {
        printStr("Free: block not marked as USED\n", 0x00FF0000);
        return;
    }
    
    releaseNode(node);
    memoryManager->usedMemory -= POW_TWO(exponent);
    
    printStr("Freed: addr=0x", 0x00FFFF00);
    printInt((uint64_t)address, 0x00FFFF00);
    printStr(", used=", 0x00FFFF00);
    printInt(memoryManager->usedMemory, 0x00FFFF00);
    printStr("\n", 0x00FFFF00);
}


static inline uint64_t getNode(uint8_t exponent) {
    return POW_TWO(MAX_EXPONENT - exponent) - 1;
}

static uint8_t *address(uint64_t node, uint8_t exponent) {
    // Match reference implementation exactly
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
    // Match reference: check if node.use == 0
    if (memoryManager->tree[node].use != 0) {
        return 0;
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
        if (memoryManager->tree[node].use == 0) {  // Use 0 instead of FREE
            memoryManager->tree[node].use = SPLIT;
        } else {
            break;
        }
    }
}static void releaseNode(uint64_t node) {
    memoryManager->tree[node].use = 0;

    if (node == 0) {
        return;
    }
    
    while (node != 0) {
        uint64_t parent = (node - 1) >> 1;
        uint64_t left = (parent << 1) + 1;
        uint64_t right = (parent << 1) + 2;

        if (memoryManager->tree[left].use == 0 && memoryManager->tree[right].use == 0) {
            memoryManager->tree[parent].use = 0;
            node = parent;
        } else {
            break;
        }
    }
}

#endif