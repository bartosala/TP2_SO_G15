#include "../include/memoryManager.h"
#include "../include/process.h"
#include "../include/scheduler.h"
#include "../include/interrupts.h"
#include "../include/defs.h"
#include <lib.h>
#include <stddef.h>
#include <string.h>
#include <shared_structs.h>
typedef struct ProcessManagerCDT {
    PCB* foregroundProcess;
    PCB* currentProcess;
    QueueADT readyQueue;
    QueueADT blockedQueue;
    QueueADT blockedQueueBySem;
    QueueADT zombieQueue;
    PCB* idleProcess;
} ProcessManagerCDT;

ProcessManagerADT createProcessManager() {
    ProcessManagerADT processManager = malloc(sizeof(ProcessManagerCDT));
    if(processManager == NULL) {
        return NULL;
    }   
    processManager->foregroundProcess = NULL;
    processManager->currentProcess = NULL;
    processManager->readyQueue = createQueue();
    if(processManager->readyQueue == NULL) {
        freeMemory(processManager);
        return NULL;
    }
    processManager->blockedQueue = createQueue();
    if(processManager->blockedQueue == NULL) {
        destroyQueue(processManager->readyQueue);
        freeMemory(processManager);
        return NULL;
    }
    processManager->blockedQueueBySem = createQueue();
    if(processManager->blockedQueueBySem == NULL) {
        destroyQueue(processManager->readyQueue);
        destroyQueue(processManager->blockedQueue);
        destroyQueue(processManager->zombieQueue); // nescesario ? creo que no zombie va despues 
        freeMemory(processManager);
        return NULL;
    }
    processManager->zombieQueue = createQueue();
    if(processManager->zombieQueue == NULL) {
        destroyQueue(processManager->readyQueue);
        destroyQueue(processManager->blockedQueue);
        destroyQueue(processManager->blockedQueueBySem);
        freeMemoryMemory(processManager);
        return NULL;
    }
    processManager->idleProcess = NULL;
    return processManager;
}

int compareByPid(void* a, void* b) {
    PCB* pcbA = (PCB*)a;
    PCB* pcbB = (PCB*)b;
    return (pcbA->pid == pcbB->pid) ? 0 : -1;
}

int hasPid(void* a, void*b){ 
    if (a == NULL || b == NULL) {
        return -1;
    }
    PCB* processA = (PCB*)a;
    pid_t* pid = (pid_t*)b;
    
    return (processA->pid == *pid) ? 0 : -1;
}

void addProcess(ProcessManagerADT pm, PCB* process) {
    if(pm == NULL || process == NULL) {
        return;
    }

    if(pm->currentProcess == NULL  || pm->currentProcess == pm->idleProcess) {
        pm->currentProcess = process;
        return;
    }
    enqueue(pm->readyQueue, process);
}


void removeFromReady(ProcessManagerADT pm, pid_t pid) { // VER ESTA FUNCION
    if(pm == NULL) {
        return;
    }

    remove(list->readyQueue , &pid, hasPid);
    if(list->foregroundProcess && list->foregroundProcess->pid == pid) {
        list->foregroundProcess = NULL;
    }
}

void removeFromZombie(ProcessManagerADT pm, pid_t pid) {
    if(pm == NULL) {
        return;
    }

    remove(pm->zombieQueue , &pid, hasPid);
}

static PCB* switchProcessFromQueues(QueueADT queueFrom, QueueADT queueTo, pid_t pid) {
    if(queueFrom == NULL || queueTo == NULL) {
        return NULL;
    }
    // Lo remuevo de la cola origen
    PCB* process = (PCB*)remove(queueFrom, &pid, hasPid);
    if(process == NULL) {
        return NULL;
    }
    // Lo agrego a la cola destino
    enqueue(queueTo, process); // ver 
    return process;
}

