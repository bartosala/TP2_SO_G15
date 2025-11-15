#include "../../Shared/shared_structs.h"
#include <defs.h>
#include <interrupts.h>
#include <lib.h>
#include <memoryManager.h>
#include <pipe.h>
#include <scheduler.h>
#include <stackFrame.h>
#include <syscall.h>
#include <textModule.h>

#define SHELL_PID 1
#define TTY 0

#define QUANTUM 3
#define MAX_PRIORITY 5
#define MIN_PRIORITY 0
#define IDLE_PRIORITY 6

uint64_t calculateQuantum(int8_t priority)
{
	if (priority == IDLE_PRIORITY) {
		return QUANTUM;
	}
	return QUANTUM * (MAX_PRIORITY - priority + 1);
}

static ProcessManagerADT processManager = NULL;
static pid_t currentPid = -1;
static pid_t nextPid = 0;
static uint64_t quantum = 0;

static PCB *createProcessOnPCB(char *name, processFun function, uint64_t argc, char **arg, uint8_t priority,
                               char foreground, int stdin, int stdout);
static void wakeUpWaitingParent(pid_t parentPid, pid_t childPid);
int getCurrentStdin();
int getCurrentStdout();
static int32_t reapCild(PCB *child, int32_t *retValue);

void startScheduler(processFun idle)
{
	createPipeManager();

	ProcessManagerADT list = createProcessManager();
	PCB *idleProcess = createProcessOnPCB("idle", idle, 0, NULL, IDLE_PRIORITY, 0, -1, -1);
	setIdleProcess(list, idleProcess);
	processManager = list;
}

pid_t createProcess(char *name, processFun function, uint64_t argc, char **arg, uint8_t priority, char foreground,
                    int stdin, int stdout)
{
	PCB *process = createProcessOnPCB(name, function, argc, arg, priority, foreground, stdin, stdout);
	if (process == NULL) {
		return -1;
	}
	return process->pid;
}

static PCB *createProcessOnPCB(char *name, processFun function, uint64_t argc, char **arg, uint8_t priority,
                               char foreground, int stdin, int stdout)
{
	if (name == NULL || function == NULL || (argc > 0 && arg == NULL)) {
		return NULL;
	}

	PCB *process = allocMemory(sizeof(PCB));
	if (process == NULL) {
		return NULL;
	}

	if (priority > IDLE_PRIORITY) {
		priority = MAX_PRIORITY;
	}

	strncpy(process->name, name, NAME_MAX_LENGTH);
	process->pid = nextPid++;
	process->waitingForPid = -1;
	process->retValue = 0;
	process->foreground = foreground ? 1 : 0;
	process->stdin = -1;
	process->stdout = -1;
	process->state = READY;
	process->priority = priority;
	process->parentPid = getCurrentPid();

	process->entryPoint = (uint64_t)function;

	process->rsp = setUpStackFrame(&process->base, (uint64_t)function, argc, arg);
	if (process->rsp == 0) {
		freeMemory(process);
		return NULL;
	}

	if (process->pid > 1) {
		if (!foreground && stdin == TTY) {
			process->stdout = stdout;
			process->state = BLOCKED;
			addToBlocked(processManager, process);
			return process;
		}
		process->stdin = (stdin == TTY) ? getCurrentStdin() : stdin;
		process->stdout = (stdout == 1) ? getCurrentStdout() : stdout;

	} else if (process->pid == SHELL_PID) {
		int pipe_fd = pipeCreate(0);
		if (pipe_fd < 0) {
			freeMemory((void *)process->base - STACK_SIZE);
			freeMemory(process);
			return NULL;
		}
		process->stdin = pipeOpenForPid(0, PIPE_READ, process->pid);
		if (process->stdin < 0) {
			freeMemory((void *)process->base - STACK_SIZE);
			freeMemory(process);
			return NULL;
		}
		process->stdout = 1;
	}

	if (priority != IDLE_PRIORITY)
		addProcess(processManager, process);
	return process;
}

pid_t getForegroundPid()
{
	PCB *current = getForegroundProcess(processManager);
	return current ? current->pid : -1;
}

pid_t getCurrentPid()
{
	PCB *current = getCurrentProcess(processManager);
	return current ? current->pid : -1;
}

uint64_t schedule(uint64_t rsp)
{
	static int first = 1;
	PCB *currentProcess = getCurrentProcess(processManager);

	if (quantum > 0 && currentProcess->state == RUNNING) {
		quantum--;
		return rsp;
	}

	if (!first)
		currentProcess->rsp = rsp;
	else
		first = 0;
	if (currentProcess->state == RUNNING)
		currentProcess->state = READY;

	PCB *nextProcess = getNextReadyProcess(processManager);

	nextProcess->state = RUNNING;
	currentPid = nextProcess->pid;
	quantum = calculateQuantum(nextProcess->priority);
	return nextProcess->rsp;
}

uint64_t blockProcess(pid_t pid)
{
	if (blockProcessQueue(processManager, pid) != 0) {
		return -1;
	}
	if (pid == getCurrentPid()) {
		yield();
	}
	return 0;
}

