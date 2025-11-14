#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <memoryManager.h>
#include <process.h>
#include <stddef.h>
#include <string.h>
#include <semaphore.h>
#include <pipe.h>
#include <doubleLinkedList.h>


#define STACK_SIZE 4096
#define FIRST_QUANTUM 1
#define MAX_PROCESSES 1000

typedef struct SchedulerCDT {
    PCB process[MAX_PROCESSES];
    uint8_t count;
    DoubleLinkedListADT readyList;
    DoubleLinkedListADT blockedList;
    uint8_t currentPID;
    uint64_t quantum;
    uint8_t started;
    uint8_t idlePID;
    uint8_t cantReady;
} SchedulerCDT;

typedef struct SchedulerCDT* SchedulerADT;

SchedulerADT getSchedulerADT();
int killProcess(uint16_t pid);
int changePriority(uint16_t pid, uint8_t newPriority);
uint16_t createProcess(EntryPoint rip, char **argv, int arc, uint8_t priority, uint16_t fileDescriptors[]);
void yield();
int setStatus(uint16_t pid, State status);
SchedulerADT createScheduler();
int blockProcess(uint16_t pid);
int unblockProcess(uint16_t pid);
PCB* getProcess(uint16_t pid);
uint16_t getPid();
void* schedule(void * processStackPointer);
int killFgProcess();
ProcessInfo* getProcessInfo(int* size);
int changeFd(uint16_t pid, uint16_t fileDescriptors[]);
int getFileDescriptor(uint8_t fd);

#endif // SCHEDULER_H