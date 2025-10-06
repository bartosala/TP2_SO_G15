#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

typedef struct Point2D {
    uint64_t x, y;
} Point2D;

uint64_t syscall_read(char *buff, uint64_t len);
uint64_t syscall_write(uint64_t fd, char *buff, uint64_t len);
uint64_t syscall_time(uint64_t mod);
uint64_t syscall_beep(uint64_t freq, uint64_t ticks);
uint64_t syscall_drawRectangle(Point2D * ul, Point2D * br, uint32_t color);
uint64_t syscall_getRegisters(uint64_t *registers);
uint64_t syscall_clearScreen();
uint64_t syscall_sizeUpFont(uint64_t increment);
uint64_t syscall_sizeDownFont(uint64_t decrement);
uint64_t syscall_getHeight();
uint64_t syscall_getWidth();
uint64_t syscall_wait(uint64_t ticks);

int64_t my_getpid();
int64_t my_create_process(char *name, uint64_t argc, char *argv[]);
int64_t my_nice(uint64_t pid, uint64_t newPrio);
int64_t my_kill(uint64_t pid);
int64_t my_block(uint64_t pid);
int64_t my_unblock(uint64_t pid);
int64_t my_sem_open(char *sem_id, uint64_t initialValue);
int64_t my_sem_wait(char *sem_id);
int64_t my_sem_post(char *sem_id);
int64_t my_sem_close(char *sem_id);
int64_t my_yield();
int64_t my_wait(int64_t pid);

#endif