uint64_t blockProcessBySem(pid_t pid)
{
	if (blockProcessQueueBySem(processManager, pid) != 0) {
		return -1;
	}
	if (pid == getCurrentPid()) {
		yield();
	}
	return 0;
}

void yield()
{
	quantum = 0;
	callTimerTick();
}

uint64_t unblockProcess(pid_t pid)
{
	return unblockProcessQueue(processManager, pid);
}

uint64_t unblockProcessBySem(pid_t pid)
{
	return unblockProcessQueueBySem(processManager, pid);
}

uint64_t kill(pid_t pid, uint64_t retValue)
{
	if (pid <= 1) {
		return -1;
	}
	PCB *process = killProcess(processManager, pid, retValue, ZOMBIE);
	if (process == NULL) {
		return -1;
	}

	freeMemory((void *)process->base - STACK_SIZE);
	wakeUpWaitingParent(process->parentPid, pid);

	if (pid == currentPid) {
		quantum = 0;
		callTimerTick();
	}
	return 0;
}

static int32_t reapCild(PCB *child, int32_t *retValue)
{
	if (retValue != NULL) {
		*retValue = child->retValue;
	}

	int32_t childPid = child->pid;

	child->state = child->retValue;
	removeFromZombie(processManager, childPid);
	freeMemory(child);

	return childPid;
}

static void wakeUpWaitingParent(pid_t parentPid, pid_t childPid)
{
	if (parentPid == -1) {
		return;
	}
	if (parentPid == getIdleProcess(processManager)->pid) {
		reapCild(getProcess(processManager, childPid), NULL);
	}

	PCB *parent = getProcess(processManager, parentPid);
	if (parent == NULL) {
		return;
	}

	if (parent->state == BLOCKED && parent->waitingForPid == childPid) {
		unblockProcess(parentPid);
	}
}
int32_t waitpid(pid_t pid, int32_t *retValue)
{
	pid_t currentProcPid = getCurrentPid();

	PCB *target = getProcess(processManager, pid);
	if (target == NULL) {
		return -1;
	}
	PCB *current = getProcess(processManager, currentProcPid);
	if (current == NULL) {
		return -1;
	}

	if (target->parentPid != currentProcPid && current->waitingForPid != pid) {
		return -1;
	}

	if (target->state == ZOMBIE) {
		current->waitingForPid = -1;
		return reapCild(target, retValue);
	}

	if (target->state < ZOMBIE) {
		current->waitingForPid = pid;
		current->state = BLOCKED;
		if (blockProcess(currentProcPid) != 0) {
			current->waitingForPid = -1;
			current->state = READY;
			return -1;
		}
		target = getProcess(processManager, pid);
		if (target == NULL) {
			return -1;
		}

		if (target->state == ZOMBIE) {
			current->waitingForPid = -1;
			return reapCild(target, retValue);
		}
	}

	return -1;
}

int8_t changePrio(pid_t pid, int8_t newPrio)
{
	PCB *process = getProcess(processManager, pid);
	if (pid <= 1) {
		return -1;
	}
	if (process == NULL) {
		return -1;
	}
	if (newPrio < MIN_PRIORITY) {
		newPrio = MIN_PRIORITY;
	} else if (newPrio > MAX_PRIORITY) {
		newPrio = MAX_PRIORITY;
	}
	process->priority = newPrio;
	return newPrio;
}

PCB *getProcessInfo(uint64_t *cantProcesses)
{
	if (processManager == NULL) {
		*cantProcesses = 0;
		return NULL;
	}

	uint64_t count = processCount(processManager);

	PCB *processInfo = allocMemory(sizeof(PCB) * count);
	if (processInfo == NULL) {
		*cantProcesses = 0;
		return NULL;
	}

	uint64_t j = 0;
	for (uint64_t i = 0; j < count; i++) {
		PCB *process = getProcess(processManager, i);
		if (process != NULL) {
			if (copyProcess(&processInfo[j++], process) == -1) {
				freeMemory(processInfo);
				*cantProcesses = 0;
				return NULL;
			}
		}
	}

	*cantProcesses = j;
	return processInfo;
}

int16_t copyProcess(PCB *dest, PCB *src)
{
	dest->pid = src->pid;
	dest->parentPid = src->parentPid;
	dest->waitingForPid = src->waitingForPid;
	dest->priority = src->priority;
	dest->state = src->state;
	dest->rsp = src->rsp;
	dest->base = src->base;
	dest->entryPoint = src->entryPoint;
	strncpy(dest->name, src->name, NAME_MAX_LENGTH);
	dest->name[NAME_MAX_LENGTH - 1] = '\0';
	dest->retValue = src->retValue;
	dest->foreground = src->foreground;
	dest->stdin = src->stdin;
	dest->stdout = src->stdout;
	return 0;
}

// Add new functions to access current process information
int getCurrentStdin()
{
	PCB *current = getCurrentProcess(processManager);
	return current ? current->stdin : -1;
}

int getCurrentStdout()
{
	PCB *current = getCurrentProcess(processManager);
	return current ? current->stdout : -1;
}
