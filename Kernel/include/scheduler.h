#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../../Shared/shared_structs.h"
#include <defs.h>
#include <process.h>
#include <stdint.h>

/**
 * @brief Schedules the next process to run
 * @param rsp Current stack pointer
 * @return uint64_t New stack pointer for next process
 */
uint64_t scheduleNext(uint64_t rsp);

/**
 * @brief Initializes the scheduler with an idle process
 * @param idle Function pointer to idle process
 */
void startScheduler(processFun idle);

/*
 * startScheduler
 * @param idle: function pointer used as the idle process entry
 * Initializes scheduler internals and sets the idle process.
 */
void startScheduler(processFun idle);

/*
 * createProcess
 * @param name: process name (string)
 * @param function: entry point for the process
 * @param argc: number of arguments
 * @param arg: argv-style argument array
 * @param priority: scheduling priority value
 * @param foreground: non-zero for foreground, 0 for background
 * @param stdin: initial stdin file descriptor
 * @param stdout: initial stdout file descriptor
 * @return: pid_t of created process, or -1 on failure
 */
pid_t createProcess(char *name, processFun function, uint64_t argc, char **arg, uint8_t priority, char foreground,
                    int stdin, int stdout);

/*
 * getCurrentPid
 * @return: pid_t of current running process, or -1 if none
 */
pid_t getCurrentPid();

/*
 * getForegroundPid
 * @return: pid_t of the foreground process, or -1 if none
 */
pid_t getForegroundPid();

/*
 * blockProcess
 * Blocks the process with given `pid` (scheduler-level blocking)
 * @return: 0 on success, -1 on failure
 */
uint64_t blockProcess(pid_t pid);

/*
 * yield
 * Voluntarily yields CPU to the next process (no parameters, no return)
 */
void yield();

/*
 * unblockProcess
 * Unblocks the process identified by `pid` and makes it ready.
 * @return: 0 on success, -1 on failure
 */
uint64_t unblockProcess(pid_t pid);

/*
 * kill
 * Kills the process with `pid` and sets its return value to `retValue`.
 * @return: 0 on success, -1 on failure
 */
uint64_t kill(pid_t pid, uint64_t retValue);

/*
 * waitpid
 * Waits for process `pid` to terminate. If pid==-1 waits for any child.
 * @param retValue: pointer to int32_t to receive exit status
 * @return: pid of the waited process, or -1 on failure
 */
pid_t waitpid(pid_t pid, int32_t *retValue);

/*
 * changePrio
 * Changes the priority of process `pid` to `newPrio`.
 * @return: the new priority on success, -1 on failure
 */
int8_t changePrio(pid_t pid, int8_t newPrio);

/*
 * blockProcessBySem
 * Blocks `pid` on a semaphore (used internally by semaphores)
 * @return: 0 on success, -1 on failure
 */
uint64_t blockProcessBySem(pid_t pid);

/*
 * unblockProcessBySem
 * Unblocks `pid` that was blocked by a semaphore
 * @return: 0 on success, -1 on failure
 */
uint64_t unblockProcessBySem(pid_t pid);

/*
 * getProcessInfo
 * Fills an array of PCBs describing current processes (caller owns returned array)
 * @param cantProcesses: pointer where the function stores number of processes
 * @return: pointer to first PCB element or NULL on failure
 */
PCB *getProcessInfo(uint64_t *cantProcesses);

/*
 * copyProcess
 * Copies the contents of `src` into `dest` safely.
 * @return: 0 on success, -1 on failure
 */
int16_t copyProcess(PCB *dest, PCB *src);

/*
 * getCurrentStdin
 * @return: stdin file descriptor for current process or -1
 */
int getCurrentStdin();

/*
 * getCurrentStdout
 * @return: stdout file descriptor for current process or -1
 */
int getCurrentStdout();

#endif /* SCHEDULER_H */