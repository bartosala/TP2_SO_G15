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

#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct Point2D {
	uint64_t x, y;
} Point2D;

typedef uint64_t (*syscall_fn)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

static uint64_t syscall_write(uint32_t fd, char *buff, uint32_t length){
	if(fd == STDERR){
		return 0;
	}
	
	int16_t fdVal = fd < 3 ? getFileDescriptor(fd) : fd;
	if(fdVal < 0){
		return -1;
	}

	if(fdVal == STDOUT){
		for(int i = 0; i < length; i++){
			putChar(buff[i], 0x00FFFFFF);
		}
		return length;
	} else if(fdVal >= 3){
		int ret = pipeWrite(fdVal, buff, length);
		yield();
		return (ret < 0) ? -1 : ret;
	}
	return -1;
}

static uint64_t syscall_clearScreen(){
	clearText(0);
	return 0;
}

static uint64_t syscall_read(uint64_t fd, char *str, uint64_t length){
	int16_t fdVal = fd < 3 ? getFileDescriptor(fd) : fd;
	if(fdVal < 0){
		return -1;
	}

	if(fdVal == STDIN){
		uint64_t bytesRead = 0;
		while(bytesRead < length){
			char c = getChar();
			if(c == (char)EOF){
				break;
			}
			str[bytesRead++] = c;
		}
		return bytesRead;
	} else if(fdVal >= 3){
		int ret = pipeRead(fdVal, str, length);
		yield();
		return (ret < 0) ? -1 : ret;
	}
	return -1;
}


static uint64_t syscall_wait(uint64_t ticks){
	wait_ticks(ticks);
	return ticks;
}


static uint64_t syscall_freeMemory(uint64_t address){
	freeMemory((void *)address);
	return 0;
}

static inline uint64_t argCounter(char **argv){
	uint64_t c = 0;
	if (argv == NULL || *argv == NULL) {
		return 0;
	}
	while (argv[c] != NULL) {
		c++;
	}
	return c;
}

uint64_t syscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6)
{
	if (syscall_number > CANT_SYSCALLS)
		return 0;
	_cli();
	syscall_fn syscalls[] = {
	    0,
	    (syscall_fn)syscall_read,
	    (syscall_fn)syscall_write,
	    (syscall_fn)clearText,
	    (syscall_fn)fontSizeUp,
	    (syscall_fn)fontSizeDown,
	    (syscall_fn)getHeight,
	    (syscall_fn)getWidth,
	    (syscall_fn)syscall_wait,
	    (syscall_fn)allocMemory,
	    (syscall_fn)syscall_freeMemory,
		(syscall_fn)createProcess,
		(syscall_fn)getPid,
		(syscall_fn)killProcess,
		(syscall_fn)blockProcess,
		(syscall_fn)unblockProcess,
		(syscall_fn)changePriority,
		(syscall_fn)getProcessInfo,
		(syscall_fn)syscall_memInfo,
		(syscall_fn)semOpen,
		(syscall_fn)semWait,
		(syscall_fn)semPost,
		(syscall_fn)semClose,
		(syscall_fn)yield,
		(syscall_fn)pipeOpen,
		(syscall_fn)pipeClose,
		(syscall_fn)pipeWrite,
		(syscall_fn)pipeRead,
		(syscall_fn)setStatus,
		(syscall_fn)changeFd,
		(syscall_fn)wait_ticks
	};
	uint64_t ret = syscalls[syscall_number](arg1, arg2, arg3, arg4, arg5, arg6);
	_sti();
	return ret;
}