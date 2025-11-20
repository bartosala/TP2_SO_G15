#include <defs.h>
#include <lib.h>
#include <memoryManager.h>
#include <process.h>
#include <stackFrame.h>

// To set up stack frame
/* copyToStack (static)
 * Copies argv strings into the newly allocated stack area, building a
 * argv-style array on the stack and returning a pointer to it.
 */
static char **copyToStack(uint64_t *paramsBase, uint64_t argc, char **argv);

uint64_t setUpStackFrame(uint64_t *base, uint64_t entryPoint, uint64_t argc, char **argv)
{
	/* allocate stack and position the base at the top */
	*base = (uint64_t)allocMemory(STACK_SIZE);
	if (*base == 0) {
		return 0;
	}

	*base += STACK_SIZE;

	/* copy parameters and prepare argv on the stack, then create final frame */
	uint64_t paramsBase = *base; /* space for parameters */
	char **newArgv = copyToStack(&paramsBase, argc, argv);
	uint64_t rspFinal = stackFrame(paramsBase, entryPoint, argc, newArgv);
	return rspFinal;
}

static char **copyToStack(uint64_t *paramsBase, uint64_t argc, char **argv)
{
	char **newArgv = NULL;
	if (argc > 0 && argv != NULL) {

		/* reserve space for argv pointers */
		*paramsBase -= sizeof(char *) * (argc + 1);
		newArgv = (char **)*paramsBase;

		/* copy strings in reverse so pointers point to correct locations */
		for (int i = argc - 1; i >= 0; i--) {
			int len = strlen(argv[i]) + 1;
			*paramsBase -= len;
			strncpy((char *)*paramsBase, argv[i], len);
			newArgv[i] = (char *)*paramsBase;
		}
		newArgv[argc] = NULL;
	}
	return newArgv;
}
