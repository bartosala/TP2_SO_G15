#include "memoryManager.h"
#include <clock.h>
#include <defs.h>
#include <keyboardDriver.h>
#include <lib.h>
#include <pipe.h>
#include <scheduler.h>
#include <semaphore.h>
#include <soundDriver.h>
#include <stdarg.h>
#include <stdint.h>
#include <textModule.h>
#include <time.h>
#include <videoDriver.h>

#define CANT_REGS 19
#define CANT_SYSCALLS 29
#define MAX_PIPES 16
extern uint64_t regs[CANT_REGS];

typedef struct Point2D {
	uint64_t x, y;
} Point2D;
typedef uint64_t (*syscall_fn)(uint64_t rbx, uint64_t rcx, uint64_t rdx);


static uint64_t syscall_write(uint64_t fd, char *buff, uint64_t length)
{
	if (fd == 1) {
		fd = getCurrentStdout();
	} else if (fd == 0) {
		fd = getCurrentStdin();
	}
	uint64_t color = fd == 1 ? 0x00FFFFFF : 0x00FF0000;
	switch (fd) {
	case 0:
		return pipeWrite(0, buff, length);
		break;
	case 1:
	case 2:
		for (int i = 0; i < length && buff[i] != -1; i++)
			putChar(buff[i], color);
		return length;
		break;
	default:
		int64_t i;
		if (fd >= 3 + MAX_PIPES || (i = pipeWrite(fd, buff, length)) < 0) {
			return -1;
		}
		return i;
		break;
	}
	return -1;
}

static uint64_t syscall_clearScreen()
{
	clearText(0);
	return 0;
}

static uint64_t syscall_read(uint64_t fd, char *str, uint64_t length)
{

	if (fd >= 3 + MAX_PIPES) {
		return -1;
	}
	if (fd == 0) {
		fd = getCurrentStdin();
		if (fd == -1) {
			return -1;
		}
	}
	return pipeRead(fd, str, length);
}

static uint64_t syscall_fontSizeUp(uint64_t increase)
{
	return fontSizeUp(increase);
}

static uint64_t syscall_fontSizeDown(uint64_t decrease)
{
	return fontSizeDown(decrease);
}

static uint64_t syscall_getWidth()
{
	return getWidth();
}

static uint64_t syscall_getHeight()
{
	return getHeight();
}

static uint64_t syscall_wait(uint64_t ticks)
{
	wait_ticks(ticks);
	return ticks;
}

static uint64_t syscall_allocMemory(uint64_t size)
{
	return (uint64_t)allocMemory(size);
}

static uint64_t syscall_freeMemory(uint64_t address)
{
	freeMemory((void *)address);
	return 0;
}

static inline uint64_t argCounter(char **argv)
{
	uint64_t c = 0;
	if (argv == NULL || *argv == NULL) {
		return 0;
	}
	while (argv[c] != NULL) {
		c++;
	}
	return c;
}

pid_t syscall_create_process(ProcessParams *p)
{
	return createProcess(p->name, (processFun)p->function, argCounter(p->arg), p->arg, p->priority, p->foreground,
	                     p->stdin, p->stdout);
}

static uint64_t syscall_exit(uint64_t ret)
{
	char char_eof = (char)0xFF;
	syscall_write(1, &char_eof, 1);
	kill(getCurrentPid(), ret);
	return 0;
}

pid_t syscall_getpid()
{
	return getCurrentPid();
}

static uint64_t syscall_kill(pid_t pid)
{
	return kill(pid, 9);
}

pid_t syscall_block(pid_t pid)
{
	return blockProcess(pid);
}

static uint64_t syscall_unblock(uint64_t pid)
{
	return unblockProcess(pid);
}

void syscall_yield(uint64_t a, uint64_t b, uint64_t c)
{
	yield();
}

static int8_t syscall_changePrio(uint64_t pid, int8_t newPrio)
{
	return changePrio(pid, newPrio);
}

static PCB *syscall_getProcessInfo(uint64_t *cantProcesses)
{
	return getProcessInfo(cantProcesses);
}

static int64_t syscall_memInfo(memInfo *user_ptr)
{
	if (user_ptr == NULL) {
		return -1;
	}
	memInfo temp;
	getMemoryInfo(&temp);
	user_ptr->total = temp.total;
	user_ptr->used = temp.used;
	user_ptr->free = temp.free;

	return 0;
}

int syscall_sem_wait(uint8_t sem_id)
{
	if (sem_id < 0)
		return -1;
	return semWait(sem_id);
}

int syscall_sem_post(uint8_t sem_id)
{
	if (sem_id < 0)
		return -1;
	return semPost(sem_id);
}

int syscall_sem_close(uint8_t sem_id)
{
	if (sem_id < 0)
		return -1;
	return semClose(sem_id);
}

int syscall_sem_open(uint8_t sem_id)
{
	if (sem_id < 0)
		return -1;
	return semOpen(sem_id);
}

int syscall_openPipe()
{
	return createPipe();
}

int syscall_closePipe(int pipe_id)
{
	return pipeClose(pipe_id);
}

int syscall_clearPipe(int pipe_id)
{
	return pipeClear(pipe_id);
}

pid_t syscall_waitPid(pid_t pid, int32_t *retValue)
{
	return waitpid(pid, retValue);
}

uint64_t syscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
	if (syscall_number > CANT_SYSCALLS)
		return 0;
	_cli();
	syscall_fn syscalls[] = {
	    0,
	    (syscall_fn)syscall_read,
	    (syscall_fn)syscall_write,
	    (syscall_fn)syscall_clearScreen,
	    (syscall_fn)syscall_fontSizeUp,
	    (syscall_fn)syscall_fontSizeDown,
	    (syscall_fn)syscall_getHeight,
	    (syscall_fn)syscall_getWidth,
	    (syscall_fn)syscall_wait,
	    (syscall_fn)syscall_allocMemory,
	    (syscall_fn)syscall_freeMemory,
	    (syscall_fn)syscall_create_process,
	    (syscall_fn)syscall_getpid,
	    (syscall_fn)syscall_kill,
	    (syscall_fn)syscall_block,
	    (syscall_fn)syscall_unblock,
	    (syscall_fn)syscall_changePrio,
	    (syscall_fn)syscall_getProcessInfo,
	    (syscall_fn)syscall_memInfo,
	    (syscall_fn)syscall_exit,
	    (syscall_fn)syscall_sem_open,
	    (syscall_fn)syscall_sem_wait,
	    (syscall_fn)syscall_sem_post,
	    (syscall_fn)syscall_sem_close,
	    (syscall_fn)syscall_yield,
	    (syscall_fn)syscall_openPipe,
	    (syscall_fn)syscall_closePipe,
	    (syscall_fn)syscall_clearPipe,
	    (syscall_fn)syscall_waitPid,
	};
	uint64_t ret = syscalls[syscall_number](arg1, arg2, arg3);
	_sti();
	return ret;
}