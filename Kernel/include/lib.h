#ifndef LIB_H
#define LIB_H

#include <stdint.h>

void * memset(void * destination, int32_t character, uint64_t length);
void * memcpy(void * destination, const void * source, uint64_t length);

char * strncpy(char * destination, const char * source, uint64_t length);
uint64_t strlen(const char * str);

char *cpuVendor(char *result);

uint64_t* getRegisters();

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void callTimerTick();

#endif