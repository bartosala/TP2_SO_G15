#include "memoryManager.h"
#include <defs.h>
#include <stdint.h>

#define BLOCK_SIZE 64 // Bloques de 64 bytes

typedef struct MemoryManagerCDT {
    void *start;   // Comienzo de memoria 
    uint32_t blockQty;	// Cantidad de bloques
    uint32_t blocksUsed;	// Bloques usados 
    uint32_t *bitmap;
    uint32_t current; // Posicion actual del bitmap
} MemoryManagerCDT;

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
		memoryManager->bitmap[i] = 0;
	}
	
	return memoryManager;
}

void *allocMemory(MemoryManagerADT const restrict memoryManager, const size_t memoryToAllocate) {
	char *allocation = memoryManager->nextAddress;

	memoryManager->nextAddress += memoryToAllocate;

	return (void *) allocation;
}