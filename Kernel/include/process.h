#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <shared_structs.h>  // defines PCB, pid_t, State
#include "queue.h"

#define STACK_SIZE 4096 // una pagina

typedef struct ProcessManagerCDT *ProcessManagerADT;

/* lifecycle */
ProcessManagerADT createProcessManager(void);
void freeProcessLinkedLists(ProcessManagerADT pm);

/* basic operations */
void addProcess(ProcessManagerADT pm, PCB* process);
void removeFromReady(ProcessManagerADT pm, pid_t pid);
void removeFromZombie(ProcessManagerADT pm, pid_t pid);

/* queue switches / blocking */
int blockProcessQueue(ProcessManagerADT list, pid_t pid);
int blockProcessQueueBySem(ProcessManagerADT list, pid_t pid);
int unblockProcessQueue(ProcessManagerADT list, pid_t pid);
int unblockProcessQueueBySem(ProcessManagerADT list, pid_t pid);

/* queries */
PCB* getProcess(ProcessManagerADT pm, pid_t pid);
PCB* getNextReadyProcess(ProcessManagerADT pm);
int hasNextReadyProcess(ProcessManagerADT pm);
PCB* getCurrentProcess(ProcessManagerADT pm);

void setIdleProcess(ProcessManagerADT pm, PCB* idleProcess);
void foregroundProcessSet(ProcessManagerADT pm, PCB* process);
PCB* getIdleProcess(ProcessManagerADT pm);

uint64_t processCount(ProcessManagerADT pm);
uint64_t readyProcessCount(ProcessManagerADT pm);
uint64_t blockedProcessCount(ProcessManagerADT pm);
uint64_t zombieProcessCount(ProcessManagerADT pm);

PCB* getForegroundProcess(ProcessManagerADT pm);
bool isCurrentProcessForeground(ProcessManagerADT pm, pid_t pid);
bool isIdleProcess(ProcessManagerADT pm, pid_t pid);
bool isForegroundProcess(PCB* process);

/* kill / terminate */
PCB* killProcess(ProcessManagerADT pm, pid_t pid, uint64_t ret, State state);

/* fds helpers */
int stdintProcess(ProcessManagerADT pm, pid_t pid);
int stdoutProcess(ProcessManagerADT pm, pid_t pid);

/* enqueue helpers */
void addToReady(ProcessManagerADT list, PCB* process);
void addToBlocked(ProcessManagerADT list, PCB* process);
void addToBlockedBySem(ProcessManagerADT list, PCB* process);

#endif // PROCESS_H