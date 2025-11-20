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

/*
 * sizeToBlockQty
 * @param size: requested size in bytes
 * @return: number of BLOCK_SIZE blocks required to satisfy `size`
 */
static uint32_t sizeToBlockQty(uint32_t size)
{
	return (uint32_t)((size % BLOCK_SIZE) ? (size / BLOCK_SIZE) + 1 : size / BLOCK_SIZE);
}

/*
 * initializeBitmap
 * Marks all entries in the bitmap as FREE.
 */
static void initializeBitmap()
{
	/* mark each block as FREE */
	for (uint32_t i = 0; i < memoryManager.blockQty; i++) {
		memoryManager.bitmap[i] = FREE;
	}
}

/*
 * findFreeBlocks
 * @param blocksNeeded: number of contiguous free blocks required
 * @param start: search start index (inclusive)
 * @param end: search end index (exclusive)
 * @return: address of the first byte of the found block group, or 0 if none
 */
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

/*
 * markGroupAsUsed
 * Marks a contiguous region of the bitmap as allocated using boundary
 * markers (START_BOUNDARY, END_BOUNDARY) and USED for inner blocks.
 * @param blocksNeeded: number of blocks in the group
 * @param index: starting block index in bitmap
 * @return: pointer to the payload start for the caller
 */
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
	/*
	 * Layout in memory:
	 * - bitmap table stored starting at HEAP_START_ADDRESS
	 * - payload area follows the bitmap table
	 * Compute how many physical blocks the bitmap itself consumes and
	 * reduce the available block count accordingly.
	 */
	memoryManager.bitmap = (uint32_t *)HEAP_START_ADDRESS;
	memoryManager.blockQty = HEAP_SIZE / BLOCK_SIZE;

	uint32_t bitmap_size_in_bytes = memoryManager.blockQty * sizeof(uint32_t);
	uint32_t bitmap_size_in_blocks = (bitmap_size_in_bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;

	memoryManager.start = (void *)((uintptr_t)HEAP_START_ADDRESS + bitmap_size_in_blocks * BLOCK_SIZE);
	/* reduce block count by the number of blocks consumed by the bitmap */
	memoryManager.blockQty -= bitmap_size_in_blocks;

	memoryManager.blocksUsed = 0;
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

	/* handle single-block allocation as a special case */
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

	/* single-block allocation case */
	if (memoryManager.bitmap[blockIndex] == SINGLE_BLOCK) {
		memoryManager.bitmap[blockIndex] = FREE;
		memoryManager.blocksUsed--;
		return;
	}

	/* ensure this is the start boundary of a multi-block allocation */
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