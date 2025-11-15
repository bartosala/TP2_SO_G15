
#include <memoryManager.h>
#include <pipe.h>
#include <process.h>
#include <scheduler.h>
#include <stddef.h>

#define MAX_PIPES 32
#define INVALID_FD -1

typedef struct PipeManagerCDT {
	pipe_t pipes[MAX_PIPES];
	uint8_t usedPipes[MAX_PIPES];
} PipeManagerCDT;

typedef struct PipeManagerCDT *PipeManagerADT;

static PipeManagerADT pipeManager = NULL;

PipeManagerADT createPipeManager()
{
	pipeManager = (PipeManagerADT)allocMemory(sizeof(PipeManagerCDT));
	if (pipeManager == NULL)
		return NULL;

	for (int i = 0; i < MAX_PIPES; i++) {
		pipeManager->usedPipes[i] = 0;
		pipeManager->pipes[i].isOpen = 0;
		pipeManager->pipes[i].readerPID = -1;
		pipeManager->pipes[i].writerPID = -1;
		pipeManager->pipes[i].count = 0;
		pipeManager->pipes[i].readIndex = 0;
		pipeManager->pipes[i].writeIndex = 0;
		pipeManager->pipes[i].readSemId = i * 3;
		pipeManager->pipes[i].writeSemId = i * 3 + 1;
		pipeManager->pipes[i].mutexId = i * 3 + 2;

		semInit(pipeManager->pipes[i].readSemId, 0);
		semInit(pipeManager->pipes[i].writeSemId, PIPE_BUFFER_SIZE);
		semInit(pipeManager->pipes[i].mutexId, 1);
	}
	return pipeManager;
}

static PipeManagerADT getPipeManager()
{
	if (pipeManager == NULL) {
		pipeManager = createPipeManager();
	}
	return pipeManager;
}

static int findFreePipeSlot(PipeManagerADT manager)
{
	if (manager == NULL)
		return INVALID_FD;

	for (int i = 0; i < MAX_PIPES; i++) {
		if (!manager->usedPipes[i]) {
			return i;
		}
	}
	return INVALID_FD;
}

static pipe_t *findPipeById(PipeManagerADT manager, uint32_t id)
{
	if (manager == NULL)
		return NULL;

	for (int i = 0; i < MAX_PIPES; i++) {
		if (manager->usedPipes[i] && manager->pipes[i].id == id) {
			return &manager->pipes[i];
		}
	}
	return NULL;
}

int pipeCreate(uint32_t id)
{
	PipeManagerADT manager = getPipeManager();
	if (manager == NULL)
		return INVALID_FD;

	if (findPipeById(manager, id) != NULL) {
		return INVALID_FD;
	}

	int pipeIndex = findFreePipeSlot(manager);
	if (pipeIndex == INVALID_FD) {
		return INVALID_FD;
	}

	pipe_t *pipe = &manager->pipes[pipeIndex];
	pipe->id = id;
	pipe->isOpen = 1;
	pipe->readerPID = -1;
	pipe->writerPID = -1;
	pipe->count = 0;
	pipe->readIndex = 0;
	pipe->writeIndex = 0;
	pipe->isFromTerminal = 0;

	manager->usedPipes[pipeIndex] = 1;

	return pipeIndex;
}

int pipeOpen(uint32_t id, int mode)
{
	PipeManagerADT manager = getPipeManager();
	if (manager == NULL)
		return INVALID_FD;

	pipe_t *pipe = findPipeById(manager, id);
	if (pipe == NULL || !pipe->isOpen) {
		return INVALID_FD;
	}

	pid_t currentPid = getCurrentPid();

	if (mode == PIPE_READ) {
		if (pipe->readerPID != -1) {
			return INVALID_FD; // Pipe already has a reader
		}
		pipe->readerPID = currentPid;
	} else if (mode == PIPE_WRITE) {
		if (pipe->writerPID != -1) {
			return INVALID_FD;
		}
		pipe->writerPID = currentPid;
	} else {
		return INVALID_FD;
	}

	return (pipe - manager->pipes);
}

