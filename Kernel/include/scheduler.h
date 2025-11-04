#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <process.h>
#include <defs.h>
#include "../../Shared/shared_structs.h"

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

/**
 * @brief Creates a new process
 * @param name Process name
 * @param function Entry point function
 * @param argc Argument count
 * @param arg Argument array
 * @param priority Process priority
 * @param foreground Whether process is foreground (1) or background (0)
 * @return pid_t Process ID or -1 on failure
 */
pid_t createProcess(char* name, processFun function, uint64_t argc, char **arg, uint8_t priority, char foreground, int stdin, int stdout);

/**
 * @brief Gets the current process ID
 * @return pid_t Current process ID or -1 if none
 */
pid_t getCurrentPid();

/**
 * @brief Gets the foreground process ID
 * @return pid_t Foreground process ID or -1 if none
 */
pid_t getForegroundPid();

/**
 * @brief Blocks a process
 * @param pid Process ID to block
 * @return uint64_t 0 on success, -1 on failure
 */
uint64_t blockProcess(pid_t pid);

/**
 * @brief Yields CPU to next process
 */
void yield();

/**
 * @brief Unblocks a process
 * @param pid Process ID to unblock
 * @return uint64_t 0 on success, -1 on failure
 */
uint64_t unblockProcess(pid_t pid);

/**
 * @brief Kills a process
 * @param pid Process ID to kill
 * @param retValue Return value for the killed process
 * @return uint64_t 0 on success, -1 on failure
 */
uint64_t kill(pid_t pid, uint64_t retValue);

/**
 * @brief Waits for a process to terminate
 * @param pid Process ID to wait for
 * @param retValue Pointer to store exit status
 * @return pid_t Process ID that was waited for or -1 on failure
 */
pid_t waitpid(pid_t pid, int32_t* retValue);

/**
 * @brief Changes process priority
 * @param pid Process ID
 * @param newPrio New priority value
 * @return int8_t New priority or -1 on failure
 */
int8_t changePrio(pid_t pid, int8_t newPrio);

/**
 * @brief Blocks a process by semaphore
 * @param pid Process ID to block
 * @return uint64_t 0 on success, -1 on failure
 */
uint64_t blockProcessBySem(pid_t pid);

/**
 * @brief Unblocks a process blocked by semaphore
 * @param pid Process ID to unblock
 * @return uint64_t 0 on success, -1 on failure
 */
uint64_t unblockProcessBySem(pid_t pid);

/**
 * @brief Gets information about all processes
 * @param cantProcesses Pointer to store process count
 * @return PCB* Array of process information or NULL on failure
 */
PCB* getProcessInfo(uint64_t *cantProcesses);

/**
 * @brief Copies process information
 * @param dest Destination PCB
 * @param src Source PCB
 * @return int16_t 0 on success, -1 on failure
 */
int16_t copyProcess(PCB *dest, PCB *src);

/**
 * @brief Gets current process stdin
 * @return int File descriptor or -1 if none
 */
int getCurrentStdin();

/**
 * @brief Gets current process stdout
 * @return int File descriptor or -1 if none
 */
int getCurrentStdout();

#endif // SCHEDULER_H