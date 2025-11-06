
#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include "../../../Shared/shared_structs.h"

#define NAME_MAX_LENGTH 32 
#define STDIN 0

typedef struct Point2D {
    uint64_t x, y;
} Point2D;

uint64_t syscall_read(uint64_t fd, char *buff, uint64_t len);
uint64_t syscall_write(uint64_t fd, char *buff, uint64_t len);
uint64_t syscall_time(uint64_t mod);
uint64_t syscall_drawRectangle(Point2D * ul, Point2D * br, uint32_t color);
uint64_t syscall_clearScreen();
uint64_t syscall_sizeUpFont(uint64_t increment);
uint64_t syscall_sizeDownFont(uint64_t decrement);
uint64_t syscall_getHeight();
uint64_t syscall_getWidth();
uint64_t syscall_wait(uint64_t ticks);
int syscall_exit();

// Memoria
void *syscall_allocMemory(uint64_t size);
int syscall_freeMemory(void *address);
int64_t syscall_memInfo(memInfo *info);

//Procesos
uint64_t syscall_create_process(char *name, processFun function, char *argv[], uint8_t priority, char foreground, int stdin, int stdout);
uint64_t syscall_getpid();
uint64_t syscall_kill(uint64_t pid);
uint64_t syscall_block(uint64_t pid);
uint64_t syscall_unblock(uint64_t pid);
int8_t syscall_changePrio(uint64_t pid, int8_t newPrio);
PCB *syscall_getProcessInfo(uint64_t *cantProcesses);
int syscall_yield();
pid_t syscall_waitpid(pid_t pid, int32_t* status);

// Semáforos
int syscall_sem_open(int sem_id, uint64_t initialValue);
int syscall_sem_wait(int sem_id);
int syscall_sem_post(int sem_id);    
int syscall_sem_close(int sem_id);

// Pipes
int syscall_open_pipe();
int syscall_close_pipe(int pipe_id);
int syscall_clear_pipe(int pipe_id);



#endif
