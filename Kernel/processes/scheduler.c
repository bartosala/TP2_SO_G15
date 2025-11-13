#include <stdint.h>
#include <memoryManager.h>
#include <process.h>
#include <stddef.h>
#include <memoryManager.h>
#include <doubleLinkedList.h>
#include <semaphore.h>
#include <pipe.h>
#include <scheduler.h>
#include <lib.h>

static void* pScheduler = NULL;

SchedulerADT getSchedulerADT(){
	return pScheduler;
}

SchedulerADT createScheduler(){
	SchedulerADT scheduler = allocMemory(sizeof(SchedulerCDT));
	if(scheduler == NULL){
		return NULL;
	}

	pScheduler = scheduler;
	scheduler->readyList = createDoubleLinkedList();
	scheduler->blockedList = createDoubleLinkedList();

	scheduler->count = 0;
	scheduler->quantum = FIRST_QUANTUM;
	scheduler->currentPID = 0;
	scheduler->started = 0;
	scheduler->idlePID = 0;
	scheduler->cantReady = 0;

	for(int i = 0; i < MAX_PROCESSES; i++){
		scheduler->process[i].state = DEAD;
		scheduler->process[i].rsp = NULL;
		scheduler->process[i].priority = 0;
		scheduler->process[i].stack = 0;
		scheduler->process[i].basePointer = 0;
		scheduler->process[i].foreground = 0;
		scheduler->process[i].argv = NULL;
		scheduler->process[i].argc = 0;
		scheduler->process[i].rip = 0;
		scheduler->process[i].fds[0] = 0;
		scheduler->process[i].fds[1] = 0;
		scheduler->process[i].fds[2] = 0;
		scheduler->process[i].name = NULL;
	}

	return scheduler;
}

int killFgProcess(){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return -2;
	}
	uint8_t index = 0;
	for(int i = 1; index < scheduler->count && i < MAX_PROCESSES; i++){
		if(scheduler->process[i].state != DEAD && scheduler->process[i].fds[STDIN] == STDIN){
			killProcess(i);
			index = 1;
		}
	}
	if(index == 0){
		return -1;
	}
	return 0;
}

int killProcess(uint16_t pid){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return -2;
	}
	if(scheduler->process[pid].state == DEAD){
		return -1;
	}
	closePipeByPID(pid);
	if(scheduler->process[pid].state == RUNNING || scheduler->process[pid].state == READY){
		scheduler->cantReady--;
		removeElement(scheduler->readyList, &scheduler->process[pid].pid);
	} else if(scheduler->process[pid].state == BLOCKED){
		removeElement(scheduler->blockedList, &scheduler->process[pid].pid);
	}

	if(scheduler->process[pid].argv != NULL){
		for(int i = 0; i < scheduler->process[pid].argc; i++){
			freeMemory(scheduler->process[pid].argv[i]);
		}
		freeMemory(scheduler->process[pid].argv);
	}
	if(scheduler->process[pid].basePointer != 0){
		freeMemory((void*)(scheduler->process[pid].basePointer - STACK_SIZE));
	}
	if((int) scheduler->process[pid].stack != 0){
		free((void*) scheduler->process[pid].stack);
	}

	scheduler->process[pid].stack = 0;
    scheduler->process[pid].basePointer = 0;
    scheduler->process[pid].state = DEAD;
    scheduler->process[pid].rsp = NULL;
    scheduler->process[pid].priority = 0;
    scheduler->process[pid].quantum = FIRST_QUANTUM;
    scheduler->process[pid].foreground = 0;
    scheduler->process[pid].argv = NULL;
    scheduler->process[pid].argc = 0;
    scheduler->process[pid].rip = 0;
    scheduler->process[pid].name = NULL;
    scheduler->process[pid].fds[0] = 0;
    scheduler->process[pid].fds[1] = 0;
    scheduler->process[pid].fds[2] = 0;

	uint16_t ppid = scheduler->process[pid].ppid;
	if(ppid != pid){
		removeElement(scheduler->process[ppid].childList, &scheduler->process[pid].pid);

		if(scheduler->process[ppid].childSem != -1){
			if(isEmpty(scheduler->process[ppid].childList)){
				semPost(scheduler->process[ppid].childSem);
				semClose(scheduler->process[ppid].childSem);
				scheduler->process[ppid].childSem = -1;
			}
		}
	}

	if(scheduler->process[pid].childList != NULL){
		freeList(scheduler->process[pid].childList);
		scheduler->process[pid].childList = NULL;
	}
	scheduler->count--;
	yield();
	return 0;
}

