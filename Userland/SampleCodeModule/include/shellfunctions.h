#ifndef SHELLFUNCTIONS_H
#define SHELLFUNCTIONS_H

#include <stdarg.h>

void showTime();
void showRegisters();

// handler functions
void help_handler(char *arg);
void time_handler(char *arg);
void registers_handler(char *arg);
void echo_handler(char *arg);
void size_up_handler(char *arg);
void size_down_handler(char *arg);
void test_div_0_handler(char *arg);
void test_invalid_opcode_handler(char *arg);
void clear_handler(char *arg);


#endif //SHELLFUNCTIONS_H
