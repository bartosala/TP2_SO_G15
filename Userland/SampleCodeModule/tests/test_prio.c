// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "syscall.h"
#include "test_util.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define TOTAL_PROCESSES 3

#define LOWEST 0
#define MEDIUM 1
#define HIGHEST 2

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

uint64_t max_value = 0;

void zero_to_max()
{
	uint64_t value = 0;

	while (value++ != max_value)
		;

	printf("PROCESS %d DONE!\n", syscall_getpid());
}

uint64_t test_prio(uint64_t argc, char *argv[])
{
	int64_t pids[TOTAL_PROCESSES];
	char *ztm_argv[] = {0};
	uint64_t i;

	if (argc != 1)
		return -1;

	if ((max_value = satoi(argv[0])) <= 0)
		return -1;

	printf("SAME PRIORITY...\n");

	for (i = 0; i < TOTAL_PROCESSES; i++)
		pids[i] = syscall_create_process("zero_to_max", (processFun)zero_to_max, ztm_argv, 0, 0, -1, 1);

	for (i = 0; i < TOTAL_PROCESSES; i++)
		syscall_waitpid(pids[i], NULL);

	printf("SAME PRIORITY, THEN CHANGE IT...\n");

	for (i = 0; i < TOTAL_PROCESSES; i++) {
		pids[i] = syscall_create_process("zero_to_max", (processFun)zero_to_max, ztm_argv, 0, 0, -1, 1);
		syscall_changePrio(pids[i], prio[i]);
		printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
	}

	for (i = 0; i < TOTAL_PROCESSES; i++)
		syscall_waitpid(pids[i], NULL);

	printf("SAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

	for (i = 0; i < TOTAL_PROCESSES; i++) {
		pids[i] = syscall_create_process("zero_to_max", (processFun)zero_to_max, ztm_argv, 0, 0, -1, 1);
		syscall_block(pids[i]);
		syscall_changePrio(pids[i], prio[i]);
		printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
	}

	for (i = 0; i < TOTAL_PROCESSES; i++)
		syscall_unblock(pids[i]);

	for (i = 0; i < TOTAL_PROCESSES; i++)
		syscall_waitpid(pids[i], NULL);

	printf("TEST PRIO COMPLETED\n");
	syscall_exit();
	return 0;
}