int setPriority(uint16_t pid, uint8_t newPriority){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL || pid >= MAX_PROCESSES || newPriority < 0){
		return -1;
	}
	if(scheduler->process[pid].state == DEAD){
		return -1;
	}
	scheduler->process[pid].priority = newPriority;
	scheduler->process[pid].quantum = FIRST_QUANTUM * (1+ newPriority);
	return 0;
}

int setStatus(uint16_t pid, State status){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL || pid >= MAX_PROCESSES){
		return -1;
	}
	scheduler->process[pid].state = status;
	return 0;
}

void yield(){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return;
	}
	scheduler->process[scheduler->currentPID].quantum = 0;
	callTimerTick();
}

uint16_t createProcess(EntryPoint rip, char **argv, int arc, uint8_t priority, uint16_t fileDescriptors[]){
	if(rip == NULL || argv == NULL || arc < 0){
		return -1;
	}

	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return -1;
	}

	if(scheduler->count >= MAX_PROCESSES){
		return -2;
	}

	int i = 0;
	while(scheduler->process[i].state != DEAD && i < MAX_PROCESSES){
		i++;
	}

	uint8_t pid = i;

	scheduler->process[pid].state = READY;
	scheduler->process[pid].priority = priority;
	scheduler->process[pid].quantum = FIRST_QUANTUM * (1 + priority);
	scheduler->process[pid].pid = pid;
	scheduler->quantum = (pid == 0) ? FIRST_QUANTUM : scheduler->process[pid].quantum;
	scheduler->process[pid].rip = (EntryPoint)processWrapper;
	scheduler->process[pid].argc = arc;

	uint16_t ppid;
	if(scheduler->count == 1){
		ppid = pid;
	} else {
		ppid = scheduler->currentPID;
	}
	scheduler->process[pid].ppid = ppid;
	scheduler->process[pid].childList = createDoubleLinkedList();
	scheduler->process[pid].childSem = -1;

	if(ppid != pid && scheduler->process[pid].foreground){
		insertLast(scheduler->process[ppid].childList, &scheduler->process[pid].pid);
	}
	scheduler->process[pid].name == allocMemory((strlen(argv[0]) + 1));
	if(scheduler->process[pid].name == NULL){
		return -1;
	}
	strncpy(scheduler->process[pid].name, argv[0], strlen(argv[0]) + 1);

	scheduler->process[pid].argv = allocArgv(&scheduler->process[pid], argv, arc);
	if(scheduler->process[pid].argv == NULL){
		freeMemory(scheduler->process[pid].name);
		return -1;
	}

	scheduler->process[pid].stack = (uint64_t)allocMemory(STACK_SIZE);
	if(scheduler->process[pid].stack == 0){
		free((void*)scheduler->process[pid].argv);
		freeMemory(scheduler->process[pid].name);
		return -1;
	}
	scheduler->process[pid].basePointer = (scheduler->process[pid].stack + STACK_SIZE);
	for(int i = 0; i < FILE_DESCRPIPTORS; i++){
		scheduler->process[pid].fds[i] = fileDescriptors[i];
	}

	if(scheduler->readyList->first == NULL){
		insertFirst(scheduler->readyList, &scheduler->process[pid].pid);
	} else {
		insertLast(scheduler->readyList, &scheduler->process[pid].pid);
	}

	scheduler->count++;
	scheduler->cantReady++;
	scheduler->process[pid].rsp = (void*) setStackFrame(scheduler->process[pid].basePointer, (uint64_t)scheduler->process[pid].rip, scheduler->process[pid].argc, scheduler->process[pid].argv, (EntryPoint)rip);

	return pid;
}

uint16_t getPid(){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return -1;
	}
	return scheduler->currentPID;
}

void switchProcess(){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return;
	}
	uint8_t pid = scheduler->currentPID;

	if(scheduler->process[pid].state == RUNNING){
		scheduler->process[pid].state = READY;
	}

	Node *aux = scheduler->readyList->first;
	if(aux->next != NULL){
		scheduler->currentPID = *(uint8_t*)aux->next->data;
		scheduler->readyList->first = aux->next;
		scheduler->readyList->first->prev = NULL;
		aux->next = NULL;
		scheduler->readyList->last->next = aux;
		aux->prev = scheduler->readyList->last;
		scheduler->readyList->last = aux;
	} else {
		scheduler->currentPID = *(uint8_t*)aux->data;
	}

	scheduler->process[scheduler->currentPID].state = RUNNING;
}

