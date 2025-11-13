#ifndef PIPE_H
#define PIPE_H

#include <process.h>
#include <scheduler.h>
#include <semaphore.h>
#include <stdint.h>

#define PIPE_BUFFER_SIZE 512
#define MAX_PIPES 50

typedef enum { PIPE_READ = 0, PIPE_WRITE = 1 } pipeEnum;

typedef struct {
	char buffer[PIPE_BUFFER_SIZE];
	uint8_t readIndex;
	uint8_t writeIndex;
	uint8_t size;
	int8_t fd;
	int8_t inputPID, outputPID;
	uint8_t readLock, writeLock;
} pipe_t;

typedef struct PipeManagerCDT *PipeManagerADT;

PipeManagerADT createPipeManager();
pipe_t pipeCreate();
uint8_t pipeOpen(uint16_t pid, uint8_t mode);
uint8_t pipeRead(uint8_t pipe_fd, char *buf, uint8_t size);
uint8_t pipeWrite(uint8_t pipe_fd, char *buf, uint8_t size);
uint8_t pipeClose(uint8_t pipe_fd);
int killAllPipeProcesses();
void closePipeByPID(uint16_t pid);

#endif // PIPE_H
