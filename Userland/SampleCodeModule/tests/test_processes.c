#include "../../../Shared/shared_structs.h"
#include "test_util.h"
#include <syscall.h>
#include <testfunctions.h>

typedef struct P_rq {
	int32_t pid;
	State state;
} p_rq;

int64_t test_processes(uint64_t argc, char *argv[])
{
	uint8_t rq;
	uint8_t alive = 0;
	uint8_t action;
	uint64_t max_processes;
	char *argvAux[] = {0};

	if (argc != 1)
		return -1;

	if ((max_processes = satoi(argv[0])) <= 0)
		return -1;

	p_rq p_rqs[max_processes];

	while (1) {

		// Create max_processes processes
		for (rq = 0; rq < max_processes; rq++) {
			p_rqs[rq].pid = syscall_create_process("endless_loop", (processFun)endless_loop, argvAux, 2, 0, -1,
			                                       1); // prio 2, fg 0 (background), stdin -1, stdout 1

			if (p_rqs[rq].pid == -1) {
				printf("test_processes: ERROR creating process\n");
				return -1;
			} else {
				p_rqs[rq].state = RUNNING;
				alive++;
			}
		}

		// Randomly kills, blocks or unblocks processes until every one has been killed
		while (alive > 0) {

			for (rq = 0; rq < max_processes; rq++) {
				action = GetUniform(100) % 2;

				switch (action) {
				case 0:
					if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED) {
						if (syscall_kill(p_rqs[rq].pid) == -1) {
							printf("test_processes: ERROR killing process\n");
							return -1;
						}
						p_rqs[rq].state = EXITED;
						alive--;
					}
					break;

				case 1:
					if (p_rqs[rq].state == RUNNING) {
						if (syscall_block(p_rqs[rq].pid) == -1) {
							printf("test_processes: ERROR blocking process\n");
							return -1;
						}
						p_rqs[rq].state = BLOCKED;
					}
					break;
				}
			}

			// Randomly unblocks processes
			for (rq = 0; rq < max_processes; rq++)
				if (p_rqs[rq].state == BLOCKED && GetUniform(100) % 2) {
					if (syscall_unblock(p_rqs[rq].pid) == -1) {
						printf("test_processes: ERROR unblocking process\n");
						return -1;
					}
					p_rqs[rq].state = RUNNING;
				}
		}
	}
}
