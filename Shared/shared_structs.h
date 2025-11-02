#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <stdint.h>

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
    EXITED,
    KILLED,
    SEM_WAITING
} State;

typedef size_t pid_t;

// Funcion que el proceso ejecuta al iniciarse
typedef uint64_t (*processFun)(uint64_t argc, char **argv);


typedef struct {
    pid_t pid;
    pid_t parentPid;
    pid_t waitingForPid;
    int8_t priority;
    State state;
    uint64_t rsp;
    uint64_t base;
    uint64_t entryPoint;
    uint64_t retValue;
    char foreground;
    //fds
    int stdin;
    int stdout;
    char name[NAME_MAX_LENGTH];
} PCB;

#endif 