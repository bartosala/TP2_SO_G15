#ifndef PIPE_H
#define PIPE_H

#include <stdint.h>
#include <semaphore.h>
#include <process.h>
#include <scheduler.h>

#define PIPE_BUFFER_SIZE 4096

typedef enum {
    PIPE_READ = 0,
    PIPE_WRITE = 1
} pipeEnum;

typedef struct {
    char buffer[PIPE_BUFFER_SIZE];
    uint32_t readIndex;
    uint32_t writeIndex;
    uint32_t count;           // Current bytes in buffer
    
    uint32_t id;             // Public identifier for pipe sharing between unrelated processes
    
    int8_t readerPID;        // Current reader process (-1 if none)
    int8_t writerPID;        // Current writer process (-1 if none)
    
    uint8_t isOpen;          // Pipe state (open/closed)
    uint8_t isFromTerminal;  // Flag to identify if pipe is connected to terminal
    
    sem_t readSem;          // Semaphore for blocking reads when empty
    sem_t writeSem;         // Semaphore for blocking writes when full
    sem_t mutex;            // Mutual exclusion for buffer access
} pipe_t;

typedef struct PipeManagerCDT* PipeManagerADT;

PipeManagerADT createPipeManager();
int pipeCreate(uint32_t id);               
int pipeOpen(uint32_t id, int mode);          
int pipeRead(int pipe_fd, void *buf, size_t count);
int pipeWrite(int pipe_fd, const void *buf, size_t count);
int pipeClose(int pipe_fd);
int pipeClear(int pipe_fd);

#endif // PIPE_H