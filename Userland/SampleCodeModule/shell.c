#include "shell.h"
#include "syscall.h"
#include <programs.h>
#include <stdint.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE 1
#define MAX_ECHO 1000
#define MAX_USERNAME_LENGTH 16
#define PROMPT "%s$> "
#define CANT_INSTRUCTIONS 19
uint64_t curr = 0;

typedef enum {
	HELP = 0,
	ECHO,
	CLEAR,
	TEST_MM,
	TEST_PROCESSES,
	TEST_PRIO,
	TEST_SYNC,
	PS,
	MEM_INFO,
	LOOP,
	NICE,
	WC,
	FILTER,
	CAT,
	TEST_MALLOC_FREE,
	KILL,
	BLOCK,
	UNBLOCK,
	EXIT
} instructions;

#define IS_BUILT_IN(i) ((i) >= KILL && (i) < EXIT)

// ========== FORWARD DECLARATIONS ==========

static pid_t (*instruction_handlers[CANT_INSTRUCTIONS - 4])(char *, int, int);
static void (*built_in_handlers[(EXIT - KILL)])(char *);

// ========== HELPER FUNCTIONS ==========

/**
 * @brief Allocates memory with error handling
 * @param size Size to allocate
 * @param error_msg Error message to print on failure
 * @return Pointer to allocated memory or NULL on failure
 */
static char *safe_malloc(size_t size, const char *error_msg)
{
	char *ptr = malloc(size * sizeof(char));
	if (ptr == NULL) {
		printferror("%s", error_msg);
	}
	return ptr;
}

/**
 * @brief Frees pipe command and its arguments
 * @param pipe_cmd Pipe command structure to free
 */
static void free_pipe_cmd(pipeCmd *pipe_cmd)
{
	if (pipe_cmd == NULL)
		return;

	if (pipe_cmd->cmd1.arguments != NULL) {
		free(pipe_cmd->cmd1.arguments);
	}
	if (pipe_cmd->cmd2.arguments != NULL) {
		free(pipe_cmd->cmd2.arguments);
	}
	free(pipe_cmd);
}

/**
 * @brief Prints error and cleans up pipe command
 * @param error_msg Error message to print
 * @param pipe_cmd Pipe command to clean up
 */
static void error_and_cleanup(const char *error_msg, pipeCmd *pipe_cmd)
{
	printferror("%s", error_msg);
	free_pipe_cmd(pipe_cmd);
}

/**
 * @brief Handles single command execution with proper cleanup
 * @param pipe_cmd Pipe command containing the command to execute
 * @return 1 to exit shell, 0 to continue
 */
static int handle_single_command(pipeCmd *pipe_cmd)
{
	if (pipe_cmd->cmd1.instruction == EXIT) {
		free(pipe_cmd->cmd1.arguments);
		free(pipe_cmd);
		return 1; // Signal to exit shell
	}

	pid_t pid = instruction_handlers[pipe_cmd->cmd1.instruction](pipe_cmd->cmd1.arguments, 0, 1);
	if (pid < 0) {
		printferror("Error al ejecutar el comando.\n");
	} else if (pid == 0) {
		// Background process - don't wait
		printf("Proceso ejecutado en background.\n");
	} else {
		// Foreground process - wait for completion
		int status = 0;
		syscall_waitpid(pid, &status);
		// Don't print "Proceso X terminado" for successful completion (status 0)
		if (status != 0) {
			printf("Proceso %d terminado con estado %d\n", pid, status);
		}
	}
	free(pipe_cmd->cmd1.arguments);
	free(pipe_cmd);
	return 0; // Continue shell
}

/**
 * @brief Handles built-in command execution with cleanup
 * @param pipe_cmd Pipe command containing the built-in command
 */
static void handle_builtin_command(pipeCmd *pipe_cmd)
{
	if (pipe_cmd->cmd1.instruction == -1) {
		error_and_cleanup("Comando invalido.\n", pipe_cmd);
	} else if (IS_BUILT_IN(pipe_cmd->cmd1.instruction)) {
		built_in_handlers[pipe_cmd->cmd1.instruction - KILL](pipe_cmd->cmd1.arguments);
		free(pipe_cmd->cmd1.arguments);
		free(pipe_cmd);
	}
}

