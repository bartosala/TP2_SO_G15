#ifndef PROCESS_H
#define PROCESS_H

#include "../../Shared/shared_structs.h"
#include <doubleLinkedList.h>
#include <stdbool.h>
#include <stdint.h>

#define FILE_DESCRPIPTORS 3

typedef struct PCB{
    char* name;
    char** argv;
    int argc;
    uint16_t foreground;
    uint8_t priority;
    uint64_t stack;
    uint64_t basePointer;
    int16_t fds[FILE_DESCRPIPTORS];
    uint16_t pid;
    uint16_t ppid;
    EntryPoint rip;
    State state;
    uint8_t quantum;
    DoubleLinkedListADT childList;
    int8_t childSem;
    void* rsp;
} PCB;

void freeProcess(PCB* process);

uint16_t waitForChildern();
uint64_t setStackFrame(uint64_t rsp, uint64_t code, int argc, char* args[], EntryPoint entryPoint);
char** allocArgv(PCB *p, char **argv, int argc);
void processWrapper(void (*entryPoint)(int, char**), int argc, char** argv);
void idleProcess();

#endif // PROCESS_H