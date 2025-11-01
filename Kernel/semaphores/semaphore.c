#include <stddef.h>
#include <semaphore.h>

#define USED 1 
#define NOT_USED 0

SemaphoreADT semaphore = NULL;

static int8_t isValidSemaphore(uint8_t sem);


static int8_t isValidSemaphore(uint8_t sem) {
    if ( semaphore == NULL || sem >= NUM_SEMS) {
        return 0;
    }
    return 1;
}

SemaphoreADT createSemaphoresManager() {
    SemaphoreADT semManager = (SemaphoreADT)malloc(sizeof(SemaphoreCDT));
    if (semManager != NULL) {
        for (int i = 0; i < NUM_SEMS; i++) {
            semManager->semaphores[i].value = 0;
            semManager->semaphores[i].lock = 0;
            semManager->semaphores[i].used = NOT_USED;
            semManager->semaphores[i].waitingProcesses = createDoubleLinkedList();
        }
    }
    return semManager;
}

int8_t semOpen(uint8_t sem) {
    if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == USED) {
        return -1;
    }
    return 0;
}

int8_t semClose(uint8_t sem) {
    if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == NOT_USED) {
        return -1;
    }
    semaphore->semaphores[sem].used = NOT_USED;
    semaphore->semaphores[sem].value = 0;

    freeList(semaphore->semaphores[sem].waitingProcesses);
    semaphore->semaphores[sem].waitingProcesses = createDoubleLinkedList();
    return 0;
}

int8_t semWait(uint8_t sem) {
    if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == NOT_USED) {
        return -1;
    }

    acquire(&semaphore->semaphores[sem].lock);
    if (semaphore->semaphores[sem].value > 0) {
        semaphore->semaphores[sem].value--;
        release(&semaphore->semaphores[sem].lock);
        return 0;
    }
    int * pid = (int *)allocMemory(sizeof(int));
    if(pid == NULL){
        release(&semaphore->semaphores[sem].lock);
        return -1;
    }
    // *pid = Sched_getPid(); cambiar el nombre
    insertLast(semaphore->semaphores[sem].waitingProcesses, pid);
    release(&semaphore->semaphores[sem].lock);
    //Sched_yield(); // cambiar el nombre
    return 0;
}

int8_t semPost(uint8_t sem) {
    if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == NOT_USED) {
        return -1;
    }
    acquire(&semaphore->semaphores[sem].lock);

    while(!isEmpty(semaphore->semaphores[sem].waitingProcesses)) {
        int * pid = (int *)getFirst(semaphore->semaphores[sem].waitingProcesses);
        removeElement(semaphore->semaphores[sem].waitingProcesses, pid);
        // Sched_unblockProcess(*pid); cambiar el nombre
        Process *pcb = Sched_getProcessByPid(*pid); //cambiar nombre de la funcion
        if(pcb == NULL || pcb->state == 3){ // 3 = DEAD
            free(pid);
            continue;
        }
        //Sched_unblockProcess(pid); cambiar el nombre de la funcion
        free(pid);
        break;
    }

    if(isEmpty(semaphore->semaphores[sem].waitingProcesses)) {
        semaphore->semaphores[sem].value++;
    }

    release(&semaphore->semaphores[sem].lock);
    return 0;
}

int8_t semInit(uint8_t sem, uint8_t value) {
    if (!isValidSemaphore(sem) || semaphore->semaphores[sem].used == USED) {
        return -1;
    }
    semaphore->semaphores[sem].value = value;
    semaphore->semaphores[sem].used = USED;
    return 0;
}

