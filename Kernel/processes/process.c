#include "../include/process.h"
#include "../../Shared/shared_structs.h"
#include "../include/defs.h"
#include "../include/interrupts.h"
#include "../include/memoryManager.h"
#include "../include/scheduler.h"
#include <lib.h>
#include <queue.h>
#include <stddef.h>
#include <string.h>

typedef struct ProcessManagerCDT {
	PCB *foregroundProcess;
	PCB *currentProcess;
	QueueADT readyQueue;
	QueueADT blockedQueue;
	QueueADT blockedQueueBySem;
	QueueADT zombieQueue;
	PCB *idleProcess;
	uint8_t lock; // Global lock for process manager operations
} ProcessManagerCDT;

extern void acquire(uint8_t *lock);
extern void release(uint8_t *lock);

ProcessManagerADT createProcessManager()
{
	ProcessManagerADT processManager = allocMemory(sizeof(ProcessManagerCDT));
	if (processManager == NULL) {
		return NULL;
	}
	processManager->foregroundProcess = NULL;
	processManager->currentProcess = NULL;
	processManager->lock = 0;
	processManager->readyQueue = createQueue();
	if (processManager->readyQueue == NULL) {
		freeMemory(processManager);
		return NULL;
	}
	processManager->blockedQueue = createQueue();
	if (processManager->blockedQueue == NULL) {
		freeQueue(processManager->readyQueue);
		freeMemory(processManager);
		return NULL;
	}
	processManager->blockedQueueBySem = createQueue();
	if (processManager->blockedQueueBySem == NULL) {
		freeQueue(processManager->readyQueue);
		freeQueue(processManager->blockedQueue);
		freeMemory(processManager);
		return NULL;
	}
	processManager->zombieQueue = createQueue();
	if (processManager->zombieQueue == NULL) {
		freeQueue(processManager->readyQueue);
		freeQueue(processManager->blockedQueue);
		freeQueue(processManager->zombieQueue);
		freeMemory(processManager);
		return NULL;
	}
	return processManager;
}

int compareByPid(void *a, void *b)
{
	PCB *pcbA = (PCB *)a;
	PCB *pcbB = (PCB *)b;
	return (pcbA->pid == pcbB->pid) ? 0 : -1;
}

int hasPid(void *a, void *b)
{
	if (a == NULL || b == NULL) {
		return -1;
	}
	PCB *processA = (PCB *)a;
	pid_t *pid = (pid_t *)b;

	return (processA->pid == *pid) ? 0 : -1;
}

void addProcess(ProcessManagerADT pm, PCB *process)
{
	if (pm == NULL || process == NULL) {
		return;
	}

	if (pm->currentProcess == NULL || pm->currentProcess == pm->idleProcess) {
		pm->currentProcess = process;
	}
	enqueue(pm->readyQueue, process);
}

void removeFromReady(ProcessManagerADT pm, pid_t pid)
{
	if (pm == NULL) {
		return;
	}

	remove(pm->readyQueue, &pid, hasPid);
	if (pm->foregroundProcess && pm->foregroundProcess->pid == pid) {
		pm->foregroundProcess = NULL;
	}
}

void removeFromZombie(ProcessManagerADT pm, pid_t pid)
{
	if (pm == NULL) {
		return;
	}

	remove(pm->zombieQueue, &pid, hasPid);
}

static PCB *switchProcessFromQueues(QueueADT queueFrom, QueueADT queueTo, pid_t pid)
{
	if (queueFrom == NULL || queueTo == NULL) {
		return NULL;
	}
	PCB *process = (PCB *)remove(queueFrom, &pid, hasPid);
	if (process == NULL) {
		return NULL;
	}
	enqueue(queueTo, process);
	return process;
}

