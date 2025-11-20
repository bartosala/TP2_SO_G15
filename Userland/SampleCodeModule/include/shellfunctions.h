#ifndef SHELL_FUNCTIONS_H
#define SHELL_FUNCTIONS_H

#include "../../../Shared/shared_structs.h"
#include <stdarg.h>
#include <stdint.h>
#include <testfunctions.h>

typedef struct command {
	int instruction;
	char *arguments;
	char foreground; // 1 si es foreground, 0 si es background
} command;

typedef struct pipecmd {
	command cmd1;
	command cmd2;
} pipeCmd;

void clearBuffer();

pid_t handle_help(char *arg, int sdtin, int stdout);
pid_t handle_echo(char *arg, int sdtin, int stdout);
pid_t handle_clear(char *arg, int sdtin, int stdout);
pid_t handle_test_mm(char *arg, int sdtin, int stdout);
pid_t handle_test_processes(char *arg, int sdtin, int stdout);
pid_t handle_test_prio(char *arg, int sdtin, int stdout);
pid_t handle_test_sync(char *arg, int sdtin, int stdout);
pid_t handle_ps(char *arg, int sdtin, int stdout);
pid_t handle_mem_info(char *arg, int sdtin, int stdout);
pid_t handle_loop(char *arg, int sdtin, int stdout);
pid_t handle_pid_info(char *arg, int sdtin, int stdout);
pid_t handle_nice(char *arg, int sdtin, int stdout);
pid_t handle_wc(char *arg, int sdtin, int stdout);
pid_t handle_filter(char *arg, int sdtin, int stdout);
pid_t handle_cat(char *arg, int sdtin, int stdout);
pid_t handle_mvar(char *arg, int sdtin, int stdout);
pid_t handle_test_malloc_free(char *arg, int sdtin, int stdout);

void kill(char *arg);
void block(char *arg);
void unblock(char *arg);
uint64_t nice(int argc, char **argv);

#endif // SHELL_FUNCTIONS_H