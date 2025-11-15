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
    int semReaders;  // ID del semáforo para lectores
    int semWriters; // ID del semáforo para escritores
    int mutex;           // ID del mutex para acceso exclusivo
    int readers;
    int writers;
    int isOpen;
} pipe_t;

typedef struct {
    pipe_t pipes[MAX_PIPES];
    int next_pipe_id;
} pipeManager;

// Funciones públicas
void createPipeManager();
int createPipe();
int pipeRead(int pipe_id, char *buffer, int size);
int pipeWrite(int pipe_id, const char *buffer, int size);
int pipeClose(int pipe_id);
int pipeClear(int pipe_id);

#endif