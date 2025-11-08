#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <syscall.h>

#define MAX_LENGTH 512
#define MAX_INT_LENGTH 21
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define NULL ((void *)0)

static uint32_t uintToBase(uint64_t value, char *buffer, uint32_t base)
{
	char *p = buffer;
	char *p1, *p2;
	uint32_t digits = 0;

	// Calculate characters for each digit
	do {
		uint32_t remainder = value % base;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
		digits++;
	} while (value /= base);

	// Terminate string in buffer.
	*p = 0;

	// Reverse string in buffer.
	p1 = buffer;
	p2 = p - 1;
	while (p1 < p2) {
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}

	return digits;
}

uint64_t strlen(const char *s)
{
	uint64_t length = 0;
	for (; s[length] != '\0'; length++)
		;
	return length;
}

void intToStr(int n, char *buff)
{
	uintToBase(n, buff, 10);
}

void signedIntToStr(int n, char *buff)
{
	if (n < 0) {
		buff[0] = '-';
		uintToBase(-n, buff + 1, 10);
	} else {
		uintToBase(n, buff, 10);
	}
}

void uint64ToStr(uint64_t n, char *buff)
{
	uintToBase(n, buff, 10);
}

void intToHex(int n, char *buff)
{
	uintToBase(n, buff, 16);
}

void uint64ToHex(uint64_t n, char *buff)
{
	uintToBase(n, buff, 16);
}

// ========== STRING UTILITIES ==========

int strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

void strcpy(char *dest, const char *src)
{
	int i = 0;
	while (src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}

void strncpy(char *dest, const char *src, uint64_t length)
{
	for (uint64_t i = 0; i < length; i++) {
		dest[i] = src[i];
	}
	dest[length] = '\0';
}

int checkNumber(char *str)
{
	if (str == NULL || *str == '\0')
		return 0;

	if (*str == '-' || *str == '+')
		str++; // Skip sign

	if (*str == '\0')
		return 0; // Just a sign is not a number

	while (*str) {
		if (*str < '0' || *str > '9')
			return 0; // Non-digit character found
		str++;
	}
	return 1; // All characters are digits
}

int64_t satoi(char *str)
{
	uint64_t i = 0;
	int64_t res = 0;
	int8_t sign = 1;

	if (!str)
		return 0;

	if (str[i] == '-') {
		i++;
		sign = -1;
	}

	for (; str[i] != '\0'; ++i) {
		if (str[i] < '0' || str[i] > '9')
			return 0;
		res = res * 10 + str[i] - '0';
	}

	return res * sign;
}

int atoi(const char *str)
{
	return (int)satoi((char *)str);
}

char *itoa(int n)
{
	char *buff = (char *)malloc(MAX_INT_LENGTH);
	intToStr(n, buff);
	return buff;
}

// ========== PRINTF HELPER ==========

/**
 * @brief Helper to copy a string to output buffer with bounds checking
 * @param output Output buffer
 * @param k Current position in output
 * @param str String to copy
 * @param max_length Maximum length of output buffer
 * @return New position in output buffer
 */
static int safe_string_copy(char *output, int k, const char *str, int max_length)
{
	for (int j = 0; str[j] != 0 && k < max_length - 1; j++, k++) {
		output[k] = str[j];
	}
	return k;
}

int format_printf(const uint64_t fd, const char *format, va_list args)
{
	// Use stack buffer instead of malloc to avoid memory allocation issues
	char output[MAX_LENGTH];
	for (int i = 0; i < MAX_LENGTH; i++) {
		output[i] = 0;
	}

	int i = 0, k = 0;
	while (format[i] != 0 && k < MAX_LENGTH - 1) {
		if (format[i] == '%' && format[i + 1] != 0) {
			i++;
			switch (format[i]) {
			case 'd': {
				int num = va_arg(args, int);
				char str[MAX_INT_LENGTH];
				signedIntToStr(num, str);
				k = safe_string_copy(output, k, str, MAX_LENGTH);
				break;
			}
			case 'l': {
				uint64_t num = va_arg(args, uint64_t);
				char str[MAX_INT_LENGTH];
				uint64ToStr(num, str);
				k = safe_string_copy(output, k, str, MAX_LENGTH);
				break;
			}
			case 's': {
				char *str = va_arg(args, char *);
				k = safe_string_copy(output, k, str, MAX_LENGTH);
				break;
			}
			case 'c': {
				uint8_t c = va_arg(args, int);
				output[k++] = c;
				break;
			}
			case 'x': {
				int num = va_arg(args, int);
				char str[MAX_INT_LENGTH];
				intToHex(num, str);
				k = safe_string_copy(output, k, str, MAX_LENGTH);
				break;
			}
			case 'p': {
				uint64_t ptr = va_arg(args, uint64_t);
				char str[20]; // Enough space for 64-bit hex + "0x" + null terminator
				output[k++] = '0';
				output[k++] = 'x';
				uint64ToHex(ptr, str);
				k = safe_string_copy(output, k, str, MAX_LENGTH);
				break;
			}
			default:
				break;
			}
		} else {
			output[k++] = format[i];
		}
		i++;
	}

	uint64_t toReturn = syscall_write(fd, output, k);
	return toReturn;
}

uint64_t printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	uint64_t aux = format_printf(STDOUT, format, args);
	va_end(args);
	return aux;
}

uint64_t printferror(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	uint64_t aux = format_printf(STDERR, format, args);
	va_end(args);
	return aux;
}

char getChar()
{
	char c;
	syscall_read(STDIN, &c, 1);
	return c;
}

char *strstr(const char *haystack, const char *needle)
{
	if (!*needle) {
		return (char *)haystack;
	}

	for (const char *h = haystack; *h; h++) {
		const char *hIter = h;
		const char *nIter = needle;

		while (*hIter && *nIter && *hIter == *nIter) {
			hIter++;
			nIter++;
		}

		if (!*nIter) {
			return (char *)h;
		}
	}

	return NULL;
}

unsigned long int next = 1;

unsigned int randInt()
{
	next = next * 1103515245 + 12345;
	return (next / 65536) % 32768;
}
void *malloc(uint64_t size)
{
	if (size == 0)
		return NULL;
	void *ptr = syscall_allocMemory(size);
	if (ptr == NULL) {
		printferror("Error allocating memory of size %llu\n", size);
	}
	return ptr;
}

void free(void *ptr)
{
	if (ptr == NULL)
		return;
	syscall_freeMemory(ptr);
}