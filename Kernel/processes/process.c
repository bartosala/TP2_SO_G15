#include "../include/memoryManager.h"
#include "../include/process.h"
#include "../include/scheduler.h"
#include "../include/interrupts.h"
#include "../include/defs.h"
#include <lib.h>
#include <stddef.h>
#include <string.h>

void initProcess(Process *process, uint16_t pid, uint16_t parentPid, Code program, char **args, char *name, uint8_t priority, int16_t fileDescriptors[]) {
    if (!process) return;
    process->pid = pid;
    process->parentPid = parentPid;
    process->program = program;
    process->args = args;
    process->name = name;
    process->priority = priority;
    if (fileDescriptors) memcpy(process->fileDescriptors, fileDescriptors, sizeof(process->fileDescriptors));
    else for (int i = 0; i < 15; i++) process->fileDescriptors[i] = -1;

    process->stackBase = NULL;
    process->stackPos = NULL;
    process->children = NULL;
    // create children list on demand; create now for convenience
    process->children = createDoubleLinkedList();
    process->state = 0; // NEW
    process->foreground = 0;
    process->exitCode = 0;
    process->childrenCount = 0;
}

void closeFileDescriptors(Process *process) {
    if (!process) return;
    for (int i = 0; i < 15; i++) process->fileDescriptors[i] = -1;
}

void freeProcess(Process *process) {
    if (!process) return;
    if (process->stackBase) freeMemory(process->stackBase);
    if (process->children) freeList(process->children);
    freeMemory(process);
}

// Allocate and copy argv strings into kernel-managed memory for a process
char **allocArgv(Process *p, char **argv, int argc) {
    if (!p) return NULL;
    if (argc <= 0) return NULL;
    char **newargv = (char**)allocMemory(sizeof(char*) * (argc + 1));
    if (!newargv) return NULL;
    for (int i = 0; i < argc; i++) {
        if (argv[i]) {
            size_t len = strlen(argv[i]) + 1;
            char *s = (char*)allocMemory(len);
            if (!s) {
                // cleanup
                for (int j = 0; j < i; j++) if (newargv[j]) freeMemory(newargv[j]);
                freeMemory(newargv);
                return NULL;
            }
            memcpy(s, argv[i], len);
            newargv[i] = s;
        } else newargv[i] = NULL;
    }
    newargv[argc] = NULL;
    p->args = newargv;
    p->argc = argc;
    return newargv;
}

// process wrapper: called when a new process starts. Expects registers set so rdi=argc, rsi=argv, rdx=entryPoint
void processWrapper(int argc, char **argv, Code entry) {
    if (entry) entry(argc, argv);
    // when returns, free and exit
    // attempt to get scheduler and mark exit; scheduler will clean on next tick
    extern SchedulerADT getScheduler();
    SchedulerADT s = getScheduler();
    if (s && s->current) {
        s->current->state = 4; // ZOMBIE
    }
    while (1) { _hlt(); }
}

