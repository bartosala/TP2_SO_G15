#ifndef STACK_FRAME_H
#define STACK_FRAME_H

/*
 * setUpStackFrame
 * Prepares the initial stack frame for a new process.
 * @param stackBase: pointer to the base/top of the new stack memory
 * @param entryPoint: function/address where the process will start executing
 * @param argc: argument count
 * @param argv: argument vector
 * @return: initial stack pointer (uint64_t) to use for the process
 */
uint64_t setUpStackFrame(uint64_t *stackBase, uint64_t entryPoint, uint64_t argc, char **argv);

/*
 * stackFrame (assembly helper)
 * @param base: base pointer for the stack
 * @param entryPoint: process entry function
 * @param argc: argument count
 * @param arg: argument vector
 * @return: resulting stack pointer after preparing frame
 */
uint64_t stackFrame(uint64_t base, uint64_t entryPoint, uint64_t argc, char **arg);

/*
 * wrapper
 * Assembly wrapper that calls the process entry function with proper args.
 * @param entryPoint: address of entry function
 * @param argv: argument vector
 */
void wrapper(uint64_t entryPoint, char **argv);

#endif /* STACK_FRAME_H */