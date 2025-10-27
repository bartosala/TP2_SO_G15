#include <time.h>
#include <stdint.h>
#include <clock.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <textModule.h>
#include <lib.h>
#include <soundDriver.h>
#include <stdarg.h>
#include "memoryManager.h"
#include <defs.h>


#define CANT_REGS 19
#define CANT_SYSCALLS 32

extern uint64_t regs[CANT_REGS];

typedef struct Point2D {
    uint64_t x, y;
} Point2D;
typedef uint64_t (*syscall_fn)(uint64_t rbx, uint64_t rcx, uint64_t rdx);


static uint64_t syscall_write(uint64_t fd, char *buff, uint64_t length) {
    if (length < 0) return 1;
    if (fd > 2 || fd < 0) return 2;
    uint64_t color = (fd == 1 ? 0x00FFFFFF : (fd == 2 ? 0x00FF0000 : 0));
    if(!color) return 0;
    for(int i = 0; i < length; i++)
        putChar(buff[i],color); 
    return length;  
}

static uint64_t syscall_beep(uint64_t freq, uint64_t ticks) {
    play_sound(freq);
    wait_ticks(ticks);
    nosound();
    return 0;
}

static uint64_t syscall_drawRectangle(Point2D* upLeft, Point2D *bottomRight, uint32_t color) {
    return drawRectangle(upLeft->x, upLeft->y, bottomRight->y - upLeft->y + 1, bottomRight->x - upLeft->x + 1, color);
}


static void syscall_getRegisters(uint64_t buff[]) {
    memcpy((void*)buff,(const void *)regs,CANT_REGS*(sizeof(void*)));
}

static uint64_t syscall_clearScreen(){
    clearText(0);
    return 0;
}

static uint64_t syscall_read( char* str,  uint64_t length){
    for(int i = 0; i < length && length > 0; i++){
        str[i] = getChar();
    }
    return length > 0 ? length : 0;
}

static uint64_t syscall_time(uint64_t mod){
    return getTimeParam(mod);
}

static uint64_t syscall_fontSizeUp(uint64_t increase){
    return fontSizeUp(increase);
}

static uint64_t syscall_fontSizeDown(uint64_t decrease){
    return fontSizeDown(decrease);
}

static uint64_t syscall_getWidth(){
    return getWidth();
}

static uint64_t syscall_getHeight(){
    return getHeight();
}

static uint64_t syscall_wait(uint64_t ticks){
    wait_ticks(ticks);
    return ticks;
}


// process/syscall wrappers will be provided below via scheduler API

static uint64_t syscall_allocMemory(uint64_t size) {
    return (uint64_t)allocMemory(size);
}

static uint64_t syscall_freeMemory(uint64_t address) {
    freeMemory((void*)address);
    return 0;
}

#include "../include/scheduler.h"

static uint64_t syscall_create_process(char *name, uint64_t argc, char *argv[]){
    // we don't have a direct entryPoint resolver by name yet; pass NULL
    return (uint64_t)Sched_createProcess((Code)0, name, argc, argv, 0);
}

static uint64_t syscall_exit(uint64_t code){
    // mark current process as zombie via scheduler and return
    // we don't have an immediate exit wrapper, scheduler will free on next tick
    // For now, set exit code and mark state
    SchedulerADT s = getScheduler();
    if (s && s->current) {
        s->current->exitCode = (uint16_t)code;
        s->current->state = 4; // ZOMBIE
    }
    return 0;
}

static uint64_t syscall_getpid(){
    return Sched_getPid();
}

static uint64_t syscall_kill(uint64_t pid){
    return (uint64_t)Sched_killProcess((uint16_t)pid);
}

static uint64_t syscall_block(uint64_t pid){
    return (uint64_t)Sched_blockProcess((uint16_t)pid);
}

static uint64_t syscall_unblock(uint64_t pid){
    return (uint64_t)Sched_unblockProcess((uint16_t)pid);
}

static uint64_t syscall_yield(uint64_t a, uint64_t b, uint64_t c){
    (void)a;(void)b;(void)c;
    return (uint64_t)Sched_yield();
}

static uint64_t syscall_wait_children(uint64_t a, uint64_t b, uint64_t c){
    (void)a;(void)b;(void)c;
    return (uint64_t)Sched_waitChildren();
}

static uint64_t syscall_set_priority(uint64_t pid, uint64_t newPrio, uint64_t c){
    (void)c;
    return (uint64_t)Sched_setPriority((uint16_t)pid, (uint8_t)newPrio);
}

static uint64_t syscall_list_processes(uint64_t a, uint64_t b, uint64_t c){
    (void)a;(void)b;(void)c;
    Sched_listProcesses();
    return 0;
}

// Prototipos de las funciones de syscall
uint64_t syscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3){
    if(syscall_number > CANT_SYSCALLS) return 0;
    syscall_fn syscalls[] = {0,
        (syscall_fn)syscall_read, (syscall_fn)syscall_write,
         (syscall_fn)syscall_time, (syscall_fn)syscall_beep,
          (syscall_fn)syscall_drawRectangle,
           (syscall_fn)syscall_getRegisters,
            (syscall_fn)syscall_clearScreen,
             (syscall_fn)syscall_fontSizeUp,
              (syscall_fn)syscall_fontSizeDown,
               (syscall_fn)syscall_getHeight,
                (syscall_fn)syscall_getWidth,
                 (syscall_fn)syscall_wait,
                  (syscall_fn)syscall_allocMemory,
                   (syscall_fn)syscall_freeMemory
                    ,(syscall_fn)syscall_create_process,
                    (syscall_fn)syscall_exit,
                    (syscall_fn)syscall_getpid,
                    (syscall_fn)syscall_kill,
                    (syscall_fn)syscall_block,
                    (syscall_fn)syscall_unblock,
                    (syscall_fn)syscall_yield,
                    (syscall_fn)syscall_wait_children,
                    (syscall_fn)syscall_set_priority,
                    (syscall_fn)syscall_list_processes
                };
    return syscalls[syscall_number](arg1, arg2, arg3);
}