int blockProcessQueue(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return -1; 
    }
    QueueADT queue = list->readyQueue;
    PCB* process = (PCB*)contains(list->readyQueue, &pid, hasPid);
    if (process == NULL) {
        process = (PCB*)contains(list->blockedQueueBySem, &pid, hasPid);
        if(process == NULL) {
            return -1; 
        }
        queue = list->blockedQueueBySem;
    }
    
    process = switchProcess(queue, list->blockedQueue, pid);
    if (process == NULL) {
        return -1;
    }
    
    if (process->state != BLOCKED) {
        process->state = BLOCKED;
    }
    
    return 0;
}

int blockProcessQueueBySem(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return -1; 
    }
    
    PCB* process = switchProcess(list->readyQueue, list->blockedQueueBySem, pid);
    if (process == NULL) {
        return -1; 
    }
    
    if (process->state != BLOCKED) {
        process->state = BLOCKED;
    }
    
    return 0;
}
// Misma idea que blockProcessQueue
int unblockProcessQueue(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return -1; 
    }
    
    PCB* process = switchProcess(list->blockedQueue, list->readyQueue, pid);
    if (process == NULL) {
        return -1; 
    }
    
    if (process->state != READY) {
        process->state = READY;
    }
    
    return 0;
}

int unblockProcessQueueBySem(ProcessManagerADT list, pid_t pid) {
    if (list == NULL) {
        return -1; 
    }
    
    PCB* process = switchProcess(list->blockedQueueBySem, list->readyQueue, pid);
    if (process == NULL) {
        return -1; 
    }
    
    if (process->state != READY) {
        process->state = READY;
    }
    
    return 0;
}

void freeProcessLinkedLists(ProcessManagerADT pm) {
    if(pm == NULL) {
        return;
    }
    destroyQueue(pm->readyQueue);
    destroyQueue(pm->blockedQueue);
    destroyQueue(pm->blockedQueueBySem); // estos dos tambien ? 
    destroyQueue(pm->zombieQueue);
    freeMemory(pm);
}

PCB* getProcess(ProcessManagerADT pm, pid_t pid) {
    if(pm == NULL) {
        return NULL;
    }

    PCB* process = (PCB*)contains(pm->readyQueue, &pid, hasPid);
    if(process != NULL) {
        return process;
    }

    process = (PCB*)contains(pm->blockedQueue, &pid, hasPid);
    if(process != NULL) {
        return process;
    }

    process = (PCB*)contains(pm->blockedQueueBySem, &pid, hasPid);
    if(process != NULL) {
        return process;
    }

    process = (PCB*)contains(pm->zombieQueue, &pid, hasPid);
    if(process != NULL) {
        return process;
    }

    if(pm->currentProcess != NULL && pm->currentProcess->pid == pid) {
        return pm->currentProcess;
    }

    if(pm->idleProcess != NULL && pm->idleProcess->pid == pid) {
        return pm->idleProcess;
    }

    return NULL; // no deberia llegar aca nunca
}

PCB* getNextReadyProcess(ProcessManagerADT pm) {
    if(pm == NULL) {
        return NULL;
    }

    if(isQueueEmpty(pm->readyQueue)) {
        pm->currentProcess = pm->idleProcess;
        return pm->idleProcess;
    }

    PCB* nextProcess = (PCB*)dequeue(pm->readyQueue);
    if(nextProcess == NULL) {
        return NULL;
    }

    if(enqueue(pm->readyQueue, nextProcess) == 0) {
        return NULL;
    }

    pm->currentProcess = nextProcess;
    if(foregroundProcess(nextProcess)) {
        foregroundProcessSet(pm, nextProcess);
    }
    return nextProcess;
}

int hasNextReadyProcess(ProcessManagerADT pm) {
    if(pm == NULL) {
        return 0;
    }

    return !isQueueEmpty(pm->readyQueue);
}

PCB* getCurrentProcess(ProcessManagerADT pm) {
    if(pm == NULL) {
        return NULL;
    }

    return pm->currentProcess;
}


