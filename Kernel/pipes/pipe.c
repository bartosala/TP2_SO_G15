#include <pipe.h>
#include <process.h>
#include <scheduler.h>
#include <stddef.h>

#define MAX_PIPES 32
#define INVALID_FD -1

typedef struct PipeManagerCDT {
    pipe_t pipes[MAX_PIPES];
    uint8_t usedPipes[MAX_PIPES];  // Bitmap for used pipes
} PipeManagerCDT;

typedef struct PipeManagerCDT* PipeManagerADT;

static PipeManagerADT pipeManager = NULL;

// Initialize pipe manager
static PipeManagerADT createPipeManager() {
    PipeManagerADT pipeManager = (PipeManagerADT)malloc(sizeof(PipeManagerCDT));
    if (pipeManager == NULL) return NULL;
    
    for (int i = 0; i < MAX_PIPES; i++) {
        pipeManager->usedPipes[i] = 0;
        pipeManager->pipes[i].isOpen = 0;
        pipeManager->pipes[i].readerPID = -1;
        pipeManager->pipes[i].writerPID = -1;
        pipeManager->pipes[i].count = 0;
        pipeManager->pipes[i].readIndex = 0;
        pipeManager->pipes[i].writeIndex = 0;
        // Initialize semaphores
        sem_init(&pipeManager->pipes[i].readSem, 0);
        sem_init(&pipeManager->pipes[i].writeSem, PIPE_BUFFER_SIZE);
        sem_init(&pipeManager->pipes[i].mutex, 1);
    }
    return pipeManager;
}

// Get or create pipe manager instance
static PipeManagerADT getPipeManager() {
    if (pipeManager == NULL) {
        pipeManager = createPipeManager();
    }
    return pipeManager;
}

// Find free pipe slot
static int findFreePipeSlot(PipeManagerADT manager) {
    if (manager == NULL) return INVALID_FD;
    
    for (int i = 0; i < MAX_PIPES; i++) {
        if (!manager->usedPipes[i]) {
            return i;
        }
    }
    return INVALID_FD;
}

// Find pipe by ID
static pipe_t* findPipeById(PipeManagerADT manager, uint32_t id) {
    if (manager == NULL) return NULL;
    
    for (int i = 0; i < MAX_PIPES; i++) {
        if (manager->usedPipes[i] && manager->pipes[i].id == id) {
            return &manager->pipes[i];
        }
    }
    return NULL;
}

int pipeCreate(uint32_t id) {
    PipeManagerADT manager = getPipeManager();
    if (manager == NULL) return INVALID_FD;
    
    // Check if pipe with this ID already exists
    if (findPipeById(manager, id) != NULL) {
        return INVALID_FD;
    }
    
    int pipeIndex = findFreePipeSlot(manager);
    if (pipeIndex == INVALID_FD) {
        return INVALID_FD;
    }
    
    pipe_t* pipe = &manager->pipes[pipeIndex];
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

int pipeOpen(uint32_t id, int mode) {
    PipeManagerADT manager = getPipeManager();
    if (manager == NULL) return INVALID_FD;
    
    pipe_t* pipe = findPipeById(manager, id);
    if (pipe == NULL || !pipe->isOpen) {
        return INVALID_FD;
    }
    
    uint16_t currentPid = Sched_getPid();
    
    if (mode == PIPE_READ) {
        if (pipe->readerPID != -1) {
            return INVALID_FD;  // Pipe already has a reader
        }
        pipe->readerPID = currentPid;
    } else if (mode == PIPE_WRITE) {
        if (pipe->writerPID != -1) {
            return INVALID_FD;  // Pipe already has a writer
        }
        pipe->writerPID = currentPid;
    } else {
        return INVALID_FD;
    }
    
    return (pipe - manager->pipes);  // Return file descriptor
}

int pipeRead(int pipe_fd, void *buf, size_t count) {
    PipeManagerADT manager = getPipeManager();
    if (manager == NULL) return -1;
    
    if (pipe_fd < 0 || pipe_fd >= MAX_PIPES || !manager->usedPipes[pipe_fd]) {
        return -1;
    }
    
    pipe_t* pipe = &manager->pipes[pipe_fd];
    if (!pipe->isOpen || pipe->readerPID != Sched_getPid()) {
        return -1;
    }
    
    char* buffer = (char*)buf;
    size_t bytesRead = 0;
    
    while (bytesRead < count) {
        sem_wait(&pipe->readSem);  // Wait for data
        sem_wait(&pipe->mutex);
        
        if (pipe->count > 0) {
            buffer[bytesRead++] = pipe->buffer[pipe->readIndex];
            pipe->readIndex = (pipe->readIndex + 1) % PIPE_BUFFER_SIZE;
            pipe->count--;
            sem_post(&pipe->writeSem);  // Signal space available
        }
        
        sem_post(&pipe->mutex);
        
        // If writer closed and no more data
        if (pipe->writerPID == -1 && pipe->count == 0) {
            break;
        }
    }
    
    return bytesRead;
}

int pipeWrite(int pipe_fd, const void *buf, size_t count) {
    PipeManagerADT manager = getPipeManager();
    if (manager == NULL) return -1;
    
    if (pipe_fd < 0 || pipe_fd >= MAX_PIPES || !manager->usedPipes[pipe_fd]) {
        return -1;
    }
    
    pipe_t* pipe = &manager->pipes[pipe_fd];
    if (!pipe->isOpen || pipe->writerPID != Sched_getPid()) {
        return -1;
    }
    
    // Check if reader is still connected
    if (pipe->readerPID == -1) {
        return -1;  // No reader connected
    }
    
    const char* buffer = (const char*)buf;
    size_t bytesWritten = 0;
    
    while (bytesWritten < count) {
        sem_wait(&pipe->writeSem);  // Wait for space
        sem_wait(&pipe->mutex);
        
        if (pipe->count < PIPE_BUFFER_SIZE) {
            pipe->buffer[pipe->writeIndex] = buffer[bytesWritten++];
            pipe->writeIndex = (pipe->writeIndex + 1) % PIPE_BUFFER_SIZE;
            pipe->count++;
            sem_post(&pipe->readSem);  // Signal data available
        }
        
        sem_post(&pipe->mutex);
        
        // If reader disconnected
        if (pipe->readerPID == -1) {
            break;
        }
    }
    
    return bytesWritten;
}

int pipeClose(int pipe_fd) {
    PipeManagerADT manager = getPipeManager();
    if (manager == NULL) return -1;
    
    if (pipe_fd < 0 || pipe_fd >= MAX_PIPES || !manager->usedPipes[pipe_fd]) {
        return -1;
    }
    
    pipe_t* pipe = &manager->pipes[pipe_fd];
    uint16_t currentPid = Sched_getPid();
    
    if (currentPid == pipe->readerPID) {
        pipe->readerPID = -1;
    } else if (currentPid == pipe->writerPID) {
        pipe->writerPID = -1;
    } else {
        return -1;  // Not authorized to close
    }
    
    // If both ends are closed
    if (pipe->readerPID == -1 && pipe->writerPID == -1) {
        pipe->isOpen = 0;
        pipe->count = 0;
        pipe->readIndex = 0;
        pipe->writeIndex = 0;
        manager->usedPipes[pipe_fd] = 0;
    }
    
    return 0;
}
