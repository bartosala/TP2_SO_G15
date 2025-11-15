#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "../../Shared/shared_structs.h"
#include "double_linked_list.h"
#include <interrupts.h>
#include <stdint.h>

#define NUM_SEMS 100

typedef struct sem_t {
	uint8_t value;
	uint8_t used;
	uint8_t lock;
	DoubleLinkedListADT waitingProcesses;
} sem_t;

typedef struct SemaphoreCDT {
	sem_t semaphores[NUM_SEMS];
} SemaphoreCDT;

typedef struct SemaphoreCDT *SemaphoreADT;

// Function declarations
SemaphoreADT createSemaphoresManager();
int8_t semOpen(uint8_t sem);
int8_t semClose(uint8_t sem);
int8_t semWait(uint8_t sem);
int8_t semPost(uint8_t sem);
int8_t semInit(uint8_t sem, uint8_t value);

#endif // SEMAPHORE_H