#include "../../Shared/shared_structs.h"
#include <programs.h>
#include <shell.h>
#include <shellfunctions.h>
#include <stdlib.h>
#include <syscall.h>
#include <testfunctions.h>

// ========== HELPER FUNCTIONS ==========

/**
 * @brief Creates a simple process with no arguments
 * @param name Process name
 * @param function Process function
 * @param foreground 1 for foreground, 0 for background
 * @param stdin Standard input fd
 * @param stdout Standard output fd
 * @return pid on success, -1 on error
 */
static pid_t create_simple_process(const char *name, processFun function, char foreground, int stdin, int stdout)
{
	pid_t pid = syscall_create_process((char *)name, function, NULL, 1, foreground, stdin, stdout);

	if (pid < 0) {
		printferror("Error al crear el proceso %s\n", name);
		return -1;
	}

	return pid;
}

/**
 * @brief Creates a process with arguments
 * @param name Process name
 * @param function Process function
 * @param args Arguments array
 * @param argc Number of arguments
 * @param foreground 1 for foreground, 0 for background
 * @param stdin Standard input fd
 * @param stdout Standard output fd
 * @return pid on success, -1 on error
 */
static pid_t create_process_with_args(const char *name, processFun function, char **args, int argc, char foreground,
                                      int stdin, int stdout)
{
	char **argv = malloc((argc + 1) * sizeof(char *));
	if (argv == NULL) {
		printferror("Error al asignar memoria para los argumentos\n");
		return -1;
	}

	for (int i = 0; i < argc; i++) {
		argv[i] = args[i];
	}
	argv[argc] = NULL;

	pid_t pid = syscall_create_process((char *)name, function, argv, 1, foreground, stdin, stdout);
	free(argv);

	if (pid < 0) {
		printferror("Error al crear el proceso %s\n", name);
		return -1;
	}

	return pid;
}

/**
 * @brief Parses arguments and creates a process with validation
 * @param name Process name
 * @param function Process function
 * @param arg Raw argument string
 * @param expected_argc Expected number of arguments
 * @param usage Usage string to print on error
 * @param stdin Standard input fd
 * @param stdout Standard output fd
 * @param use_stdin Whether process should use stdin (for pipes)
 * @return pid on success, -1 on error
 */
static pid_t handle_process_with_args(const char *name, processFun function, char *arg, int expected_argc,
                                      const char *usage, int stdin, int stdout, char use_stdin)
{
	char arg_storage[expected_argc > 0 ? expected_argc : 1][32];
	char *args[expected_argc > 0 ? expected_argc : 1];

	for (int i = 0; i < (expected_argc > 0 ? expected_argc : 1); i++) {
		args[i] = arg_storage[i];
	}

	int bg = anal_arg(arg, args, expected_argc, 32);

	if (bg == -1) {
		printferror("%s", usage);
		return -1;
	}

	char foreground = !bg;
	pid_t pid;

	if (expected_argc == 0) {
		pid = create_simple_process(name, function, foreground, use_stdin ? stdin : -1, stdout);
	} else {
		pid = create_process_with_args(name, function, args, expected_argc, foreground, use_stdin ? stdin : -1, stdout);
	}

	if (pid < 0) {
		return -1;
	}

	return foreground ? pid : 0;
}

/**
 * @brief Validates a PID to ensure it's not shell or idle
 * @param pid Process ID to check
 * @return 0 if valid, 1 if invalid
 */
static char checkErrorInPid(uint64_t pid)
{
	if (pid == 1) {
		printferror("Error: la shell no se toca\n");
		return 1;
	} else if (pid == 0) {
		printferror("Error: idle no se toca\n");
		return 1;
	}
	return 0;
}

// ========== COMMAND HANDLERS ==========

void clearBuffer()
{
	syscall_clear_pipe(0);
}

