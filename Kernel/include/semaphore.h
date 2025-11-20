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
	uint8_t used;    /* non-zero if this slot is in use */
	QueueADT blocked; /* queue of blocked processes waiting on this sem */
} sem_t;


typedef struct SemaphoreCDT *SemaphoreADT;

/*
 * createSemaphoresManager
 * @return: newly allocated SemaphoreADT manager
 */
SemaphoreADT createSemaphoresManager();

/*
 * semInit
 * Initializes semaphore at `id` with initial `value`.
 * @return: 0 on success, -1 on error
 */
int semInit (int id, uint32_t value);

/*
 * semOpen
 * Marks semaphore `id` as used/available.
 * @return: 0 on success, -1 on failure
 */
int semOpen (int id);

/*
 * semClose
 * Closes semaphore `id` and releases related resources.
 * @return: 0 on success, -1 on failure
 */
int semClose(int id);

/*
 * semWait
 * Decrements semaphore `id` (may block the calling process).
 * @return: 0 on success, -1 on error
 */
int semWait(int id);

/*
 * semPost
 * Increments semaphore `id` and wakes a blocked process if any.
 * @return: 0 on success, -1 on error
 */
int semPost(int id);

/*
 * wait
 * Internal: wait operation on a `sem_t` object pointer.
 * @return: 0 on success, -1 on error
 */
int wait(sem_t *sem);

/*
 * post
 * Internal: post operation on a `sem_t` object pointer.
 * @return: 0 on success, -1 on error
 */
int post(sem_t *sem);

#endif