// ========== COMMAND HANDLING ==========

static char *inst_list[] = {
    "help", "echo", "clear",  "test_mm", "test_processes",   "test_prio", "test_sync", "ps",      "memInfo", "loop",
    "nice", "wc",   "filter", "cat",     "test_malloc_free", "kill",      "block",     "unblock", "exit",
};

static pid_t (*instruction_handlers[CANT_INSTRUCTIONS - 4])(char *, int, int) = {
    handle_help,      handle_echo,      handle_clear,  handle_test_mm,  handle_test_processes,
    handle_test_prio, handle_test_sync, handle_ps,     handle_mem_info, handle_loop,
    handle_nice,      handle_wc,        handle_filter, handle_cat,      handle_test_malloc_free,
};

static void (*built_in_handlers[(EXIT - KILL)])(char *) = {
    kill,
    block,
    unblock,
};

int verifyInstruction(char *instruction)
{
	for (int i = 0; i < CANT_INSTRUCTIONS - 1; i++) {
		if (strcmp(inst_list[i], instruction) == 0) {
			return i;
		}
	}
	return -1;
}

int getInstruction(char *buffer, char *arguments)
{
	int i = 0;
	int j = 0;

	char *instruction = safe_malloc(BUFFER_SPACE, "Error al asignar memoria para la instruccion.\n");
	if (instruction == NULL) {
		return -1;
	}

	for (; i < BUFFER_SPACE; i++) {
		if (buffer[i] == ' ' || buffer[i] == '\0') {
			instruction[j] = 0;
			i++;
			break;
		} else {
			instruction[j++] = buffer[i];
		}
	}

	int arg_index = 0;
	while (buffer[i] != '\0' && buffer[i] != '\n') {
		arguments[arg_index++] = buffer[i++];
	}
	arguments[arg_index] = '\0';
	free(buffer);

	int iNum = 0;
	if ((iNum = verifyInstruction(instruction)) == -1 && instruction[0] != 0) {
		printferror("Comando no reconocido: %s\n", instruction);
	}
	free(instruction);
	return iNum;
}

static int bufferControl(pipeCmd *pipe_cmd)
{
	char *shell_buffer = safe_malloc(BUFFER_SPACE, "Error al asignar memoria para el buffer.\n");
	if (shell_buffer == NULL) {
		return -1;
	}

	int instructions = 0;
	readLine(shell_buffer, BUFFER_SPACE);

	char *pipe_pos = strstr(shell_buffer, "|");
	if (pipe_pos != NULL) {
		*(pipe_pos - 1) = 0;
		*pipe_pos = 0;
		*(pipe_pos + 1) = 0;
		char *arg2 = safe_malloc(BUFFER_SPACE, "Error al asignar memoria para arg2.\n");
		pipe_cmd->cmd2.instruction = getInstruction(pipe_pos + 2, arg2);
		pipe_cmd->cmd2.arguments = arg2;
		if (pipe_cmd->cmd2.instruction >= 0)
			instructions++;
	}
	char *arg1 = safe_malloc(BUFFER_SPACE, "Error al asignar memoria para arg1.\n");
	pipe_cmd->cmd1.instruction = getInstruction(shell_buffer, arg1);
	pipe_cmd->cmd1.arguments = arg1;
	if (pipe_cmd->cmd1.instruction >= 0)
		instructions++;
	if (IS_BUILT_IN(pipe_cmd->cmd1.instruction) && IS_BUILT_IN(pipe_cmd->cmd2.instruction)) {
		error_and_cleanup("No se pueden usar comandos built-in con pipes.\n", pipe_cmd);
		return -1;
	}
	if (IS_BUILT_IN(pipe_cmd->cmd1.instruction)) {
		return 0;
	}
	return instructions;
}

