#include <pipe.h>
#include <memoryManager.h>
#include <stddef.h>
#include <string.h>
#include <scheduler.h>
#include <process.h>

typedef struct PipeManagerCDT {
	pipe_t pipes[MAX_PIPES];
	uint8_t pipeCount;
} PipeManagerCDT;

static PipeManagerADT pipeManager = NULL;

PipeManagerADT createPipeManager() {
	pipeManager = (PipeManagerADT)memAlloc(sizeof(PipeManagerCDT));
	if (pipeManager != NULL) {
		return NULL;
	}

	pipeManager->pipeCount = 0;

	for(int i = 0; i < MAX_PIPES; i++) {
		pipeManager->pipes[i] = pipeCreate();
	}

	return pipeManager;
}

pipe_t pipeCreate() {
	pipe_t newPipe;

	newPipe.readIndex = 0;
	newPipe.writeIndex = 0;
	newPipe.size = 0;
	newPipe.fd = -1;
	newPipe.inputPID = -1;
	newPipe.outputPID = -1;
	newPipe.readLock = 0;
	newPipe.writeLock = 0;

	return newPipe;
}

uint8_t pipeOpen(uint16_t pid, uint8_t mode) {
	
	if(pipeManager == NULL || pipeManager->pipeCount >= MAX_PIPES) {
		return -1;
	}

	int8_t index = 0;
	while(index < MAX_PIPES) {
		if(pipeManager->pipes[index].fd != -1) {
			if(pipeManager->pipes[index].inputPID == -1){
				pipeManager->pipes[index].inputPID = pid;
				return pipeManager->pipes[index].fd;
			} else if(mode == PIPE_WRITE && pipeManager->pipes[index].outputPID == -1) {
				pipeManager->pipes[index].outputPID = pid;
				return pipeManager->pipes[index].fd;
			}
		}
	}
	
	index = -1;

	for(int i = 0; index < MAX_PIPES; index ++, i++){
		if(pipeManager->pipes[i].fd == -1){
			index = i;
			break;
		}
	}

	if(index == -1) {
		return -1;
	}

	pipe_t newPipe = pipeCreate();
	newPipe.fd = index + 3;
	if(mode == PIPE_READ) {
		newPipe.inputPID = pid;
	} else {
		return -1;
	}

	pipeManager->pipes[index] = newPipe;
	pipeManager->pipeCount++;
	return pipeManager->pipes[index].fd;
}

uint8_t closePipe(uint8_t fd){
	if(pipeManager == NULL || pipeManager->pipes[fd - 3].fd == -1){
		return 0;
	}

	pipeManager->pipes[fd - 3].fd = -1;
	pipeManager->pipes[fd - 3].inputPID = -1;
	pipeManager->pipes[fd - 3].outputPID = -1;
	pipeManager->pipeCount--;
	return 1;
}

uint8_t writePipe(uint8_t fd, char *buffer, uint8_t size){
	if(pipeManager == NULL || fd-3 >= MAX_PIPES || size > PIPE_BUFFER_SIZE || pipeManager->pipes[fd - 3].writeLock  
	|| pipeManager->pipes[fd - 3].size == PIPE_BUFFER_SIZE || pipeManager->pipes[fd - 3].fd == -1 || pipeManager->pipes[fd - 3].outputPID == -1){
		return 0;
	}

	pipe_t *pipe = &pipeManager->pipes[fd - 3];
	uint8_t written = 0;
	for(int i = 0; i < size; i++){
		while(pipe->size == PIPE_BUFFER_SIZE){
			pipe->writeLock = 1;
			blockProcess(pipe->outputPID);
			yield();
			if(pipe->fd == -1 || pipe->outputPID == -1){
				return written;
			}
		}
		pipe->buffer[pipe->writeIndex] = buffer[i];
		pipe->writeIndex = (pipe->writeIndex + 1) % PIPE_BUFFER_SIZE;
		pipe->size++;
		written++;
		
		if(pipe->readLock){
			pipe->readLock = 0;
			unblockProcess(pipe->inputPID);
		}
	}
	pipe->writeLock = 0;
	return written;
}

uint8_t pipeRead(uint8_t pipe_fd, char *buf, uint8_t size){
	if( pipeManager == NULL || pipe_fd-3 >= MAX_PIPES || size > PIPE_BUFFER_SIZE){
		return 0;
	}

	if(pipeManager->pipes[pipe_fd - 3].fd == -1 || pipeManager->pipes[pipe_fd - 3].inputPID == -1){
		return 0;
	}

	if(pipeManager->pipes[pipe_fd - 3].size == 0 && pipeManager->pipes[pipe_fd - 3].outputPID == -1){
		return -1;
	}

	while(pipeManager->pipes[pipe_fd - 3].size == 0 && pipeManager->pipes[pipe_fd - 3].outputPID != -1){
		pipeManager->pipes[pipe_fd - 3].readLock = 1;
		blockProcess(pipeManager->pipes[pipe_fd - 3].inputPID);
		yield();
		if(pipeManager->pipes[pipe_fd - 3].size == 0 && pipeManager->pipes[pipe_fd - 3].outputPID == -1){
			return -1;
		}
	}

	uint8_t read = size < pipeManager->pipes[pipe_fd - 3].size ? size : pipeManager->pipes[pipe_fd - 3].size;
	for(int i = 0; i < read; i++){
		buf[i] = pipeManager->pipes[pipe_fd - 3].buffer[pipeManager->pipes[pipe_fd - 3].readIndex];
		pipeManager->pipes[pipe_fd - 3].readIndex = (pipeManager->pipes[pipe_fd - 3].readIndex + 1) % PIPE_BUFFER_SIZE;
		pipeManager->pipes[pipe_fd - 3].size--;

		if(pipeManager->pipes[pipe_fd - 3].writeLock){
			pipeManager->pipes[pipe_fd - 3].writeLock = 0;
			unblockProcess(pipeManager->pipes[pipe_fd - 3].outputPID);
		}
	}
	pipeManager->pipes[pipe_fd - 3].readLock = 0;
	return read;
}

int killAllPipeProcesses(){


}
