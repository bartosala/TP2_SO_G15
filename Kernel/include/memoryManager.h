#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdlib.h>
#include <stdint.h>
#include "../../Shared/shared_structs.h"

#define BLOCK_SIZE 64 // Tamaño de bloque en bytes

typedef struct MemoryManagerCDT *MemoryManagerADT;

void createMemoryManager(void *start, uint64_t size);

void *allocMemory(uint64_t size);
void freeMemory(void *address);
int getUsedMemory(void);
int getFreeMemory(void);

void getMemoryInfo(memInfo *info);



#endif