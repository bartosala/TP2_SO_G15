#include "../../Shared/shared_structs.h"
#include <memoryManager.h>
#include <queue.h>
#include <scheduler.h>
#include <semaphore.h>
#include <stddef.h>

#define USED 1
#define NOT_USED 0

SemaphoreADT semaphore = NULL;

static int8_t isValidSemaphore(uint8_t sem);

static int8_t isValidSemaphore(uint8_t sem)
{
	if (semaphore == NULL || sem >= NUM_SEMS) {
		return 0;
	}
	return 1;
}

SemaphoreADT createSemaphoresManager()
{
	SemaphoreADT semManager = (SemaphoreADT)allocMemory(sizeof(SemaphoreCDT));
	if (semManager != NULL) {
		for (int i = 0; i < NUM_SEMS; i++) {
			semManager->semaphores[i].value = 0;
			semManager->semaphores[i].lock = 0;
			semManager->semaphores[i].used = NOT_USED;
			semManager->semaphores[i].waitingProcesses = createDoubleLinkedList();
		}
	}
	return semManager;
}

int8_t semOpen(uint8_t sem)
{
	if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == USED) {
		return -1;
	}
	return 0;
}

int8_t semClose(uint8_t sem)
{
	if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == NOT_USED) {
		return -1;
	}
	semaphore->semaphores[sem].used = NOT_USED;
	semaphore->semaphores[sem].value = 0;

	freeList(semaphore->semaphores[sem].waitingProcesses);
	semaphore->semaphores[sem].waitingProcesses = createDoubleLinkedList();
	return 0;
}

int8_t semWait(uint8_t sem)
{
	if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == NOT_USED) {
		return -1;
	}

	acquire(&semaphore->semaphores[sem].lock);
	if (semaphore->semaphores[sem].value > 0) {
		semaphore->semaphores[sem].value--;
		release(&semaphore->semaphores[sem].lock);
		return 0;
	}
	int *pid = (int *)allocMemory(sizeof(int));
	if (pid == NULL) {
		release(&semaphore->semaphores[sem].lock);
		return -1;
	}
	*pid = (int)getCurrentPid();
	insertLast(semaphore->semaphores[sem].waitingProcesses, pid);
	release(&semaphore->semaphores[sem].lock);
	blockProcessBySem(*pid);
	yield();
	return 0;
}

int8_t semPost(uint8_t sem)
{
	if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == NOT_USED) {
		return -1;
	}
	acquire(&semaphore->semaphores[sem].lock);

	while (!isEmpty(semaphore->semaphores[sem].waitingProcesses)) {
		int *pid_ptr = (int *)getFirst(semaphore->semaphores[sem].waitingProcesses);
		if (pid_ptr == NULL)
			break;
		removeElement(semaphore->semaphores[sem].waitingProcesses, pid_ptr);
		pid_t pid = (pid_t)(*pid_ptr);
		unblockProcessBySem(pid);
		freeMemory(pid_ptr);
		break;
	}

	if (isEmpty(semaphore->semaphores[sem].waitingProcesses)) {
		semaphore->semaphores[sem].value++;
	}

	release(&semaphore->semaphores[sem].lock);
	return 0;
}

int8_t semInit(uint8_t sem, uint8_t value)
{
	if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == USED) {
		return -1;
	}
	semaphore->semaphores[sem].value = value;
	semaphore->semaphores[sem].used = USED;
	return 0;
}
