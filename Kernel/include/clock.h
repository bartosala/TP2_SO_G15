#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

typedef struct time{
    int sec;
    int min;
    int hour;
    int day;
    int month;
    int year;
} time_t;

time_t getTime(int64_t timeZone);

uint64_t getTimeParam(uint64_t param); // -1 si falla

int64_t diffTimeMillis(time_t start, time_t end);

#endif // CLOCK_H