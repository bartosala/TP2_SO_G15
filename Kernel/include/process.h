#ifndef PROCESS_H
#define PROCESS_H

#include "../../Shared/shared_structs.h"
#include "queue.h"
#include <stdbool.h>
#include <stdint.h>

#define STACK_SIZE 4096 /* one page */

typedef struct ProcessManagerCDT *ProcessManagerADT;

/* lifecycle */
/*
 * createProcessManager
 * @return: newly allocated ProcessManagerADT, or NULL on failure
 */
ProcessManagerADT createProcessManager(void);

/*
 * freeProcessLinkedLists
 * Frees internal lists and resources used by the process manager.
 * @param pm: process manager instance to free
 */
void freeProcessLinkedLists(ProcessManagerADT pm);

/* basic operations */
/*
 * addProcess
 * @param pm: process manager
 * @param process: pointer to a PCB to add to management structures
 */
void addProcess(ProcessManagerADT pm, PCB *process);

/*
 * removeFromReady
 * Removes the process with `pid` from the ready queue (if present).
 * @param pm: process manager
 * @param pid: process id to remove
 */
void removeFromReady(ProcessManagerADT pm, pid_t pid);

/*
 * removeFromZombie
 * Removes the process with `pid` from the zombie list.
 * @param pm: process manager
 * @param pid: process id to remove
 */
void removeFromZombie(ProcessManagerADT pm, pid_t pid);

/* queue switches / blocking */
/*
 * blockProcessQueue
 * Blocks the process with `pid` by moving it to a blocked queue.
 * @return: 0 on success, -1 on failure
 */
int blockProcessQueue(ProcessManagerADT list, pid_t pid);

/*
 * blockProcessQueueBySem
 * Blocks the process with `pid` due to a semaphore wait.
 * @return: 0 on success, -1 on failure
 */
int blockProcessQueueBySem(ProcessManagerADT list, pid_t pid);

/*
 * unblockProcessQueue
 * Unblocks a process with `pid` and moves it back to ready queue.
 * @return: 0 on success, -1 on failure
 */
int unblockProcessQueue(ProcessManagerADT list, pid_t pid);

/*
 * unblockProcessQueueBySem
 * Unblocks a process previously blocked by a semaphore.
 * @return: 0 on success, -1 on failure
 */
int unblockProcessQueueBySem(ProcessManagerADT list, pid_t pid);

/* queries */
/*
 * getProcess
 * @param pm: process manager
 * @param pid: process id to lookup
 * @return: pointer to PCB or NULL if not found
 */
PCB *getProcess(ProcessManagerADT pm, pid_t pid);

/*
 * getNextReadyProcess
 * Returns next ready process (scheduler order) without removing it.
 * @return: pointer to PCB or NULL if none
 */
PCB *getNextReadyProcess(ProcessManagerADT pm);

/*
 * getNextProcess
 * Returns next process for scheduling (may remove or return depending on impl).
 * @return: pointer to PCB or NULL if none
 */
PCB *getNextProcess(ProcessManagerADT pm);

/*
 * hasNextReadyProcess
 * @return: non-zero if there is at least one ready process, 0 otherwise
 */
int hasNextReadyProcess(ProcessManagerADT pm);

/*
 * getCurrentProcess
 * @return: pointer to the current running PCB, or NULL if none
 */
PCB *getCurrentProcess(ProcessManagerADT pm);

/*
 * setIdleProcess
 * Sets the idle process PCB used when no other ready processes exist.
 */
void setIdleProcess(ProcessManagerADT pm, PCB *idleProcess);

/*
 * foregroundProcessSet
 * Marks `process` as the foreground process (receives input, etc.)
 */
void foregroundProcessSet(ProcessManagerADT pm, PCB *process);

/*
 * getIdleProcess
 * @return: pointer to the idle process PCB
 */
PCB *getIdleProcess(ProcessManagerADT pm);

/* counters */
uint64_t processCount(ProcessManagerADT pm);
uint64_t readyProcessCount(ProcessManagerADT pm);
uint64_t blockedProcessCount(ProcessManagerADT pm);
uint64_t zombieProcessCount(ProcessManagerADT pm);

/*
 * getForegroundProcess
 * @return: pointer to foreground process PCB, or NULL if none
 */
PCB *getForegroundProcess(ProcessManagerADT pm);

/*
 * isCurrentProcessForeground
 * @return: true if the current process with `pid` is the foreground process
 */
bool isCurrentProcessForeground(ProcessManagerADT pm, pid_t pid);

/*
 * isIdleProcess
 * @return: true if `pid` identifies the idle process
 */
bool isIdleProcess(ProcessManagerADT pm, pid_t pid);

/*
 * isForegroundProcess
 * @return: true if `process` is marked as foreground
 */
bool isForegroundProcess(PCB *process);

/* kill / terminate */
/*
 * killProcess
 * Terminates the process identified by `pid` with return code `ret` and
 * final `state` (e.g. ZOMBIE). Returns the PCB of the killed process or
 * NULL on failure.
 */
PCB *killProcess(ProcessManagerADT pm, pid_t pid, uint64_t ret, State state);

/* fds helpers */
/*
 * stdintProcess
 * @return: stdin file descriptor for `pid`, or -1 if none
 */
int stdintProcess(ProcessManagerADT pm, pid_t pid);

/*
 * stdoutProcess
 * @return: stdout file descriptor for `pid`, or -1 if none
 */
int stdoutProcess(ProcessManagerADT pm, pid_t pid);

/* enqueue helpers */
void addToReady(ProcessManagerADT list, PCB *process);
void addToBlocked(ProcessManagerADT list, PCB *process);
void addToBlockedBySem(ProcessManagerADT list, PCB *process);

#endif /* PROCESS_H */