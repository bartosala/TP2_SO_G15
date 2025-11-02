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
#include <scheduler.h>
#include <textModule.h>


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
    return ticks; // ver si tiene que ser en segundos
}

static uint64_t syscall_toggle_cursor(uint64_t a, uint64_t b, uint64_t c){
    (void)a;(void)b;(void)c;
    // Placeholder for toggle cursor functionality
    return 0;
}


// process/syscall wrappers will be provided below via scheduler API

static uint64_t syscall_allocMemory(uint64_t size) {
    return (uint64_t)allocMemory(size);
}

static uint64_t syscall_freeMemory(uint64_t address) {
    freeMemory((void*)address);
    return 0;
}

static pid_t syscall_create_process(char* name, processFun function, int8_t priority, char** arg, char foreground, int stdin, int stdout){
    return createProcess(name, function, 0, arg, priority, foreground, stdin, stdout);
}

static uint64_t syscall_exit(uint64_t ret){
    char EOF = -1;
    syscall_write(1, &EOF, 1); // Escribir EOF en stdout
    kill(getCurrentPid(), ret);
    return 0;
}

pid_t syscall_getpid(){
    return getCurrentPid();
}

static uint64_t syscall_kill(pid_t pid){
    return kill(pid, 9); // Ver el valor de retorno
}

pid_t syscall_block(pid_t pid){
    return blockProcess(pid);
}

static uint64_t syscall_unblock(uint64_t pid){
    retunr unblockProcess(pid);
}

void syscall_yield(uint64_t a, uint64_t b, uint64_t c){
    yield() 
}

pid_t syscall_waitPid(pid_t pid, int32_t* retValue){
    return waitpid(pid, retValue);
}

static int8_t syscall_changePrio(uint64_t pid, int8_t newPrio){
    return changePrio(pid, newPrio);
}

static PCB * syscall_getProcessInfo(uint64_t *cantProcesses){
    return getProcessInfo(cantProcesses);
}

// SEMAFOROS 

int syscall_sem_wait(uint8_t sem_id){
    if(sem_id < 0) return -1;
    return semWait(sem_id);
}

int syscall_sem_post(uint8_t sem_id){
    if(sem_id < 0) return -1;
    return semPost(sem_id);
}

int syscall_sem_close(uint8_t sem_id){
    if(sem_id < 0) return -1;
    return semClose(sem_id);
}

int syscall_sem_open(uint8_t sem_id){
    if(sem_id < 0) return -1;
    return semOpen(sem_id);
}

// PIPES

int syscall_createPipe(int* pipe_id){
    return pipeCreate(pipe_id);
}

int syscall_closePipe(int pipe_id){
    return pipeClose(pipe_id);
}

int syscall_clearPipe(int pipe_id){
    return pipeClear(pipe_id);
}


// Prototipos de las funciones de syscall
uint64_t syscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3){
    if(syscall_number > CANT_SYSCALLS) return 0;
    syscall_fn syscalls[] = {0,
        (syscall_fn)syscall_read,            // 1
        (syscall_fn)syscall_write,           // 2
        (syscall_fn)syscall_time,            // 3
        (syscall_fn)syscall_beep,            // 4
        (syscall_fn)syscall_drawRectangle,   // 5
        (syscall_fn)syscall_getRegisters,    // 6
        (syscall_fn)syscall_clearScreen,     // 7
        (syscall_fn)syscall_fontSizeUp,      // 8
        (syscall_fn)syscall_fontSizeDown,    // 9
        (syscall_fn)syscall_getHeight,       // 10
        (syscall_fn)syscall_getWidth,        // 11
        (syscall_fn)syscall_wait,            // 12
        (syscall_fn)syscall_toggle_cursor,   // 13
        (syscall_fn)syscall_allocMemory,     // 14
        (syscall_fn)syscall_freeMemory,      // 15
        (syscall_fn)syscall_create_process,  // 16
        (syscall_fn)syscall_exit,            // 17
        (syscall_fn)syscall_getpid,          // 18
        (syscall_fn)syscall_kill,            // 19
        (syscall_fn)syscall_block,           // 20
        (syscall_fn)syscall_unblock,         // 21
        (syscall_fn)syscall_yield,           // 22
        (syscall_fn)syscall_changePrio,      // 23 (name in file: syscall_changePrio)
        (syscall_fn)syscall_waitPid,         // 24
        (syscall_fn)syscall_getProcessInfo,  // 25 (list/get process info)
        (syscall_fn)syscall_sem_wait,        // 26
        (syscall_fn)syscall_sem_post,        // 27
        (syscall_fn)syscall_sem_close,       // 28
        (syscall_fn)syscall_sem_open,        // 29
        (syscall_fn)syscall_createPipe,      // 30
        (syscall_fn)syscall_closePipe,       // 31
        (syscall_fn)syscall_clearPipe        // 32 
    };
    return syscalls[syscall_number](arg1, arg2, arg3);
}