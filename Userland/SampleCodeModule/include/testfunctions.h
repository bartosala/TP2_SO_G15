#ifndef TEST_FUNCTIONS_H
#define TEST_FUNCTIONS_H

#include <stdint.h>
#include <stdlib.h>

// Declaraciones de funciones de prueba
uint64_t test_mm(uint64_t argc, char *argv[]);
uint64_t test_prio();
int64_t test_processes(uint64_t argc, char *argv[]);
uint64_t test_sync(uint64_t argc, char *argv[]);

#endif // TEST_FUNCTIONS_H