static void handle_piped_commands(pipeCmd *pipe_cmd)
{
	if (pipe_cmd->cmd1.instruction == -1 || pipe_cmd->cmd2.instruction == -1) {
		error_and_cleanup("Comando invalido.\n", pipe_cmd);
		return;
	}
	if (IS_BUILT_IN(pipe_cmd->cmd1.instruction) || IS_BUILT_IN(pipe_cmd->cmd2.instruction)) {
		error_and_cleanup("No se pueden usar comandos built-in con pipes.\n", pipe_cmd);
		return;
	}

	int pipe = syscall_open_pipe();
	if (pipe < 0) {
		error_and_cleanup("Error al abrir el pipe\n", pipe_cmd);
		return;
	}

	pid_t pids[2];
	pids[0] = instruction_handlers[pipe_cmd->cmd1.instruction](pipe_cmd->cmd1.arguments, 0, pipe);
	pids[1] = instruction_handlers[pipe_cmd->cmd2.instruction](pipe_cmd->cmd2.arguments, pipe, 1);

	if (pids[0] < 0 || pids[1] < 0) {
		printferror("Error al crear los procesos del pipe\n");
		syscall_close_pipe(pipe);
		free_pipe_cmd(pipe_cmd);
		return;
	}

	int status = 0;
	syscall_waitpid(pids[0], &status);
	free(pipe_cmd->cmd1.arguments);

	syscall_waitpid(pids[1], &status);
	free(pipe_cmd->cmd2.arguments);

	free(pipe_cmd);
	syscall_close_pipe(pipe);
}

uint64_t shell(uint64_t argc, char **argv)
{
	syscall_clearScreen();

	// ASCII Art Header
	syscall_sizeUpFont(1);
	printf("\n");
	printf("  ========================================\n");
	printf("                                          \n");
	printf("         SHELL - Sistema Operativo                \n");
	printf("               TP2 - Grupo 15                     \n");
	printf("                                           \n");
	printf("  ========================================\n");
	printf("\n");
	syscall_sizeDownFont(1);

	// Welcome message
	printf("  > Bienvenido a la shell interactiva\n");
	printf("  > Escribe 'help' para ver todos los comandos\n");
	printf("  > Escribe 'exit' para salir\n");
	printf("\n");

	printf("  Ingrese su nombre de usuario: ");
	char username[MAX_USERNAME_LENGTH];
	readLine(username, MAX_USERNAME_LENGTH);

	unsigned int exit = FALSE;
	int instructions;
	syscall_clearScreen();
	pipeCmd *pipe_cmd;
	while (!exit) {
		clearBuffer();
		printf(PROMPT, username);
		pipe_cmd = (pipeCmd *)malloc(sizeof(pipeCmd));
		if (pipe_cmd == NULL) {
			printferror("Error al asignar memoria para los argumentos.\n");
			return 1;
		}
		pipe_cmd->cmd1 = (command){-1, 0};
		pipe_cmd->cmd2 = (command){-1, 0};
		instructions = bufferControl(pipe_cmd);
		switch (instructions) {
		case 0: // built-in
			handle_builtin_command(pipe_cmd);
			break;
		case 1:
			if (handle_single_command(pipe_cmd)) {
				exit = TRUE;
			}
			break;
		case 2:
			handle_piped_commands(pipe_cmd);
			break;

		default:
			break;
		}
	}

	// Farewell message
	printf("\n");
	printf("  ============================================================\n");
	printf("  ||                                                        ||\n");
	printf("  ||                Cerrando la shell...                   ||\n");
	printf("  ||                                                        ||\n");
	printf("  ============================================================\n");
	printf("\n");

	syscall_wait(2);
	syscall_clearScreen();

	printf("\n\n");
	syscall_sizeUpFont(1);
	printf("  ============================================================\n");
	printf("  ||                                                        ||\n");
	printf("  ||          Gracias TOTALES - Cerati              		||\n");
	printf("  ||                                                        ||\n");
	printf("  ============================================================\n");
	syscall_sizeDownFont(1);
	printf("\n");

	syscall_wait(150);
	syscall_clearScreen();
	return 0;
}