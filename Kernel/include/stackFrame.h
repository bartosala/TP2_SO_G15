#ifndef STACK_FRAME_H
#define STACK_FRAME_H

uint64_t setUpStackFrame(uint64_t *stackBase, uint64_t entryPoint, uint64_t argc, char **argv);
// asm

uint64_t stackFrame(uint64_t base, uint64_t entryPoint, uint64_t argc, char **arg);
void wrapper(uint64_t entryPoint, char **argv);

#endif