int blockProcess(uint16_t pid){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return -1;
	}

	if(scheduler->process[pid].state == RUNNING){
		scheduler->process[pid].state = BLOCKED;
		removeElement(scheduler->readyList, &scheduler->process[pid].pid);
		insertLast(scheduler->blockedList, &scheduler->process[pid].pid);
		yield();
	} else if(scheduler->process[pid].state == READY){
		scheduler->process[pid].state = BLOCKED;
		removeElement(scheduler->readyList, &scheduler->process[pid].pid);
		insertLast(scheduler->blockedList, &scheduler->process[pid].pid);
	}
	scheduler->cantReady--;
	return 0;
}

PCB* getProcess(uint16_t pid){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL || pid >= MAX_PROCESSES){
		return NULL;
	}
	return &scheduler->process[pid];
}

int unblockProcess(uint16_t pid){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return -1;
	}

	if(scheduler->process[pid].state == BLOCKED){
		scheduler->process[pid].state = READY;
		if(scheduler->readyList->first == NULL){
			insertFirst(scheduler->readyList, &scheduler->process[pid].pid);
		} else {
			insertLast(scheduler->readyList, &scheduler->process[pid].pid);
		}
		removeElement(scheduler->blockedList, &scheduler->process[pid].pid);
	}
	scheduler->cantReady++;
	return 0;
}

void* schedule(void* processStackPointer){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL || scheduler->count == 0){
		return processStackPointer;
	}

	if(scheduler->process[scheduler->currentPID].quantum = 0 && scheduler->readyList != NULL){
		scheduler->process[scheduler->currentPID].quantum = FIRST_QUANTUM * (1 + scheduler->process[scheduler->currentPID].priority);
	}

	scheduler->quantum--;
	if(scheduler->quantum > 0 && scheduler->process[scheduler->currentPID].state == RUNNING){
		return processStackPointer;
	}

	if(scheduler->cantReady == 1){
		return scheduler->process[1].rsp;
	}

	if(scheduler->quantum == 0 || scheduler->process[scheduler->currentPID].state != RUNNING){
		if(scheduler->started != 0){
			scheduler->process[scheduler->currentPID].rsp = processStackPointer;
		}
		switchProcess();
		scheduler->quantum = scheduler->process[scheduler->currentPID].quantum;
		scheduler->started = 1;
	}

	if(scheduler->process[scheduler->currentPID].rsp == NULL){
		killProcess(scheduler->currentPID);
		return schedule(processStackPointer);
	}

	return scheduler->process[scheduler->currentPID].rsp;
}

void processWrapper(void (*entryPoint)(int, char**), int argc, char** argv){
	entryPoint(argc, argv);
	SchedulerADT scheduler = getSchedulerADT();
	killProcess(scheduler->currentPID);
}

ProcessInfo* getProcessInfo(int* size){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return NULL;
	}

	ProcessInfo* processInfoList = allocMemory(sizeof(ProcessInfo) * scheduler->count);
	if(processInfoList == NULL){
		return NULL;
	}

	int index = 0;
	for(int i = 0; i < MAX_PROCESSES && index < scheduler->count; i++){
		if(scheduler->process[i].state != DEAD){
			processInfoList[index].name = scheduler->process[i].name;
			processInfoList[index].pid = scheduler->process[i].pid;
			processInfoList[index].priority = scheduler->process[i].priority;
			processInfoList[index].foreground = scheduler->process[i].foreground;
			processInfoList[index].stack = scheduler->process[i].stack;
			processInfoList[index].state = scheduler->process[i].state;
			index++;
		}
	}
	*size = index;
	return processInfoList;
}

int changeFd(uint16_t pid, uint16_t fileDescriptors[]){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL || pid >= MAX_PROCESSES || scheduler->process[pid].state == DEAD){
		return -1;
	}
	for(int i = 0; i < FILE_DESCRPIPTORS; i++){
		scheduler->process[pid].fds[i] = fileDescriptors[i];
	}
	return 0;
}

int getFileDescriptor(uint8_t fd){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL && scheduler->currentPID < MAX_PROCESSES){
		return -1;
	}
	return scheduler->process[scheduler->currentPID].fds[fd];
}