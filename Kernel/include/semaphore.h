#include <stdint.h>
#include <double_linked_list.h>
#include <scheduler.h>
#include <memoryManager.h>

#define NUM_SEMS 200

typedef struct sem_t {
    int8_t value;
    uint8_t lock;
    uint8_t used;
    DoubleLinkedListADT waitingProcesses;
} sem_t;

typedef struct SemaphoreCDT {
    sem_t semaphores[NUM_SEMS];
} SemaphoreCDT;

typedef struct SemaphoreCDT* SemaphoreADT;

extern void acquire(uint8_t *lock);
extern void release(uint8_t *lock);

SemaphoreADT createSemaphoresManager();
int8_t semOpen(uint8_t sem);
int8_t semClose(uint8_t sem);
int8_t semWait(uint8_t sem);
int8_t semPost(uint8_t sem);
int8_t semInit(uint8_t sem, uint8_t value);