int pipeOpenForPid(uint32_t id, int mode, pid_t pid)
{
	PipeManagerADT manager = getPipeManager();
	if (manager == NULL)
		return INVALID_FD;

	pipe_t *pipe = findPipeById(manager, id);
	if (pipe == NULL || !pipe->isOpen) {
		return INVALID_FD;
	}

	if (mode == PIPE_READ) {
		if (pipe->readerPID != -1) {
			return INVALID_FD;
		}
		pipe->readerPID = pid;
	} else if (mode == PIPE_WRITE) {
		if (pipe->writerPID != -1) {
			return INVALID_FD;
		}
		pipe->writerPID = pid;
	} else {
		return INVALID_FD;
	}

	return (pipe - manager->pipes);
}

int pipeRead(int pipe_fd, void *buf, size_t count)
{
	PipeManagerADT manager = getPipeManager();
	if (manager == NULL)
		return -1;

	if (pipe_fd < 0 || pipe_fd >= MAX_PIPES || !manager->usedPipes[pipe_fd]) {
		return -1;
	}

	pipe_t *pipe = &manager->pipes[pipe_fd];
	if (!pipe->isOpen) {
		return -1;
	}

	char *buffer = (char *)buf;
	int bytesRead = 0;

	for (int i = 0; i < count; i++) {
		semWait(pipe->readSemId);
		semWait(pipe->mutexId);

		if (pipe->count > 0) {
			buffer[i] = pipe->buffer[pipe->readIndex];
			pipe->readIndex = (pipe->readIndex + 1) % PIPE_BUFFER_SIZE;
			pipe->count--;
			bytesRead++;
		}

		semPost(pipe->mutexId);
		semPost(pipe->writeSemId);
	}

	return bytesRead;
}

int pipeWrite(int pipe_fd, const void *buf, size_t count)
{
	PipeManagerADT manager = getPipeManager();
	if (manager == NULL)
		return -1;

	if (pipe_fd < 0 || pipe_fd >= MAX_PIPES || !manager->usedPipes[pipe_fd]) {
		return -1;
	}

	pipe_t *pipe = &manager->pipes[pipe_fd];
	if (!pipe->isOpen) {
		return -1;
	}

	if (pipe_fd != 0 && pipe->writerPID != getCurrentPid()) {
		return -1;
	}

	if (pipe->readerPID == -1) {
		return -1;
	}

	const char *buffer = (const char *)buf;
	int bytesWritten = 0;

	for (int i = 0; i < count; i++) {
		semWait(pipe->writeSemId);
		semWait(pipe->mutexId);

		if (pipe->count < PIPE_BUFFER_SIZE) {
			pipe->buffer[pipe->writeIndex] = buffer[i];
			pipe->writeIndex = (pipe->writeIndex + 1) % PIPE_BUFFER_SIZE;
			pipe->count++;
			bytesWritten++;
		}

		semPost(pipe->mutexId);
		semPost(pipe->readSemId);
	}

	return bytesWritten;
}

int pipeClose(int pipe_fd)
{
	PipeManagerADT manager = getPipeManager();
	if (manager == NULL)
		return -1;

	if (pipe_fd < 0 || pipe_fd >= MAX_PIPES || !manager->usedPipes[pipe_fd]) {
		return -1;
	}

	pipe_t *pipe = &manager->pipes[pipe_fd];
	pid_t currentPid = getCurrentPid();

	if (currentPid == pipe->readerPID) {
		pipe->readerPID = -1;
	} else if (currentPid == pipe->writerPID) {
		pipe->writerPID = -1;
	} else {
		return -1;
	}

	if (pipe->readerPID == -1 && pipe->writerPID == -1) {
		pipe->isOpen = 0;
		pipe->count = 0;
		pipe->readIndex = 0;
		pipe->writeIndex = 0;
		manager->usedPipes[pipe_fd] = 0;
	}

	return 0;
}

int pipeClear(int id)
{
	PipeManagerADT manager = getPipeManager();
	if (manager == NULL)
		return -1;

	pipe_t *pipe = findPipeById(manager, id);
	if (pipe == NULL || !pipe->isOpen) {
		return -1;
	}

	semWait(pipe->mutexId);
	pipe->count = 0;
	pipe->readIndex = 0;
	pipe->writeIndex = 0;
	semPost(pipe->mutexId);

	semInit(pipe->readSemId, 0);
	semInit(pipe->writeSemId, PIPE_BUFFER_SIZE);

	return 0;
}