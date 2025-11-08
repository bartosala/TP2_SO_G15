#include "test_util.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <syscall.h>
#include <testfunctions.h>

#define SEM_ID 10
#define TOTAL_PAIR_PROCESSES 2

int64_t global; // shared memory

void slowInc(int64_t *p, int64_t inc)
{
	uint64_t aux = *p;
	syscall_yield(); // This makes the race condition highly probable
	aux += inc;
	*p = aux;
}

uint64_t my_process_inc(uint64_t argc, char *argv[])
{
	uint64_t n;
	int8_t inc;
	int8_t use_sem;

	if (argc != 3) {
		syscall_exit();
		return -1;
	}

	if ((n = satoi(argv[0])) <= 0) {
		syscall_exit();
		return -1;
	}
	if ((inc = satoi(argv[1])) == 0) {
		syscall_exit();
		return -1;
	}
	if ((use_sem = satoi(argv[2])) < 0) {
		syscall_exit();
		return -1;
	}

	// DON'T open semaphore here - parent already opened it!

	uint64_t i;
	for (i = 0; i < n; i++) {
		if (use_sem)
			syscall_sem_wait(SEM_ID);
		slowInc(&global, inc);
		if (use_sem)
			syscall_sem_post(SEM_ID);
	}

	// DON'T close semaphore here - parent will close it!

	syscall_exit(); // ADD THIS - critical!
	return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[])
{ //{n, use_sem, 0}
	uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

	if (argc != 2) {
		printferror("test_sync: ERROR: expected 2 arguments, got %d\n", argc);
		syscall_exit();
		return -1;
	}

	int8_t use_sem = satoi(argv[1]);
	if (use_sem < 0) {
		syscall_exit();
		return -1;
	}

	// OPEN SEMAPHORE BEFORE CREATING CHILD PROCESSES
	if (use_sem) {
		if (syscall_sem_open(SEM_ID, 1) == -1) {
			printf("test_sync: ERROR opening semaphore\n");
			syscall_exit();
			return -1;
		}
	}

	char *argvDec[] = {argv[0], "-1", argv[1], NULL};
	char *argvInc[] = {argv[0], "1", argv[1], NULL};

	global = 0;

	uint64_t i;
	for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
		pids[i] = syscall_create_process("my_process_inc", (processFun)my_process_inc, argvDec, 1, 0, -1, 1);
		pids[i + TOTAL_PAIR_PROCESSES] =
		    syscall_create_process("my_process_inc", (processFun)my_process_inc, argvInc, 1, 0, -1, 1);
	}

	for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
		syscall_waitpid(pids[i], NULL);
		syscall_waitpid(pids[i + TOTAL_PAIR_PROCESSES], NULL);
	}

	// CLOSE SEMAPHORE AFTER ALL CHILDREN FINISH
	if (use_sem) {
		if (syscall_sem_close(SEM_ID) == -1) {
			printf("test_sync: ERROR closing semaphore\n");
			syscall_exit();
			return -1;
		}
	}

	printf("\nFinal value: %d\n", global);

	syscall_exit();
	return 0;
}