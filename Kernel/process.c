#include <memoryManager.h>
#include <process.h>
#include <stdlib.h> // free

void initProcess(Process *process, uint16_t pid, uint16_t parentPid, Code program, char **args, char *name, uint8_t priority, int16_t fileDescriptors[]) {
    process->pid = pid;
    process->parentPid = parentPid;
    process->program = program;
    process->args = args;
    process->name = name;
    process->priority = priority;

    process->stackBase = allocMemory(NULL, SIZE);
	process->args = allocArguments(args);  
	process->name = allocMemory(strlen(name) + 1);
    
}

void freeProcess(Process *process) {
   
}

void closeFileDescriptors(Process *process) {

}

