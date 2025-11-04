/*
 * IRQ dispatcher: routes IRQ numbers to appropriate handlers.
 
#include <time.h>
#include <stdint.h>
#include <keyboardDriver.h>
#include <lib.h>

void int_20();
void int_21();

void irqDispatcher(uint64_t irq) {
	switch (irq) {
		case 0:
			int_20();
			break;
		case 1:
			int_21();
			break;
		default:
			break;
	}
	outb(0x20, 0x20); // EOI to PIC
	return;
}

void int_20(){
	timer_handler();
}

void int_21(){
	bufferWrite();
}
	*/
