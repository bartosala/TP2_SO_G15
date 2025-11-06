#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>
#include <stdlib.h>
#include <shell.h>
#include <syscall.h>
#include <shellfunctions.h>
#include <stddef.h>
#include "../../../Shared/shared_structs.h"

//utils

/**
 * @brief Lee de la entrada estandar un string
 * @param buff Buffer donde poner lo leído
 * @param length cantidad de caracteres a leer (menor o igual al tamaño del buffer)
 * @return cantidad de caracteres leidos
 */
uint64_t readLine(char * buff, uint64_t length);

/**
 * @brief Analiza un string y lo divide en argumentos
 * @param arg string con los argumentos
 * @param args array de strings donde se guardan los argumentos
 * @param max_args cantidad de argumentos esperados
 * @param max_size tamaño maximo del array de strings
 */
int parse_string(char *arg, char **args, int max_args, int max_size);

/**
 * @brief Analiza los argumentos de un comando y los guarda en un array de strings.
 * @param arg string con los argumentos
 * @param args array de strings donde se guardan los argumentos
 * @param expected_args cantidad de argumentos esperados
 * @param max_size tamaño maximo del array de strings
 * @return -1 si hay error en el parsing, 0 si los argumentos son correctos y no hay &, 1 si los argumentos son correctos y hay &
 */
int anal_arg(char *arg, char **args, int expected_args, int max_size);

//shellfunctions
void printHeader();
void printProcessInfo(PCB processInfo); 

//programs
void loop(uint64_t argc, char *argv[]);
uint64_t cat(uint64_t argc, char *argv[]);
uint64_t wc(uint64_t argc, char *argv[]);
uint64_t filter(uint64_t argc, char *argv[]);

#endif