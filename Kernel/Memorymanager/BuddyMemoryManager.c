#ifdef BUDDY
#include <memoryManager.h>
#include <defs.h>
#include <stdint.h>
#include <stddef.h>

// --- Based on the provided reference implementation ---

#define MIN_ORDER 6 // Minimum block size will be 2^6 = 64 bytes, matching BLOCK_SIZE
#define MAX_ORDER 30 // A sufficiently large max order

enum { FREE, OCCUPIED };

typedef struct block_t {
  int8_t order;
  int8_t status;
  struct block_t *next;
} block_t;

typedef struct buddy_manager {
  int8_t max_order;
  block_t *free_blocks[MAX_ORDER];
  uint64_t total_mem;
  uint64_t used_mem;
} buddy_manager;

static buddy_manager buddy_man;

// --- Forward declarations for helper functions ---
static block_t *create_block(void *address, int8_t order);
static void remove_block(block_t *block);
static void split_block(int8_t order);


// --- Core Implementation ---

static block_t *create_block(void *address, int8_t order) {
  block_t *block = (block_t *)address;
  block->order = order;
  block->status = FREE;
  block->next = buddy_man.free_blocks[order];
  buddy_man.free_blocks[order] = block;
  return block;
}

static void remove_block(block_t *block) {
  if (block == NULL) {
    return;
  }
  uint8_t order = block->order;
  block_t *curr_block = buddy_man.free_blocks[order];
  if (curr_block == NULL) {
    return;
  }

  if (curr_block == block) {
    buddy_man.free_blocks[order]->status = OCCUPIED;
    buddy_man.free_blocks[order] = buddy_man.free_blocks[order]->next;
    return;
  }

  while (curr_block->next != NULL && curr_block->next != block) {
    curr_block = curr_block->next;
  }

  if (curr_block->next == NULL) {
    return;
  }

  curr_block->next = curr_block->next->next;
  block->status = OCCUPIED; // Mark the block being removed as occupied
}

static void split_block(int8_t order) {
  block_t *block = buddy_man.free_blocks[order];
  remove_block(block);
  block_t *buddy_block = (block_t *)((void *)block + (1L << (order - 1)));
  create_block((void *)buddy_block, order - 1);
  create_block((void *)block, order - 1);
}

void createMemoryManager(void *start, uint64_t size) {
    uint64_t total_size = size;
    int8_t order = 0;
    uint64_t current_size = 1;
    while(current_size < total_size) {
        current_size *= 2;
        order++;
    }
    
    buddy_man.max_order = order;
    buddy_man.total_mem = current_size;
    buddy_man.used_mem = 0;

    for (int i = 0; i < MAX_ORDER; i++) {
        buddy_man.free_blocks[i] = NULL;
    }
    create_block(start, order);
}

void *allocMemory(const size_t memoryToAllocate) {
    if (memoryToAllocate <= 0 || memoryToAllocate > HEAP_SIZE) {
        return NULL;
    }

    int8_t order = 1;
    int64_t block_size = 2;
    while (block_size < memoryToAllocate + sizeof(block_t)) {
        order++;
        block_size *= 2;
    }

    order = (MIN_ORDER > order) ? MIN_ORDER : order;

    if (buddy_man.free_blocks[order] == NULL) {
        uint8_t order_approx = 0;
        for (uint8_t i = order + 1; i <= buddy_man.max_order && order_approx == 0; i++) {
            if (buddy_man.free_blocks[i] != NULL) {
                order_approx = i;
            }
        }

        if (order_approx == 0) {
            return NULL; // No suitable block found
        }

        while (order_approx > order) {
            split_block(order_approx);
            order_approx--;
        }
    }

    block_t *block = buddy_man.free_blocks[order];
    remove_block(block);
    block->status = OCCUPIED;
    buddy_man.used_mem += (1L << order);

    return (void *)((uint8_t *)block + sizeof(block_t));
}

void freeMemory(void *address) {
    if (address == NULL) {
        return;
    }

    block_t *block = (block_t *)((uint8_t*)address - sizeof(block_t));
    if (block->status == FREE) {
        return;
    }
    
    buddy_man.used_mem -= (1L << block->order);
    block->status = FREE;

    uint64_t block_pos = (uint64_t)((void *)block - HEAP_START_ADDRESS);
    
    while (block->order < buddy_man.max_order) {
        uint64_t buddy_pos = block_pos ^ (1L << block->order);
        block_t *buddy_block = (block_t *)((uint64_t)HEAP_START_ADDRESS + buddy_pos);

        if (buddy_block->status != FREE || buddy_block->order != block->order) {
            break; // Buddy is not free or not of the same order, stop merging
        }

        block_t* left_block = (block_pos < buddy_pos) ? block : buddy_block;
        
        remove_block(block);
        remove_block(buddy_block);

        left_block->order++;
        block = left_block; // Continue loop with the new, larger block
        block_pos = (uint64_t)((void *)block - HEAP_START_ADDRESS);
    }

    create_block((void *)block, block->order);
}

void getMemoryInfo(memInfo *info) {
    if (info == NULL) {
        return;
    }
    info->total = buddy_man.total_mem;
    info->used = buddy_man.used_mem;
    info->free = info->total - info->used;
}

#endif