int blockProcessQueue(ProcessManagerADT pm, pid_t pid)
{
	if (pm == NULL) {
		return -1;
	}
	
	// Don't allow blocking processes that are already blocked by semaphores
	// This would break the semaphore's ability to unblock them
	PCB *semBlockedProcess = (PCB *)containsQueue(pm->blockedQueueBySem, &pid, hasPid);
	if (semBlockedProcess != NULL) {
		// Process is blocked by semaphore - cannot manually block it
		return -1;
	}
	
	QueueADT queue = pm->readyQueue;
	PCB *process = (PCB *)containsQueue(pm->readyQueue, &pid, hasPid);
	if (process == NULL) {
		// Check if already in blocked queue
		process = (PCB *)containsQueue(pm->blockedQueue, &pid, hasPid);
		if (process != NULL) {
			// Already blocked
			return -1;
		}
		return -1;
	}

	process = switchProcessFromQueues(queue, pm->blockedQueue, pid);
	if (process == NULL) {
		return -1;
	}

	/* ensure state is BLOCKED */
	if (process->state != BLOCKED) {
		process->state = BLOCKED;
	}
	
	return 0;
}

int blockProcessQueueBySem(ProcessManagerADT pm, pid_t pid)
{
	if (pm == NULL) {
		return -1;
	}

	PCB *process = switchProcessFromQueues(pm->readyQueue, pm->blockedQueueBySem, pid);
	if (process == NULL) {
		return -1;
	}

	if (process->state != BLOCKED) {
		process->state = BLOCKED;
	}
	
	return 0;
}

int unblockProcessQueue(ProcessManagerADT pm, pid_t pid)
{
	if (pm == NULL) {
		return -1;
	}

	PCB *process = switchProcessFromQueues(pm->blockedQueue, pm->readyQueue, pid);
	if (process == NULL) {
		return -1;
	}

	if (process->state != READY) {
		process->state = READY;
	}
	
	return 0;
}

int unblockProcessQueueBySem(ProcessManagerADT pm, pid_t pid)
{
	if (pm == NULL) {
		return -1;
	}

	PCB *process = switchProcessFromQueues(pm->blockedQueueBySem, pm->readyQueue, pid);
	if (process == NULL) {
		return -1;
	}

	if (process->state != READY) {
		process->state = READY;
	}
	
	return 0;
}

void freeProcessLinkedLists(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return;
	}
	freeQueue(pm->readyQueue);
	freeQueue(pm->blockedQueue);
	freeQueue(pm->blockedQueueBySem);
	freeQueue(pm->zombieQueue);
	freeMemory(pm);
}

PCB *getProcess(ProcessManagerADT pm, pid_t pid)
{
	if (pm == NULL) {
		return NULL;
	}

	PCB *process = (PCB *)containsQueue(pm->readyQueue, &pid, hasPid);
	if (process != NULL) {
		return process;
	}

	process = (PCB *)containsQueue(pm->blockedQueue, &pid, hasPid);
	if (process != NULL) {
		return process;
	}

	process = (PCB *)containsQueue(pm->blockedQueueBySem, &pid, hasPid);
	if (process != NULL) {
		return process;
	}

	process = (PCB *)containsQueue(pm->zombieQueue, &pid, hasPid);
	if (process != NULL) {
		return process;
	}

	if (pm->currentProcess != NULL && pm->currentProcess->pid == pid) {
		return pm->currentProcess;
	}

	if (pm->idleProcess != NULL && pm->idleProcess->pid == pid) {
		return pm->idleProcess;
	}

	return NULL;
}

PCB *getNextReadyProcess(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return NULL;
	}

	if (isQueueEmpty(pm->readyQueue)) {
		pm->currentProcess = pm->idleProcess;
		return pm->idleProcess;
	}

	PCB *nextProcess = (PCB *)dequeue(pm->readyQueue);
	if (nextProcess == NULL) {
		return NULL;
	}

	if (enqueue(pm->readyQueue, nextProcess) != 0) {
		return NULL;
	}

	pm->currentProcess = nextProcess;
	if (isForegroundProcess(nextProcess)) {
		foregroundProcessSet(pm, nextProcess);
	}
	return nextProcess;
}

PCB *getNextProcess(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return NULL;
	}

	if (isQueueEmpty(pm->readyQueue)) {
		pm->currentProcess = pm->idleProcess;
		return pm->idleProcess;
	}

	PCB *nextProcess = (PCB *)dequeue(pm->readyQueue);
	if (nextProcess == NULL) {
		return NULL;
	}

	if (enqueue(pm->readyQueue, nextProcess) != 0) {
		return NULL;
	}

	pm->currentProcess = nextProcess;
	if (isForegroundProcess(nextProcess)) {
		foregroundProcessSet(pm, nextProcess);
	}
	return nextProcess;
}

