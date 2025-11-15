#ifndef STDLIB_H
#define STDLIB_H

#include <stdarg.h>
#include <stdint.h>
#include <syscall.h>

#define EOF -1

/**
 * @brief Longitud de un string null-terminated
 * @param s string para sacarle la longitud
 *
 */
uint64_t strlen(const char *s);

/**
 * @brief Imprime en pantalla un string formateado
 * @param format string con formato
 * @param ... argumentos a imprimir
 * @return cantidad de caracteres impresos
 */
uint64_t printf(const char *format, ...);

/**
 * @brief Imprime en pantalla un string formateado de error
 * @param format string con formato
 * @param ... argumentos a imprimir
 * @return cantidad de caracteres impresos
 */
uint64_t printferror(const char *format, ...);

/**
 * @brief Busca una subcadena en un string
 * @param haystack string donde buscar
 * @param needle subcadena a buscar
 * @return puntero a la subcadena encontrada o NULL si no se encuentra
 */
char *strstr(const char *haystack, const char *needle);

/**
 * @brief Copia un string a otro
 * @param dest string destino
 * @param src string origen
 */
void strcpy(char *dest, const char *src);

/**
 * @brief Copia un string a otro
 * @param dest string destino
 * @param src string origen
 * @param length cantidad de caracteres a copiar
 */
void strncpy(char *dest, const char *src, uint64_t length);

/**
 * @brief Compara dos strings
 * @param s1 primer string
 * @param s2 segundo string
 * @return 0 si son iguales, otro valor si no
 */
int strcmp(const char *s1, const char *s2);

/**
 * @brief Convierte un entero a string
 * @param n entero a convertir
 * @param buff buffer donde se guardará el string
 */
void intToStr(int n, char *buff);

/**
 * @brief Convierte un entero a string en hexadecimal
 * @param n entero a convertir
 * @param buff buffer donde se guardará el string
 */
void intToHex(int n, char *buff);

/**
 * @brief Convierte un entero con signo a string (maneja números negativos)
 * @param n entero con signo a convertir
 * @param buff buffer donde se guardará el string
 */
void signedIntToStr(int n, char *buff);

/**
 * @brief Convierte un uint64_t a string
 * @param n uint64_t a convertir
 * @param buff buffer donde se guardará el string
 */
void uint64ToStr(uint64_t n, char *buff);

/**
 * @brief Obtiene un carácter de la entrada estándar.
 *
 * Esta función espera y devuelve un único carácter ingresado por el usuario.
 * No requiere parámetros y devuelve el carácter como tipo char.
 *
 * @return El carácter ingresado por el usuario.
 */
char getChar();

/**
 * @brief Genera un número entero sin signo aleatorio.
 *
 * Esta función devuelve un valor entero sin signo aleatorio.
 *
 * @return Un número entero sin signo aleatorio.
 */
unsigned int randInt();

/**
 * @brief Reserva memoria dinámica
 * @param size tamaño de la memoria a reservar
 * @return puntero a la memoria reservada
 */
void *malloc(uint64_t size);

int checkNumber(char *str);

/**
 * @brief Libera memoria dinámica
 * @param ptr puntero a la memoria a liberar
 */
void free(void *ptr);

/**
 * @brief Convierte un string a entero
 * @param str string a convertir
 * @return entero convertido
 */
int64_t satoi(char *str);

/**
 * @brief Convierte un string a entero
 * @param str string a convertir
 * @return entero convertido
 */
int atoi(const char *str);

/**
 * @brief Convierte un entero a string
 * @param n entero a convertir
 * @param buff buffer donde se guardará el string
 */
char *itoa(int n);

int checkNumber(char *str);

#endif // STDLIB_H