void setIdleProcess(ProcessManagerADT pm, PCB* idleProcess) {
    if(pm == NULL || idleProcess == NULL) {
        return;
    }

    pm->idleProcess = idleProcess;
    pm->currentProcess = idleProcess;
}

void foregroundProcessSet(ProcessManagerADT pm, PCB* process) {
    if(pm == NULL) {
        return;
    }

    pm->foregroundProcess = process;
}

PBC* getIdleProcess(ProcessManagerADT pm) {
    if(pm == NULL) {
        return NULL;
    }

    return pm->idleProcess;
}   

uint_64 processCount(ProcessManagerADT pm) {
    if(pm == NULL) {
        return 0;
    }

    uint_64 count = 0;
    count += queueSize(pm->readyQueue);
    count += queueSize(pm->blockedQueue);
    count += queueSize(pm->blockedQueueBySem);
    count += queueSize(pm->zombieQueue);
    count++; 
    
    return count;   
}

uint_64 readyProcessCount(ProcessManagerADT pm) {
    if(pm == NULL) {
        return 0;
    }

    return queueSize(pm->readyQueue);
}

uint_64 blockedProcessCount(ProcessManagerADT pm) {
    if(pm == NULL) {
        return 0;
    }

    return queueSize(pm->blockedQueue) + queueSize(pm->blockedQueueBySem);
}

uint_64 zombieProcessCount(ProcessManagerADT pm) {
    if(pm == NULL) {
        return 0;
    }

    return queueSize(pm->zombieQueue);
}

PCB* getForegroundProcess(ProcessManagerADT pm) {
    if(pm == NULL) {
        return NULL;
    }

    return pm->foregroundProcess;
}

bool isCurrentProcessForeground(ProcessManagerADT pm, pid_t pid) {
    retunr pm && pm->foregroundProcess && pm->foregroundProcess->pid == pid;
}

bool isIdleProcess(ProcessManagerADT pm, pid_t pid) {
    return pm && pm->idleProcess && pm->idleProcess->pid == pid;
}

bool isForegroundProcess(PCB* process) {
    return process && process->isForeground;
}

PCB * killProcess(ProcessManagerADT, pid_t pid, uint_64 ret, State state) {
    if (pm == NULL) {
        return NULL;
    }

    if(pm->currentProcess && pm->currentProcess->pid == pid) {
        pm->currentProcess = pm->idleProcess;
    }
    // Intento sacarlo de todas las colas
    PCB* process = switchProcessFromQueues(pm->readyQueue, pm->zombieQueue, pid);
    if(process == NULL) {
        process = switchProcessFromQueues(pm->blockedQueue, pm->zombieQueue, pid);
        if(process == NULL) {
            process = switchProcessFromQueues(pm->blockedQueueBySem, pm->zombieQueue, pid);
            if(process == NULL) {
                return NULL;
            }
    }

    if(list->foregroundProcess && list->foregroundProcess->pid == pid) {
        list->foregroundProcess = NULL;
    }

    process->retValue = ret;
    process->state = state;
    return process;
}   

int stdintProcess(ProcessManagerADT pm, pid_t pid){
    PCB * process = getProcess(pm, pid);
    if(process == NULL){
        return -1;
    }
    return process->stdin;
}

int stdoutProcess(ProcessManagerADT pm, pid_t pid){
    PCB * process = getProcess(pm, pid);
    if(process == NULL){
        return -1;
    }
    return process->stdout;
}


void addToReady(ProcessManagerADT list, PCB* process) {
    if (list == NULL || process == NULL){ 
        return;
    }
    enqueue(list->readyQueue, process);
}

void addToBlocked(ProcessManagerADT list, PCB* process) {
    if (list == NULL || process == NULL) {
        return;
    }
    enqueue(list->blockedQueue, process);
}

void addToBlockedBySem(ProcessManagerADT list, PCB* process) {
    if (list == NULL || process == NULL) {
        return
    };
    enqueue(list->blockedQueueBySem, process);
}