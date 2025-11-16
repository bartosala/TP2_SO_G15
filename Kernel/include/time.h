#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>


void timer_handler();
int ticks_elapsed();
int seconds_elapsed();
void wait_ticks(uint64_t ticksToWait);
void wait_seconds(uint64_t secondsToWait);
void setup_timer(uint16_t freq);

#endif
