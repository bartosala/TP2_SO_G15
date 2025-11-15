#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#include <stdint.h>
#include <queue.h>
#include <interrupts.h>
#include "../../Shared/shared_structs.h"

#define NUM_SEMS 300

typedef struct sem {
	uint32_t value;
	uint8_t lock;
    uint8_t used;
	QueueADT blocked;
} sem_t;


typedef struct SemaphoreCDT *SemaphoreADT;

SemaphoreADT createSemaphoresManager();
int semInit (int id, uint32_t value);
int semOpen (int id);
int semClose(int id);
int semWait(int id);
int semPost(int id);
int wait(sem_t *sem);
int post(sem_t *sem);

#endif