uint64_t doHelp(uint64_t argc, char **argv)
{
	printf("\n Lista de comandos disponibles:\n");
	printf(" - help: muestra este menu de ayuda\n");
	printf(" - echo <texto>: imprime el texto especificado\n");
	printf(" - clear: borra la pantalla y comienza arriba\n");
	printf(" - exit: sale de la shell\n");
	printf(" - memInfo: muestra estado de memoria\n");
	printf(" - ps: muestra todos los procesos con su informacion\n");
	printf(" - loop <tiempo>: imprime su PID cada tiempo especificado\n");
	printf(" - kill <pid>: mata el proceso con el PID especificado\n");
	printf(" - nice <pid> <prioridad>: cambia la prioridad de un proceso (0-5)\n");
	printf(" - block <pid>: bloquea el proceso con el PID especificado\n");
	printf(" - unblock <pid>: desbloquea el proceso con el PID especificado\n");
	printf(" - cat: muestra el input tal cual se recibe (usa Ctrl+D para terminar)\n");
	printf(" - wc: cuenta la cantidad de lineas del input\n");
	printf(" - filter: filtra las vocales del input\n");
	printf(" - <comando> | <comando>: conecta dos procesos con un pipe\n");
	printf(" - <comando> &: ejecuta el comando en background\n");
	printf(" - test_mm <max_memory>: test de gestion de memoria\n");
	printf(" - test_processes <max_processes>: test de creacion y manejo de procesos\n");
	printf(" - test_prio <max_value>: test de prioridades.\n");
	printf(" - test_sync <iterations> <use_sem>: test de sincronizacion (0=sin sem, 1=con sem)\n");
	printf(" - test_malloc_free: test de malloc y free\n");
	printf(" - Ctrl + C: mata el proceso en foreground\n");
	printf(" - Ctrl + D: envia EOF (fin de archivo) al proceso\n");
	return 0;
}

pid_t handle_help(char *arg, int stdin, int stdout)
{
	free(arg);
	return create_simple_process("help", (processFun)doHelp, 1, stdin, stdout);
}

uint64_t echo(int argc, char **argv)
{
	if (argc == 0) {
		return 1;
	}
	printf("%s\n", *argv);
	return 0;
}

pid_t handle_echo(char *arg, int stdin, int stdout)
{
	return syscall_create_process("echo", (processFun)echo, &arg, 1, 1, -1, stdout);
}

uint64_t clearScreen(int argc, char **argv)
{
	syscall_clearScreen();
	return 0;
}

pid_t handle_clear(char *arg, int stdin, int stdout)
{
	free(arg);
	return create_simple_process("clear", (processFun)clearScreen, 1, stdin, stdout);
}

uint64_t showMemInfo(int argc, char **argv)
{
	printf("[Memoria] Estado:\n");

	memInfo info;
	if (syscall_memInfo(&info) == -1) {
		printferror("Error al obtener el estado de memoria\n");
		return 1;
	}

	printf("Memoria total: %l bytes\n", info.total);
	printf("Memoria usada: %l bytes\n", info.used);
	printf("Memoria libre: %l bytes\n", info.free);
	return 0;
}

pid_t handle_mem_info(char *arg, int stdin, int stdout)
{
	free(arg);
	return create_simple_process("memInfo", (processFun)showMemInfo, 1, stdin, stdout);
}

pid_t handle_test_mm(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("test_mm", (processFun)test_mm, arg, 1, "Uso: test_mm <max_memory>\n", stdin,
	                                stdout, 0);
}

pid_t handle_test_processes(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("test_processes", (processFun)test_processes, arg, 1,
	                                "Uso: test_processes <max_processes>\n", stdin, stdout, 0);
}

pid_t handle_test_prio(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("test_prio", (processFun)test_prio, arg, 1, "Uso: test_prio <max_value>\n", stdin, stdout, 0);
}

pid_t handle_test_sync(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("test_sync", (processFun)test_sync, arg, 2,
	                                "Uso: test_sync <iterations> <use_sem>\n  iterations: numero de iteraciones por "
	                                "proceso\n  use_sem: 1 para usar semaforos, 0 para no usarlos\n",
	                                stdin, stdout, 0);
}

