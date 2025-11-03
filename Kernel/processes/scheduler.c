#include <scheduler.h>
#include <defs.h>
#include <memoryManager.h>
#include <interrupts.h>
#include "../../Shared/shared_structs.h"
#include <stackFrame.h> 
#include <lib.h>
#include <textModule.h>
#include <syscall.h>
#include <pipe.h>



#define IDLE_PRIORITY 6
#define MAX_PRIORITY 5
#define MIN_PRIORITY 0 

#define SHELL_PID 1

#define TIME_QUANTUM 3

#define MAX_PROCESS_NAME_LENGTH 32

static ProcessManagerADT processManager = NULL;
static pid_t currentPid = -1;
static pid_t nextPid = 0;
static uint64_t time_quantum = 0;

// Function prototypes
static PCB * createProcessOnPCB(char *name, processFun function, uint64_t argc, char **arg, int8_t priority, char foreground, int stdin, int stdout);
static void wakeUpWaitingParent(pid_t parentPid, pid_t childPid);

void startSchedueler(processFun idle){
    createPipeManager();
    
    ProcessManagerADT pm = createProcessManager();
    PCB* idleProcess = createProcessOnPCB("idle", idle, 0, NULL, IDLE_PRIORITY, 0, -1, -1);
    setIdleProcess(pm, idleProcess);
    processManager = pm;
}

pid_t createProcess(char* name, processFun function, uint64_t argc, char **arg, int8_t priority, char foreground, int stdin, int stdout) {
    PCB * newProcess = createProcessOnPCB(name, function, argc, arg, priority, foreground, stdin, stdout);
    return newProcess ? newProcess->pid : -1;
}

static PCB * createProcessOnPCB(char *name, processFun function, uint64_t argc, char **arg, int8_t priority, char foreground, int stdin, int stdout) {
    if(name == NULL || function == NULL || priority < MIN_PRIORITY || priority > MAX_PRIORITY ) {
        return NULL;
    }

    PCB *newProcess = allocMemory(sizeof(PCB));
    if(newProcess == NULL) {
        return NULL;
    }

    if(priority > IDLE_PRIORITY) {
        priority = MAX_PRIORITY;
    }

    strncpy(newProcess->name, name, MAX_PROCESS_NAME_LENGTH);
    newProcess->pid = nextPid++;
    newProcess->state = READY;
    newProcess->priority = priority;
    newProcess->foreground = foreground;
    newProcess->stdin = stdin;
    newProcess->stdout = stdout;
    newProcess->waitingForPid = -1;
    newProcess->retValue = 0;
    newProcess->parentPid = getCurrentPid();
    
    newProcess->entryPoint = (uint64_t)function;

    newProcess->rsp = setUpStackFrame(&newProcess->base, (uint64_t)function, argc, arg); 
    if(newProcess->rsp == 0) {
        freeMemory(newProcess);
        return NULL;
    }
    


    if(processManager == NULL) {
        freeMemory(newProcess);
        return NULL;
    }

    if(newProcess->pid > 1){
        if(!foreground && !stdin){
            newProcess->stdout = stdout;
            newProcess->state = BLOCKED;
            addToBlocked(processManager, newProcess);
            return newProcess;
        }
        newProcess->stdin = stdin ? getCurrentStdin() : stdin;
        newProcess->stdout = stdout ? getCurrentStdout() : stdout;

    }
    else if(newProcess->pid == SHELL_PID){
        newProcess->stdin = pipeCreate(0); // VER CUAL ES EL ID
        if(newProcess->stdin < 0){
            freeMemory((void*)newProcess->base - STACK_SIZE);
            freeMemory(newProcess);
            return NULL;
        }
        newProcess->stdout = 1; // terminal
    }
    if(priority != IDLE_PRIORITY) {
        addProcess(processManager, newProcess);
    }
    return newProcess;
}

pid_t getForegrounfdPid() {
    PCB* fgProcess = getForegroundProcess(processManager);
    return fgProcess ? fgProcess->pid : -1;
}

pid_t getCurrentPid() {
    PCB * currentProcess = getCurrentProcess(processManager);
    return currentProcess ? currentProcess->pid : -1;    
}

uint64_t schedule(uint64_t rsp) {
    static int firstCall = 1;
    PCB* currentProcess = getCurrentProcess(processManager);
    
    if(time_quantum > 0 && currentProcess->state == RUNNING && !isIdleProcess(processManager, currentProcess->pid)) {
        time_quantum--;
        return rsp;
    }

    if(!firstCall){
        currentProcess->rsp = rsp;
    }
    else {
        firstCall = 0;
    }
    if(currentProcess->state == RUNNING){
        currentProcess->state = READY;
    }

    PCB * nextProcess = getNextReadyProcess(processManager);

    nextProcess->state = RUNNING;
    currentPid = nextProcess->pid;
    time_quantum = nextProcess->priority == IDLE_PRIORITY ? TIME_QUANTUM : TIME_QUANTUM * (MAX_PRIORITY - nextProcess->priority + 1);
    return nextProcess->rsp;
}

uint64_t blockProcess(pid_t pid) {
    if(blockProcessQueue(processManager, pid)) {
        return -1;
    }
    if(pid == getCurrentPid()){
        yield();
    }   
    return 0;
}

