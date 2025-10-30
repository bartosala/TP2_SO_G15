#ifdef BITMAP
#include "memoryManager.h"
#include <defs.h>
#include <stdint.h>

enum { FREE, USED, START_BOUNDARY, END_BOUNDARY, SINGLE_BLOCK };

typedef struct MemoryManagerCDT {
    void *start;   // Comienzo de memoria 
    uint32_t blockQty;	// Cantidad de bloques
    uint32_t blocksUsed;	// Bloques usados 
    uint32_t *bitmap;
    uint32_t current; // Posicion actual del bitmap
} MemoryManagerCDT;

static void *startingAddress;
MemoryManagerCDT memoryManager;

static uint32_t sizeToBlockQty(uint32_t size);
static void *markGroupAsUsed(uint32_t blocksNeeded, uint32_t index);
static uintptr_t findFreeBlocks(uint32_t blocksNeeded, uint32_t start, uint32_t end);

static uint32_t sizeToBlockQty(uint32_t size){
	return (size == 0) ? 0 : (size - 1) / BLOCK_SIZE + 1;
}

static uintptr_t findFreeBlocks(uint32_t blocksNeeded, uint32_t start, uint32_t end) {
    uint32_t freeBlocks = 0;
    uint32_t i;
    
    for (i = start; i < end; i++) {
        if (memoryManager.bitmap[i] == FREE) {
            freeBlocks++;
            if (freeBlocks == blocksNeeded) {
                return (uintptr_t)(memoryManager.start) + (i - blocksNeeded + 1) * BLOCK_SIZE;
            }
        } else {
            freeBlocks = 0;
        }
    }
    
    if (freeBlocks == blocksNeeded) {
        return (uintptr_t)(memoryManager.start) + (i - blocksNeeded) * BLOCK_SIZE;
    }
    
    return 0;
}

static void *markGroupAsUsed(uint32_t blocksNeeded, uint32_t index) {
    if (blocksNeeded == 1) {
        memoryManager.bitmap[index] = SINGLE_BLOCK;
        memoryManager.blocksUsed++;
    } else {
        memoryManager.bitmap[index] = START_BOUNDARY;
        memoryManager.bitmap[index + blocksNeeded - 1] = END_BOUNDARY;
        
        for (uint32_t i = 1; i < blocksNeeded - 1; i++) {
            memoryManager.bitmap[index + i] = USED;
        }
        
        memoryManager.blocksUsed += blocksNeeded;
    }
    
    return (void *)((char *)memoryManager.start + index * BLOCK_SIZE);
}

void createMemoryManager(void *start, size_t size) { // Podria ser sin parametros esta todo en defs
    startingAddress = start;
	MemoryManagerADT memoryManager = (MemoryManagerADT)startingAddress;
	size_t totalMemory = size;
	memoryManager->blockQty = totalMemory / BLOCK_SIZE; // ESTO DA 0 !!!
    int d = memoryManager->blockQty;

	memoryManager->bitmap = startingAddress;
    memoryManager->start = startingAddress;

	memoryManager->blocksUsed = 0;
	memoryManager->current = 0;

	// inicializar bitmap

	for(uint32_t i = 0; i < memoryManager->blockQty; i++) {
		memoryManager->bitmap[i] = FREE;
	}
	
}

void *allocMemory(const size_t memoryToAllocate) {
    uint32_t blocksNeeded = sizeToBlockQty(memoryToAllocate);
    // el probelma es que blockqty es 0 
    int d = memoryManager.blockQty;
    if (blocksNeeded == 0 /*|| blocksNeeded > memoryManager.blockQty - memoryManager.blocksUsed */) {
        return NULL; // DEVUELVE ESTE NULL POR LA SEGUNDA PROP, DA 1 > 0 !!! 
    
    }
    
    
   
    uintptr_t initialBlockAddress = findFreeBlocks(
        blocksNeeded, memoryManager.current, memoryManager.blockQty);
    
    if (initialBlockAddress == 0) {
        initialBlockAddress = findFreeBlocks(
            blocksNeeded, 0, memoryManager.current);
    }
    
    if (initialBlockAddress == 0) {
        return NULL; // ESTO TAMBIE DA NULL !!!! ESOS SON LOS PROBLEMAS CON EL BITMAP
    }
    
    uint32_t blockIndex = (initialBlockAddress - (uintptr_t)memoryManager.start) / BLOCK_SIZE;
    memoryManager.current = (blockIndex + blocksNeeded) % memoryManager.blockQty;
    
    return markGroupAsUsed(blocksNeeded, blockIndex);
}

void freeMemory(void *address) {
    if (address == NULL || (uintptr_t)address % BLOCK_SIZE != 0) {
        return;
    }
    

    uintptr_t blockAddress = (uintptr_t)address;
    if (blockAddress < (uintptr_t)memoryManager.start) {
        return;
    }
    
    uint32_t blockIndex = (blockAddress - (uintptr_t)memoryManager.start) / BLOCK_SIZE;
    
    if (memoryManager.bitmap[blockIndex] == SINGLE_BLOCK) {
        memoryManager.bitmap[blockIndex] = FREE;
        memoryManager.blocksUsed--;
        return;
    }

        
    if (memoryManager.bitmap[blockIndex] != START_BOUNDARY) {
        return;
    }
    
    uint32_t blocksToFree = 0;
    while (blockIndex + blocksToFree < memoryManager.blockQty && 
           memoryManager.bitmap[blockIndex + blocksToFree] != END_BOUNDARY) {
        memoryManager.bitmap[blockIndex + blocksToFree] = FREE;
        blocksToFree++;
    }
    
    if (blockIndex + blocksToFree < memoryManager.blockQty) {
        memoryManager.bitmap[blockIndex + blocksToFree] = FREE;
        memoryManager.blocksUsed -= blocksToFree + 1;
    }
    
    return;
}

#endif