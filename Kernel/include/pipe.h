#ifndef PIPE_H
#define PIPE_H

#include <semaphore.h>

#define PIPE_BUFFER_SIZE 100
#define MAX_PIPES 16

typedef struct {
    char buffer[PIPE_BUFFER_SIZE];
    int readIdx;
    int writeIdx;
    int count;
    int semReaders;
    int semWriters; 
    int mutex;         
    int readers;
    int writers;
    int isOpen;
} pipe_t;

typedef struct {
    pipe_t pipes[MAX_PIPES];
    int next_pipe_id;
} pipeManager;

/* Public functions */

/*
 * createPipeManager
 * Initializes the global pipe manager and internal structures.
 * @return: none
 */
void createPipeManager();

/*
 * createPipe
 * Creates a new pipe and returns its id.
 * @return: pipe id (>=0) on success, -1 on failure
 */
int createPipe();

/*
 * pipeRead
 * Reads up to `size` bytes from pipe `pipe_id` into `buffer`.
 * This function may block depending on pipe semantics.
 * @param pipe_id: id of the pipe to read from
 * @param buffer: destination buffer (must be at least `size` bytes)
 * @param size: number of bytes requested to read
 * @return: number of bytes actually read, or -1 on error
 */
int pipeRead(int pipe_id, char *buffer, int size);

/*
 * pipeWrite
 * Writes up to `size` bytes from `buffer` into pipe `pipe_id`.
 * @param pipe_id: id of the pipe to write to
 * @param buffer: source buffer
 * @param size: number of bytes to write
 * @return: number of bytes actually written, or -1 on error
 */
int pipeWrite(int pipe_id, const char *buffer, int size);

/*
 * pipeClose
 * Closes the pipe with id `pipe_id` and frees associated resources.
 * @param pipe_id: id of the pipe to close
 * @return: 0 on success, -1 on failure
 */
int pipeClose(int pipe_id);

/*
 * pipeClear
 * Empties the pipe buffer without closing the pipe.
 * @param pipe_id: id of the pipe to clear
 * @return: 0 on success, -1 on failure
 */
int pipeClear(int pipe_id);

#endif