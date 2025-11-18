#include <programs.h>

// MVar simulation using two pipes:
// - data_pipe: carries the char values written by writers and consumed by readers
// - token_pipe: a single-token pipe used as a binary semaphore (1 token means MVar is empty)
// Usage: mvar <num_writers> <num_readers>

static void active_wait_random(uint64_t seed)
{
	uint64_t iters = (seed % 5 + 1) * 50;
	for (uint64_t i = 0; i < iters; i++)
		syscall_yield();
}

// Writer entry: argv: [value_char, data_pipe_id, token_pipe_id, seed, NULL]
uint64_t mvar_writer_entry(uint64_t argc, char *argv[])
{
	if (argc < 4)
		return -1;
	char val = argv[0][0];
	int data_pipe = satoi(argv[1]);
	int token_pipe = satoi(argv[2]);
	uint64_t seed = satoi(argv[3]);

	while (1) {
		active_wait_random(seed);

		// acquire token (blocks if no token -> MVar full)
		char dummy;
		if (syscall_read(token_pipe, &dummy, 1) <= 0) {
			// read error, just yield and retry
			syscall_yield();
			continue;
		}

		// write the value into data pipe
		if (syscall_write(data_pipe, (char *)&val, 1) <= 0) {
			// write error: yield and retry
			syscall_yield();
		}
	}
	return 0;
}

// Reader entry: argv: [idstr, data_pipe_id, token_pipe_id, seed, NULL]
uint64_t mvar_reader_entry(uint64_t argc, char *argv[])
{
	if (argc < 4)
		return -1;
	char *idstr = argv[0];
	int data_pipe = satoi(argv[1]);
	int token_pipe = satoi(argv[2]);
	uint64_t seed = satoi(argv[3]);

	while (1) {
		active_wait_random(seed + 7);

		char v = 0;
		if (syscall_read(data_pipe, &v, 1) <= 0) {
			syscall_yield();
			continue;
		}

		// print id and consumed value
		printf("%s%c", idstr, v);

		// release token back to token_pipe
		char one = 1;
		if (syscall_write(token_pipe, &one, 1) <= 0) {
			syscall_yield();
		}
	}
	return 0;
}

uint64_t mvar(uint64_t argc, char *argv[])
{
	if (argc != 2)
		return -1;

	int num_writers = satoi(argv[0]);
	int num_readers = satoi(argv[1]);

	if (num_writers <= 0 || num_writers > 26) {
		printf("Error: num_writers debe estar entre 1 y 26.\n");
		return -1;
	}
	if (num_readers <= 0 || num_readers > 20) {
		printf("Error: num_readers debe estar entre 1 y 20.\n");
		return -1;
	}

	// create pipes
	int data_pipe = syscall_open_pipe();
	if (data_pipe < 0) {
		printf("Error: no se pudo crear el pipe de datos.\n");
		return -1;
	}
	int token_pipe = syscall_open_pipe();
	if (token_pipe < 0) {
		printf("Error: no se pudo crear el pipe token.\n");
		return -1;
	}

	// initialize token pipe with one token (so MVar starts empty)
	char one = 1;
	if (syscall_write(token_pipe, &one, 1) <= 0) {
		printf("Warning: no se pudo inicializar token pipe.\n");
	}

	// spawn writers
	for (int i = 0; i < num_writers; i++) {
		// allocate argv for child so it remains valid
		char **pargv = malloc(5 * sizeof(char *));
		if (!pargv)
			continue;

		pargv[0] = malloc(2);
		pargv[0][0] = 'A' + (i % 26);
		pargv[0][1] = '\0';

		pargv[1] = malloc(32);
		uint64ToStr((uint64_t)data_pipe, pargv[1]);

		pargv[2] = malloc(32);
		uint64ToStr((uint64_t)token_pipe, pargv[2]);

		pargv[3] = malloc(32);
		uint64ToStr((uint64_t)(i + 1), pargv[3]);

		pargv[4] = NULL;

		char name[32];
		strcpy(name, "writer_");
		name[7] = 'A' + (i % 26);
		name[8] = '\0';
		syscall_create_process(name, (processFun)mvar_writer_entry, pargv, 1, 0, -1, 1);
	}

	// spawn readers
	for (int i = 0; i < num_readers; i++) {
		char **pargv = malloc(5 * sizeof(char *));
		if (!pargv)
			continue;

		pargv[0] = malloc(16);
		pargv[0][0] = 'R';
		uint64ToStr((uint64_t)(i + 1), pargv[0] + 1);
		size_t len = strlen(pargv[0]);
		pargv[0][len] = ':';
		pargv[0][len + 1] = '\0';

		pargv[1] = malloc(32);
		uint64ToStr((uint64_t)data_pipe, pargv[1]);

		pargv[2] = malloc(32);
		uint64ToStr((uint64_t)token_pipe, pargv[2]);

		pargv[3] = malloc(32);
		uint64ToStr((uint64_t)(i + 10), pargv[3]);

		pargv[4] = NULL;

		char name[32];
		strcpy(name, "reader_");
		uint64ToStr((uint64_t)(i + 1), name + 7);
		syscall_create_process(name, (processFun)mvar_reader_entry, pargv, 1, 0, -1, 1);
	}

	printf("MVar: %d escritores creados, %d lectores creados.\n", num_writers, num_readers);

	return 0;
}

