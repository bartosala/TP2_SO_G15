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

/*
 * getTime
 * @param timeZone: offset in hours from UTC (can be negative)
 * @return: current local time as a `time_t` structure adjusted by `timeZone`.
 */
time_t getTime(int64_t timeZone);

/*
 * getTimeParam
 * @param param: parameter selector (implementation-specific). Returns a
 *               numeric time-related value or (uint64_t)-1 on failure.
 * @return: requested time parameter as uint64_t, or (uint64_t)-1 if it fails.
 */
uint64_t getTimeParam(uint64_t param);

/*
 * diffTimeMillis
 * @param start: start time
 * @param end:   end time
 * @return: difference between `end` and `start` in milliseconds. If `end`
 *          is earlier than `start` the result may be negative.
 */
int64_t diffTimeMillis(time_t start, time_t end);

#endif // CLOCK_H