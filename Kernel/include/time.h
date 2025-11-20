#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>


/*
 * timer_handler
 * Interrupt handler for the timer. Called by the IRQ/tick routine.
 */
void timer_handler();

/*
 * ticks_elapsed
 * @return: number of timer ticks since boot or since timer reset
 */
int ticks_elapsed();

/*
 * seconds_elapsed
 * @return: number of seconds elapsed since boot or since timer reset
 */
int seconds_elapsed();

/*
 * wait_ticks
 * Blocks or busy-waits until the specified number of ticks have elapsed.
 * @param ticksToWait: number of timer ticks to wait
 */
void wait_ticks(uint64_t ticksToWait);

/*
 * wait_seconds
 * Blocks or busy-waits until the specified number of seconds have elapsed.
 * @param secondsToWait: number of seconds to wait
 */
void wait_seconds(uint64_t secondsToWait);

/*
 * setup_timer
 * Configures the timer hardware to `freq` Hz.
 * @param freq: desired frequency in Hz
 */
void setup_timer(uint16_t freq);

#endif
