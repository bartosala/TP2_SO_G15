#include <time.h>
#include <lib.h>
#include <textModule.h>
#include <interrupts.h>
#include <scheduler.h>
#include <clock.h>

static uint64_t ticks = 0;
static uint16_t frequency = 18;

void timer_handler() {
	ticks++;
}

int ticks_elapsed() {
	_cli();
	int t = ticks;
	_sti();
	return t;
}

int seconds_elapsed() {
	_cli();
	int t = ticks / frequency;
	_sti();
	return t;
}

void wait_ticks(uint64_t ticksToWait) {
	uint64_t currentTicks;
	_cli();
	currentTicks = ticks;
	_sti();
	uint64_t targetTicks = currentTicks + ticksToWait;
	while (currentTicks < targetTicks) {
		yield();
		_cli();
		currentTicks = ticks;
		_sti();
	}
}

void wait_seconds(uint64_t secondsToWait) {
	wait_ticks(secondsToWait * frequency);
}

void setup_timer(uint16_t freq) {
    uint16_t divisor = 1193180 / freq;

    outb(0x43, 0x36);            
    outb(0x40, divisor & 0xFF);   
    outb(0x40, (divisor >> 8) & 0xFF); 

    frequency = freq;
}
