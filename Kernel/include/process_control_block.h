#ifndef PROCESS_CONTROL_BLOCK_H
#define PROCESS_CONTROL_BLOCK_H

#include <stdint.h>

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    // TERMINATED
} State;

typedef struct Registers {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip; // Instruction Pointer
    uint64_t cs;  // Code Segment
    uint64_t rflags; // Flags Register
    uint64_t rsp; // Stack Pointer
    uint64_t ss;  // Stack Segment    
};

typedef struct Registers * Registers;

typedef int pid_t;

typedef struct PCB {
    pid_t pid; 
    pid_t parentPid;
    int priority; 
    State state; 
    Registers registers; // Guardar el estado de los registros
    void *stackBase; // Base de la pila
    void *stackPointer; // Puntero actual de la pila
} PCB;
#endif // PROCESS_CONTROL_BLOCK_H