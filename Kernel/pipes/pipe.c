#include <pipe.h>
#include <semaphore.h>
#include <stddef.h>

static pipeManager pipes;

static int next_sem_id = 0;
static int pipeManagerInit = 0;

/*
 * ensurePipeManagerInit
 * Ensures the global pipe manager is initialized exactly once.
 * Returns 0 always (initialization is idempotent).
 */
static int ensurePipeManagerInit() {
    if (!pipeManagerInit) {
        createPipeManager();
        pipeManagerInit = 1;
    }
    return 0;
}

/*
 * PIPE_ID_CHECK: validate a pipe id and map exported ids (>2) to internal
 * array indices. Returns -1 on invalid id or closed pipe.
 */
#define PIPE_ID_CHECK(id) \
    if(id < 0 || id >= MAX_PIPES) { \
        return -1; \
    } \
    if(id > 2) { \
        id -= 2;\
    } \
    if(!pipes.pipes[id].isOpen) { \
        return -1; \
    }

void createPipeManager() {
    /*
     * Initialize the manager: mark all pipes closed and clear indices/counters.
     */
    pipes.next_pipe_id = 0;
    for(int i = 0; i < MAX_PIPES; i++) {
        pipes.pipes[i].isOpen = 0;
        pipes.pipes[i].readIdx = 0;
        pipes.pipes[i].writeIdx = 0;
        pipes.pipes[i].count = 0;
        pipes.pipes[i].readers = 0;
        pipes.pipes[i].writers = 0;
        pipes.pipes[i].semReaders = -1;
        pipes.pipes[i].semWriters = -1;
        pipes.pipes[i].mutex = -1;
    }
}

int createPipe() {
    ensurePipeManagerInit();
    
    for(int i = 0; i < MAX_PIPES; i++) {
        if(!pipes.pipes[i].isOpen) {
            pipe_t *pipe = &pipes.pipes[i];
            
            pipe->readIdx = 0;
            pipe->writeIdx = 0;
            pipe->count = 0;
            pipe->readers = 0;
            pipe->writers = 0;
            pipe->isOpen = 1;
            
            pipe->semReaders = next_sem_id++;
            pipe->semWriters = next_sem_id++;
            pipe->mutex = next_sem_id++;
            
            if(semInit(pipe->semReaders, 0) < 0 ||           
               semInit(pipe->semWriters, PIPE_BUFFER_SIZE) < 0 || 
               semInit(pipe->mutex, 1) < 0) {                    
                return -1;
            }
            if(i == 0) 
                return 0;
            return i+2;
        }
    }
    return -1; 
}



int pipeRead(int pipe_id, char *buffer, int size) {
    ensurePipeManagerInit();
    PIPE_ID_CHECK(pipe_id)
    if(buffer == NULL || size <= 0)
        return -1;
    pipe_t *pipe = &pipes.pipes[pipe_id];
    int bytes_read = 0;
    /*
     * Read loop: for each requested byte, wait for a reader-count token
     * (blocks until data available), then grab the mutex, consume one
     * byte from the circular buffer and release the mutex/writer token.
     */
    for (int i = 0; i < size; i++) {
        semWait(pipe->semReaders);    /* wait until data available */
        semWait(pipe->mutex);         /* enter critical section */

        if (pipe->count > 0) {
            buffer[i] = pipe->buffer[pipe->readIdx];
            pipe->readIdx = (pipe->readIdx + 1) % PIPE_BUFFER_SIZE;
            pipe->count--;
            bytes_read++;
        }

        semPost(pipe->mutex);        /* leave critical section */
        semPost(pipe->semWriters);   /* signal writers there is space */
    }

    return bytes_read;
}

int pipeWrite(int pipe_id, const char *buffer, int size) {
    ensurePipeManagerInit();
    
    PIPE_ID_CHECK(pipe_id)
    if(buffer == NULL || size <= 0)
        return -1;
    pipe_t *pipe = &pipes.pipes[pipe_id];
    int bytes_written = 0;
    
    for(int i = 0; i < size; i++) {
        semWait(pipe->semWriters); 
        semWait(pipe->mutex);           
        
        if(pipe->count < PIPE_BUFFER_SIZE) {
            pipe->buffer[pipe->writeIdx] = buffer[i];
            pipe->writeIdx = (pipe->writeIdx + 1) % PIPE_BUFFER_SIZE;
            pipe->count++;
            bytes_written++;
        }

        semPost(pipe->mutex);
        semPost(pipe->semReaders);
    }

    return bytes_written;
}

int pipeClose(int pipe_id) {
    ensurePipeManagerInit();
    PIPE_ID_CHECK(pipe_id)
    if(pipe_id <= 2)
        return -1;
    
    pipe_t *pipe = &pipes.pipes[pipe_id];
    
    // Cerrar semáforos
    /* close semaphores used by this pipe */
    semClose(pipe->semReaders);
    semClose(pipe->semWriters);
    semClose(pipe->mutex);

    /* clear and mark as closed */
    pipe->isOpen = 0;
    pipe->readIdx = 0;
    pipe->writeIdx = 0;
    pipe->count = 0;
    pipe->readers = 0;
    pipe->writers = 0;
    pipe->semReaders = -1;
    pipe->semWriters = -1;
    pipe->mutex = -1;

    return 0;
}

int pipeClear(int pipe_id){
    ensurePipeManagerInit();
    PIPE_ID_CHECK(pipe_id)
    pipe_t *pipe = &pipes.pipes[pipe_id];
    /* clear contents of the circular buffer under mutex */
    semWait(pipe->mutex);

    pipe->readIdx = 0;
    pipe->writeIdx = 0;
    pipe->count = 0;

    semPost(pipe->mutex);

    return 0;
}