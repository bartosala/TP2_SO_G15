#include <stdint.h>
#include <memoryManager.h>
#include <process.h>
#include <semaphore.h>
#include <semaphore.h>
#include <scheduler.h>
#include "../../Shared/stdlib.h"

char** allocArgv(PCB* p, char** argv, int argc){
	char ** newArgv = allocMemory((argc + 1) * sizeof(char*));
	if(newArgv == NULL){
		return NULL;
	}

	for(int i = 0; i < argc; i++){
		newArgv[i] = allocMemory((strlen(argv[i]) + 1));
		if(newArgv[i] == NULL){
			for(int j = 0; j < i; j++){
				freeMemory(newArgv[j]);
			}
			freeMemory(newArgv);
			return NULL;
		}
		strcpy(newArgv[i], argv[i]);
	}
	newArgv[argc] = NULL;
	return newArgv;
}

void freeProcess(PCB* p){
	if(p == NULL){
		return;
	}
	freeMemory(&p->rsp);
	freeMemory(&p->stack);
	freeMemory(&p->basePointer);
	free(p);
}

uint16_t waitForChildern(){
	SchedulerADT scheduler = getSchedulerADT();
	if(scheduler == NULL){
		return -1;
	}

	uint16_t currentPID = getPid();
	PCB* currentProcess = &scheduler->process[currentPID];
	if(currentProcess == NULL || currentProcess->state == DEAD){
		return -1;
	}

	if(currentProcess->childList == NULL || isEmpty(currentProcess->childList)){
		return 0;
	}

	if(currentProcess->childSem == -1){
		currentProcess->childSem = currentProcess->pid + 2;
		semOpen(currentProcess->childSem);
		semInit(currentProcess->childSem, 0);
	}
	semWait(currentProcess->childSem);
	yield();
	return 0;
}

void idleProcess(){
	while(1){
		_hlt();
		yield();
	}
}