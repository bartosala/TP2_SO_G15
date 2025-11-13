#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <stdint.h>

#define NAME_MAX_LENGTH 32

typedef struct memInfo {
    uint64_t total;
    uint64_t used;
    uint64_t free;
} memInfo;


typedef enum {
    BLOCKED,
    READY,
    RUNNING,
    ZOMBIE,
    DEAD
} State;

// Funcion que el proceso ejecuta al iniciarse
typedef uint64_t (*processFun)(uint64_t argc, char **argv);
typedef int (*EntryPoint)();

typedef struct ProcessInfo {
    char* name;
    uint16_t pid;
    uint8_t priority;
    char foreground;
    uint64_t stack;
    State state;
} ProcessInfo;

#endif 