uint64_t processInfo(uint64_t argc, char **argv)
{
	uint64_t cantProcesses;

	PCB *processInfo = syscall_getProcessInfo(&cantProcesses);

	if (cantProcesses == 0 || processInfo == NULL) {
		printf("No se encontraron procesos.\n");
		return -1;
	}

	printHeader();
	for (int i = 0; i < cantProcesses; i++) {
		printProcessInfo(processInfo[i]);
	}

	syscall_freeMemory(processInfo);
	return 0;
}

pid_t handle_ps(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("ps", (processFun)processInfo, arg, 0, "Uso: ps\n", stdin, stdout, 0);
}

pid_t handle_loop(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("loop", (processFun)loop, arg, 1, "Uso: loop <time>\n", stdin, stdout, 0);
}

pid_t handle_wc(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("wc", (processFun)wc, arg, 0, "Uso: wc\n", stdin, stdout, 1);
}

pid_t handle_filter(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("filter", (processFun)filter, arg, 0, "Uso: filter\n", stdin, stdout, 1);
}

pid_t handle_cat(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("cat", (processFun)cat, arg, 0, "Uso: cat\n", stdin, stdout, 1);
}

uint64_t nice(int argc, char **argv)
{
	uint64_t pid = satoi(argv[0]);
	int8_t new_priority = satoi(argv[1]);

	if (checkErrorInPid(pid)) {
		return 1;
	}

	if (new_priority < 0 || new_priority > 5) {
		printferror("Error: la prioridad debe estar entre 0 y 5\n");
		return 1;
	}

	if (syscall_changePrio(pid, new_priority) == -1) {
		printferror("Error al cambiar la prioridad del proceso %l\n", pid);
		return 1;
	}

	printf("Prioridad del proceso %l cambiada a %d\n", pid, new_priority);
	return 0;
}

pid_t handle_nice(char *arg, int stdin, int stdout)
{
	return handle_process_with_args("nice", (processFun)nice, arg, 2, "Uso: nice <pid> <new_priority>\n", stdin, stdout,
	                                0);
}

uint64_t test_malloc_free(int argc, char **argv)
{
	printf("Estado de memoria antes de malloc:\n");
	memInfo info;
	syscall_memInfo(&info);
	printf("Usada: %l, Libre: %l\n", info.used, info.free);

	uint64_t size = 1000;
	void *ptr = syscall_allocMemory(size);
	if (!ptr) {
		printferror("Error al reservar memoria\n");
		return 1;
	}
	printf("Estado de memoria despues de malloc:\n");
	syscall_memInfo(&info);
	printf("Usada: %l, Libre: %l\n", info.used, info.free);

	syscall_freeMemory(ptr);

	printf("Estado de memoria despues de free:\n");
	syscall_memInfo(&info);
	printf("Usada: %l, Libre: %l\n", info.used, info.free);
	return 0;
}

pid_t handle_test_malloc_free(char *arg, int stdin, int stdout)
{
	free(arg);
	return create_simple_process("test_malloc_free", (processFun)test_malloc_free, 1, -1, stdout);
}

void kill(char *arg)
{
	uint64_t pid = satoi(arg);
	if (checkErrorInPid(pid)) {
		return;
	}

	if (syscall_kill(pid) == -1) {
		printferror("Error al matar el proceso %l\n", pid);
	} else {
		printf("Proceso %l finalizado\n", pid);
	}
}

void block(char *arg)
{
	uint64_t pid = satoi(arg);
	if (checkErrorInPid(pid)) {
		return;
	}

	if (syscall_block(pid) == -1) {
		printferror("Error al bloquear el proceso %l\n", pid);
	} else {
		printf("Proceso %l bloqueado\n", pid);
	}
}

void unblock(char *arg)
{
	uint64_t pid = satoi(arg);
	if (checkErrorInPid(pid)) {
		return;
	}

	if (syscall_unblock(pid) == -1) {
		printferror("Error al desbloquear el proceso %l\n", pid);
	} else {
		printf("Proceso %l desbloqueado\n", pid);
	}
}