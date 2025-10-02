#include "memoryManager.h"
#include <defs.h>
#include <stdint.h>

#define BLOCK_SIZE 64 // Bloques de 64 bytes

enum { FREE, USED, START_BOUNDARY, END_BOUNDARY, SINGLE_BLOCK };

typedef struct MemoryManagerCDT {
    void *start;   // Comienzo de memoria 
    uint32_t blockQty;	// Cantidad de bloques
    uint32_t blocksUsed;	// Bloques usados 
    uint32_t *bitmap;
    uint32_t current; // Posicion actual del bitmap
} MemoryManagerCDT;

static sizeToBlockQty(uint32_t size);
static void *markGroupAsUsed(MemoryManagerADT memoryManager, uint32_t blocksNeeded, uint32_t index);
static uintptr_t findFreeBlocks(MemoryManagerADT memoryManager, uint32_t blocksNeeded, uint32_t start, uint32_t end);

static sizeToBlockQty(uint32_t size){
	return (size == 0) ? 0 : (size - 1) / BLOCK_SIZE + 1;
}

static uintptr_t findFreeBlocks(MemoryManagerADT memoryManager, uint32_t blocksNeeded, uint32_t start, uint32_t end) {
    uint32_t freeBlocks = 0;
    uint32_t i;
    
    for (i = start; i < end; i++) {
        if (memoryManager->bitmap[i] == FREE) {
            freeBlocks++;
            if (freeBlocks == blocksNeeded) {
                return (uintptr_t)(memoryManager->start) + (i - blocksNeeded + 1) * BLOCK_SIZE;
            }
        } else {
            freeBlocks = 0;
        }
    }
    
    if (freeBlocks == blocksNeeded) {
        return (uintptr_t)(memoryManager->start) + (i - blocksNeeded) * BLOCK_SIZE;
    }
    
    return 0;
}

static void *markGroupAsUsed(MemoryManagerADT memoryManager, uint32_t blocksNeeded, uint32_t index) {
    if (blocksNeeded == 1) {
        memoryManager->bitmap[index] = SINGLE_BLOCK;
        memoryManager->blocksUsed++;
    } else {
        memoryManager->bitmap[index] = START_BOUNDARY;
        memoryManager->bitmap[index + blocksNeeded - 1] = END_BOUNDARY;
        
        for (uint32_t i = 1; i < blocksNeeded - 1; i++) {
            memoryManager->bitmap[index + i] = USED;
        }
        
        memoryManager->blocksUsed += blocksNeeded;
    }
    
    return (void *)((char *)memoryManager->start + index * BLOCK_SIZE);
}

MemoryManagerADT createMemoryManager(void *const restrict memoryForMemoryManager, void *const restrict managedMemory) {
	MemoryManagerADT memoryManager = (MemoryManagerADT) memoryForMemoryManager;

	uint32_t totalMemory = HEAP_SIZE;
	memoryManager->blockQty = totalMemory / BLOCK_SIZE;

	uint32_t bitmapSizeInBytes = memoryManager->blockQty * sizeof(uint32_t);
    uint32_t bitmapSizeInBlocks = (bitmapSizeInBytes + BLOCK_SIZE - 1) / BLOCK_SIZE;

	memoryManager->bitmap = (void*)HEAP_ADDRESS;
    memoryManager->start = (void*)HEAP_ADDRESS;

	memoryManager->blocksUsed = 0;
	memoryManager->current = 0;

	// inicializar bitmap

	for(uint32_t i = 0; i < memoryManager->blockQty; i++) {
		memoryManager->bitmap[i] = FREE;
	}
	
	return memoryManager;
}

void *allocMemory(MemoryManagerADT const restrict memoryManager, const size_t memoryToAllocate) {
    uint32_t blocksNeeded = sizeToBlockQty(memoryToAllocate);
    
    if (blocksNeeded == 0 || blocksNeeded > memoryManager->blockQty - memoryManager->blocksUsed) {
        return NULL;
    }
   
    uintptr_t initialBlockAddress = findFreeBlocks(
        memoryManager, blocksNeeded, memoryManager->current, memoryManager->blockQty);
    
    if (initialBlockAddress == 0) {
        initialBlockAddress = findFreeBlocks(
            memoryManager, blocksNeeded, 0, memoryManager->current);
    }
    
    if (initialBlockAddress == 0) {
        return NULL;
    }
    
    uint32_t blockIndex = (initialBlockAddress - (uintptr_t)memoryManager->start) / BLOCK_SIZE;
    memoryManager->current = (blockIndex + blocksNeeded) % memoryManager->blockQty;
    
    return markGroupAsUsed(memoryManager, blocksNeeded, blockIndex);
}

void freeMemory(MemoryManagerADT const restrict memoryManager, void *address) {
    if (address == NULL || (uintptr_t)address % BLOCK_SIZE != 0) {
        return;
    }
    
    // Calculate block index
    uintptr_t blockAddress = (uintptr_t)address;
    if (blockAddress < (uintptr_t)memoryManager->start) {
        return;
    }
    
    uint32_t blockIndex = (blockAddress - (uintptr_t)memoryManager->start) / BLOCK_SIZE;
    
    // Check if it's a single block allocation
    if (memoryManager->bitmap[blockIndex] == SINGLE_BLOCK) {
        memoryManager->bitmap[blockIndex] = FREE;
        memoryManager->blocksUsed--;
        return;
    }
    
    // Check if it's the start of a multi-block allocation
    if (memoryManager->bitmap[blockIndex] != START_BOUNDARY) {
        return;
    }
    
    // Free all blocks in the allocation
    uint32_t blocksToFree = 0;
    while (blockIndex + blocksToFree < memoryManager->blockQty && 
           memoryManager->bitmap[blockIndex + blocksToFree] != END_BOUNDARY) {
        memoryManager->bitmap[blockIndex + blocksToFree] = FREE;
        blocksToFree++;
    }
    
    // Free the end boundary block
    if (blockIndex + blocksToFree < memoryManager->blockQty) {
        memoryManager->bitmap[blockIndex + blocksToFree] = FREE;
        memoryManager->blocksUsed -= blocksToFree + 1;
    }
}