uint64_t blockProcessBySem(pid_t pid) {
    if(blockProcessQueueBySem(processManager, pid)) {
        return -1;
    }
    if(pid == getCurrentPid()){
        yield();
    }
    return 0;
}

void yield() {
    time_quantum = 0;
    callTimerTick();
}

uint64_t unblockProcess(pid_t pid) {
    if(unblockProcessQueue(processManager, pid)) {
        return -1;
    }
    return 0;
}

uint64_t unblockProcessBySem(pid_t pid) {
    return unblockProcessQueueBySem(processManager, pid);
}

uint64_t kill(pid_t pid, uint64_t retValue) {
    if(pid <= 1) {
        return -1;
    }
    PCB* process = killProcess(processManager, pid, retValue, ZOMBIE);
    if(process == NULL) {
        return -1;
    }

    freeMemory((void*)process->base - STACK_SIZE);
    wakeUpWaitingParent(process->parentPid, pid);
    if(pid == getCurrentPid()) {
        yield();
    }

    return 0 ;
}

/*
This occurs for the child processes, where the entry is still needed to allow the parent
 process to read its child's exit status: once the exit status is read via the wait system call, the
  defunct process'
 entry is removed from the process table and it is said to be "reaped".*/
static int32_t reapCild(PCB* childProcess, int * retValue) {
    
    if(childProcess == NULL || childProcess->state != ZOMBIE) { 
        return -1;
    }

    if(retValue != NULL) {
        *retValue = childProcess->retValue;
    }
    int32_t c = childProcess->pid;
    childProcess->state = childProcess->retValue; 

    removeFromZombie(processManager, childProcess->pid);
    freeMemory((void*)childProcess->base - STACK_SIZE);
    freeMemory(childProcess);
    return c;
}

static void wakeUpWaitingParent(pid_t parentPid, pid_t childPid) {
    if(parentPid < 0) {
        return;
    }

    if(parentPid == getIdleProcess(processManager)->pid) {
        reapCild(getProcess(processManager, childPid), NULL);
    }


    PCB* parentProcess = getProcess(processManager, parentPid);
    if(parentProcess != NULL && parentProcess->waitingForPid == childPid) {
        unblockProcess(parentPid);
    }

    return;
}
// ver esta funcion 
int32_t waitpid(pid_t pid, int32_t* retValue) {
    pid_t currentPid = getCurrentPid();
    if(currentPid < 0) {
        return -1;
    }

    PCB * proc = getProcess(processManager, currentPid);
    if(proc == NULL) {
        return -1;
    }

    PCB * currentProcess = getCurrentProcess(processManager);

    if(currentProcess == NULL) {
        return -1;
    }

    if(proc->parentPid != currentPid && currentProcess->pid != pid) {
        return -1;
    }

    if(proc->state == ZOMBIE) {
        currentProcess->waitingForPid = -1;
        return reapCild(proc, retValue);
    }

    if(proc->state < ZOMBIE) {
        currentProcess->waitingForPid = pid;
        currentProcess->state = BLOCKED;

        if(blockProcess(currentPid)) {
            currentProcess->waitingForPid = -1;
            currentProcess->state = READY;
            return -1;
        }

        proc = getProcess(processManager, currentPid);
        if(proc == NULL) {
            return -1;
        }

        if(proc->state == ZOMBIE) {
            currentProcess->waitingForPid = -1;
            return reapCild(proc, retValue);
        } 
    }
    return -1; // should not reach here
}


int8_t changePrio(pid_t pid, int8_t newPrio) {
    if(newPrio < MIN_PRIORITY || newPrio > MAX_PRIORITY) {
        return -1;
    }

    PCB* process = getProcess(processManager, pid);
    if(process == NULL) {
        return -1;
    }

    process->priority = newPrio;
    return newPrio;
}

PCB* getProcessInfo(uint64_t *cantProcesses){
    if ( processManager == NULL) {
        *cantProcesses = 0;
        return NULL;
    }
    
    uint64_t count = processCount(processManager);

    PCB* processInfo = allocMemory(sizeof(PCB) * count);
    if (processInfo == NULL) {
        *cantProcesses = 0;
        return NULL;
    }
    
    uint64_t j = 0;
    for (uint64_t i = 0; j < count; i++) {
        PCB* process = getProcess(processManager, i);
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

int16_t copyProcess(PCB *dest, PCB *src){
    if (dest == NULL || src == NULL) {
        return -1;
    }
    
    dest->pid = src->pid;
    dest->parentPid = src->parentPid;
    dest->waitingForPid = src->waitingForPid;
    dest->priority = src->priority;
    dest->state = src->state;
    dest->rsp = src->rsp;
    dest->base = src->base;
    dest->entryPoint = src->entryPoint;
    dest->retValue = src->retValue;
    dest->foreground = src->foreground;
    dest->stdin = src->stdin;
    dest->stdout = src->stdout;
    strncpy(dest->name, src->name, MAX_PROCESS_NAME_LENGTH);
    
    return 0;
}

int getCurrentStdin(){
    PCB * currentProcess = getCurrentProcess(processManager);
    if(currentProcess == NULL){
        return -1;
    }
    return currentProcess->stdin;
}

int getCurrentStdout(){
    PCB * currentProcess = getCurrentProcess(processManager);
    if(currentProcess == NULL){
        return -1;
    }
    return currentProcess->stdout;
}




