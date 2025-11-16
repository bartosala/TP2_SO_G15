#ifndef BUDDY
#include "../../Shared/shared_structs.h"
#include "memoryManager.h"
#include <defs.h>
#include <stdint.h>

enum { FREE, USED, START_BOUNDARY, END_BOUNDARY, SINGLE_BLOCK };

typedef struct MemoryManagerCDT {
	void *start;
	uint32_t blockQty;
	uint32_t blocksUsed;
	uint32_t *bitmap;
	uint32_t current;
} MemoryManagerCDT;

static MemoryManagerCDT memoryManager;

static uint32_t sizeToBlockQty(uint32_t size);
static void *markGroupAsUsed(uint32_t blocksNeeded, uint32_t index);
static uintptr_t findFreeBlocks(uint32_t blocksNeeded, uint32_t start, uint32_t end);
static void initializeBitmap();

static uint32_t sizeToBlockQty(uint32_t size)
{
	return (uint32_t)((size % BLOCK_SIZE) ? (size / BLOCK_SIZE) + 1 : size / BLOCK_SIZE);
}

static void initializeBitmap()
{
	for (uint32_t i = 0; i < memoryManager.blockQty; i++) {
		memoryManager.bitmap[i] = FREE;
	}
}

static uintptr_t findFreeBlocks(uint32_t blocksNeeded, uint32_t start, uint32_t end)
{
	uint32_t freeBlocks = 0;
	uint32_t i;
	for (i = start; i < end; i++) {
		if (memoryManager.bitmap[i] == FREE) {
			freeBlocks++;
			if (freeBlocks == blocksNeeded) {
				return (uintptr_t)(memoryManager.start + (i - blocksNeeded + 1) * BLOCK_SIZE);
			}
		} else {
			freeBlocks = 0;
		}
	}
	if (freeBlocks == blocksNeeded) {
		return (uintptr_t)(memoryManager.start + (i - blocksNeeded + 1) * BLOCK_SIZE);
	}
	return 0;
}

static void *markGroupAsUsed(uint32_t blocksNeeded, uint32_t index)
{
	memoryManager.bitmap[index] = START_BOUNDARY;
	memoryManager.bitmap[index + blocksNeeded - 1] = END_BOUNDARY;
	memoryManager.blocksUsed += 2;
	for (uint32_t i = 1; i < blocksNeeded - 1; i++) {
		memoryManager.blocksUsed++;
		memoryManager.bitmap[index + i] = USED;
	}
	return (void *)(memoryManager.start + index * BLOCK_SIZE);
}

void createMemoryManager(void *start, uint64_t size)
{
	uint32_t totalNeeded = size;
	memoryManager.start = start;
	memoryManager.blockQty = totalNeeded / BLOCK_SIZE;
	uint32_t bitMapSize = memoryManager.blockQty / BLOCK_SIZE;
	totalNeeded += bitMapSize * BLOCK_SIZE;
	memoryManager.bitmap = start;
	memoryManager.blocksUsed = 0;
	memoryManager.start = (uint8_t *)memoryManager.bitmap + bitMapSize * BLOCK_SIZE;
	memoryManager.current = 0;
	initializeBitmap();
}

void *allocMemory(uint64_t size)
{
	uint32_t blocksNeeded = sizeToBlockQty(size);

	if (blocksNeeded > memoryManager.blockQty - memoryManager.blocksUsed) {
		return NULL;
	}

	uintptr_t initialBlockAddress = findFreeBlocks(blocksNeeded, memoryManager.current, memoryManager.blockQty);

	if (initialBlockAddress == 0) {
		initialBlockAddress = findFreeBlocks(blocksNeeded, 0, memoryManager.blockQty);
	}

	if (initialBlockAddress == 0) {
		return NULL;
	}

	uint32_t blockIndex = (initialBlockAddress - (uintptr_t)memoryManager.start) / BLOCK_SIZE;
	memoryManager.current = blockIndex + blocksNeeded;

	if (blocksNeeded == 1) {
		memoryManager.blocksUsed++;
		memoryManager.bitmap[blockIndex] = SINGLE_BLOCK;
		return (void *)initialBlockAddress;
	}

	return markGroupAsUsed(blocksNeeded, blockIndex);
}

void getMemoryInfo(memInfo *info)
{
	if (info == NULL) {
		return;
	}

	info->total = memoryManager.blockQty * BLOCK_SIZE;
	info->used = memoryManager.blocksUsed * BLOCK_SIZE;
	info->free = info->total - info->used;
}

void freeMemory(void *address)
{
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
	while (memoryManager.bitmap[blockIndex + blocksToFree] != END_BOUNDARY) {
		memoryManager.bitmap[blockIndex + blocksToFree] = FREE;
		blocksToFree++;
	}
	memoryManager.bitmap[blockIndex + blocksToFree] = FREE;
	memoryManager.blocksUsed -= blocksToFree + 1;
}

int getUsedMemory()
{
	return memoryManager.blocksUsed * BLOCK_SIZE;
}

int getFreeMemory()
{
	return (memoryManager.blockQty - memoryManager.blocksUsed) * BLOCK_SIZE;
}

#endif