#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <double_linked_list.h>

typedef int (*Code)(int argc, char **args); // Puntero a la función del programa

typedef struct Process { // PCB
	uint16_t pid;             
	uint16_t parentPid;
	Code program;                
	char **args;                 
	int argc;
	char *name;                  
	uint8_t priority;         
	int16_t fileDescriptors[15]; 


	void *stackBase; // MemoryBlock
	void *stackPos;
    
	// children processes (double linked list using the ADT)
	DoubleLinkedListADT children;

	// state
	uint8_t state; // 0 = BLOCKED, 1 = READY, 2 = RUNNING, 3 = DEAD, 4 = ZOMBIE
	uint8_t foreground; // boolean: foreground/background
	uint16_t exitCode;
	uint16_t childrenCount;

} Process;

void initProcess(Process *process, uint16_t pid, uint16_t parentPid, Code program, char **args, char *name, uint8_t priority, int16_t fileDescriptors[]);
void freeProcess(Process *process);
void closeFileDescriptors(Process *process);

// Process wrapper called as entry point for new processes. Signature: argc, argv, original entry
void processWrapper(int argc, char **argv, Code entry);

// Allocate and copy argv into kernel-managed memory
char **allocArgv(Process *p, char **argv, int argc);


#endif