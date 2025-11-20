#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "../../Shared/shared_structs.h"
#include <stdint.h>
#include <stdlib.h>

#define BLOCK_SIZE 64 /* block size in bytes used by memory manager */

typedef struct MemoryManagerCDT *MemoryManagerADT;

/*
 * createMemoryManager
 * @param start: pointer to the start of the memory region managed
 * @param size:  size in bytes of the region
 * @return: none. Initializes internal memory manager data structures.
 */
void createMemoryManager(void *start, uint64_t size);

/*
 * allocMemory
 * @param size: number of bytes to allocate
 * @return: pointer to allocated memory, or NULL on failure
 */
void *allocMemory(uint64_t size);

/*
 * freeMemory
 * @param address: pointer previously returned by allocMemory to free
 * @return: none
 */
void freeMemory(void *address);

/*
 * getUsedMemory
 * @return: number of bytes currently allocated/used
 */
int getUsedMemory(void);

/*
 * getFreeMemory
 * @return: number of free bytes available to allocate
 */
int getFreeMemory(void);

/*
 * getMemoryInfo
 * @param info: pointer to a `memInfo` struct that will be filled with
 *              current memory manager statistics. Caller must provide
 *              a valid pointer.
 */
void getMemoryInfo(memInfo *info);

#endif