#include "../include/semaphore.h"
#include <stddef.h>
#include <memoryManager.h>
#include <queue.h>
#include <scheduler.h>

typedef struct SemaphoreCDT {
    sem_t semaphores[NUM_SEMS];
} SemaphoreCDT;

SemaphoreADT manager = NULL;

#define validateid(id) \
    do{ \
        if (id < 0 || id >= NUM_SEMS) { \
            return -1; \
        } \
    } while(0) 
    

SemaphoreADT createSemaphoresManager(){
    manager = allocMemory(sizeof(SemaphoreCDT));
    if (manager == NULL) {
        return NULL;
    }

    for (int i = 0; i < NUM_SEMS; i++) {
        manager->semaphores[i].value = 0;
        manager->semaphores[i].lock = 0;
        manager->semaphores[i].used = 0;
        manager->semaphores[i].blocked = NULL;
    }
    return manager;
}


int semInit (int id, uint32_t value) {
    validateid(id);

    if( ! manager->semaphores[id].used ) {
        manager->semaphores[id].value = value;
        manager->semaphores[id].lock = 0;
        manager->semaphores[id].used = 1;
        manager->semaphores[id].blocked = createQueue();

        if (manager->semaphores[id].blocked == NULL) {
            freeMemory(manager);
            return -1;
        }

        return 0;
    }
    return -1;
}

int semOpen (int id) {
    validateid(id);
    if (manager->semaphores[id].used) { 
        return 0;
    }
    return -1;
}

int semClose(int id) {
    validateid(id);
	if (! manager->semaphores[id].used) {
		return -1;
    }
	freeQueue(manager->semaphores[id].blocked);
    manager->semaphores[id].used = 0;
    manager->semaphores[id].value = 0;
    manager->semaphores[id].lock = 0;
    return 0;
}

int semWait (int id) {
    return wait(&manager->semaphores[id]);
}

int semPost (int id) {
    return post(&manager->semaphores[id]);
}

int wait (sem_t *sem){
    acquire(&sem->lock);
    if (sem->value > 0) {
        sem->value--;
        release(&sem->lock);
        return 0;
    }
    pid_t *pid = (pid_t *)allocMemory(sizeof(pid_t));
    if (pid == NULL) {
        release(&sem->lock);
        return -1; // Memory allocation failed
    }
    *pid = getCurrentPid();
    enqueue(sem->blocked, (void*)pid);
    release(&sem->lock);
    blockProcessBySem(*pid);
    return 0;
}

int post (sem_t *sem){
    acquire(&sem->lock);
    pid_t *pidPtr = (pid_t *)dequeue(sem->blocked);
    if (pidPtr != NULL) {
        pid_t pid = *pidPtr;
        freeMemory(pidPtr);  // Free the memory allocated in wait()
        unblockProcessBySem(pid);
        release(&sem->lock);
        return 0;   
    }
    sem->value++;
    release(&sem->lock);
    return 0;
}