int hasNextReadyProcess(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return 0;
	}

	return !isQueueEmpty(pm->readyQueue);
}

PCB *getCurrentProcess(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return NULL;
	}

	return pm->currentProcess;
}

void setIdleProcess(ProcessManagerADT pm, PCB *idleProcess)
{
	if (pm != NULL) {
		pm->idleProcess = idleProcess;
		pm->currentProcess = idleProcess;
	}
}

void foregroundProcessSet(ProcessManagerADT pm, PCB *process)
{
	if (pm == NULL) {
		return;
	}

	pm->foregroundProcess = process;
}

PCB *getIdleProcess(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return NULL;
	}

	return pm->idleProcess;
}

uint64_t processCount(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return 0;
	}

	uint64_t count = 0;
	count += queueSize(pm->readyQueue);
	count += queueSize(pm->blockedQueue);
	count += queueSize(pm->blockedQueueBySem);
	count += queueSize(pm->zombieQueue);
	count++;

	return count;
}

uint64_t readyProcessCount(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return 0;
	}

	return queueSize(pm->readyQueue);
}

uint64_t blockedProcessCount(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return 0;
	}

	return queueSize(pm->blockedQueue) + queueSize(pm->blockedQueueBySem);
}

uint64_t zombieProcessCount(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return 0;
	}

	return queueSize(pm->zombieQueue);
}

PCB *getForegroundProcess(ProcessManagerADT pm)
{
	if (pm == NULL) {
		return NULL;
	}

	return pm->foregroundProcess;
}

bool isCurrentProcessForeground(ProcessManagerADT pm, pid_t pid)
{
	return pm && pm->foregroundProcess && pm->foregroundProcess->pid == pid;
}

bool isIdleProcess(ProcessManagerADT pm, pid_t pid)
{
	return pm && pm->idleProcess && pm->idleProcess->pid == pid;
}

bool isForegroundProcess(PCB *process)
{
	return process && process->foreground == 1;
}

PCB *killProcess(ProcessManagerADT pm, pid_t pid, uint64_t ret, State state)
{
	if (pm == NULL) {
		return NULL;
	}

	if (pm->currentProcess && pm->currentProcess->pid == pid) {
		pm->currentProcess = pm->idleProcess;
	}
	PCB *process = switchProcessFromQueues(pm->readyQueue, pm->zombieQueue, pid);
	if (process == NULL) {
		process = switchProcessFromQueues(pm->blockedQueue, pm->zombieQueue, pid);
		if (process == NULL) {
			process = switchProcessFromQueues(pm->blockedQueueBySem, pm->zombieQueue, pid);
			if (process == NULL) {
				return NULL;
			}
		}
	}

	if (pm->foregroundProcess && pm->foregroundProcess->pid == pid) {
		pm->foregroundProcess = NULL;
	}

	process->retValue = ret;
	process->state = state;
	return process;
}

int stdintProcess(ProcessManagerADT pm, pid_t pid)
{
	PCB *process = getProcess(pm, pid);
	if (process == NULL) {
		return -1;
	}
	return process->stdin;
}

int stdoutProcess(ProcessManagerADT pm, pid_t pid)
{
	PCB *process = getProcess(pm, pid);
	if (process == NULL) {
		return -1;
	}
	return process->stdout;
}

void addToReady(ProcessManagerADT pm, PCB *process)
{
	if (pm == NULL || process == NULL) {
		return;
	}
	enqueue(pm->readyQueue, process);
}

void addToBlocked(ProcessManagerADT pm, PCB *process)
{
	if (pm == NULL || process == NULL) {
		return;
	}
	enqueue(pm->blockedQueue, process);
}

void addToBlockedBySem(ProcessManagerADT pm, PCB *process)
{
	if (pm == NULL || process == NULL) {
		return;
	}
	enqueue(pm->blockedQueueBySem, process);
}