#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>
#include "process.h"
#include "double_linked_list.h"

#define SCHED_MIN_QUANTUM 1
#define SCHED_STACK_SIZE 0x1000
#define SCHED_MAX_PRIORITY 8

typedef struct SchedulerCDT {
    DoubleLinkedListADT readyLists[SCHED_MAX_PRIORITY];
    DoubleLinkedListADT blockedList;
    Process *current;
    uint16_t nextPid;
    uint64_t quantum;
    uint8_t started;
} SchedulerCDT;

typedef struct SchedulerCDT *SchedulerADT;

SchedulerADT getScheduler();
SchedulerADT createScheduler();

uint16_t Sched_createProcess(Code entryPoint, char *name, uint64_t argc, char *argv[], uint8_t priority);
int Sched_killProcess(uint16_t pid);
int Sched_blockProcess(uint16_t pid);
int Sched_unblockProcess(uint16_t pid);
int Sched_setPriority(uint16_t pid, uint8_t priority);
int Sched_yield(void);
uint64_t Sched_getPid(void);
int Sched_waitChildren(void);
void Sched_listProcesses(void);

// Called from asm interrupt handler. Must be exported as 'schedule'
uint64_t schedule(uint64_t rsp);

#endif
