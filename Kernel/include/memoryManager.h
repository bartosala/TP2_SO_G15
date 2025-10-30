#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdlib.h>

#define BLOCK_SIZE 64 // Tamaño de bloque en bytes

typedef struct MemoryManagerCDT *MemoryManagerADT;

void createMemoryManager(void *startAddress, size_t memorySize);

void *allocMemory(const size_t memoryToAllocate);

void freeMemory(void *address);



#endif