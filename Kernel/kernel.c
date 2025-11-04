#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <idtLoader.h>
#include <clock.h>
#include <time.h>
#include <videoDriver.h>
#include <textModule.h>
#include <keyboardDriver.h>
#include <memoryManager.h>
#include <defs.h>
#include <process.h>
#include <scheduler.h>
#include <interrupts.h>
#include <pipe.h>
#include <semaphore.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

#define SAMPLE_CODE_MODULE_ADDR    0x400000UL
#define SAMPLE_DATA_MODULE_ADDR    0x500000UL



static void * const sampleCodeModuleAddress = (void*)SAMPLE_CODE_MODULE_ADDR;
static void * const sampleDataModuleAddress = (void*)SAMPLE_DATA_MODULE_ADDR;

typedef int (*EntryPoint)();


void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				
		- sizeof(uint64_t)			
	);
}

void * initializeKernelBinary()
{
	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	clearBSS(&bss, &endOfKernel - &bss);
	return getStackBase();
}


void printSlow(char * str, uint32_t color, uint64_t pause){
	for(int i = 0; str[i]; i++){
		putChar(str[i], color);
		wait_ticks(pause);
	}
}

uint64_t idle(uint64_t argc, char **argv) {
	while(1){
		_hlt();
	}
	return 0xdeadbeef;
}

#define WHITE 0x00FFFFFF
#define RED 0x000000FF

int main(){	
	_cli();
	fontSizeUp(2);
	printStr(" TP 2 SO \n", WHITE);
	fontSizeDown(2);

	// MEMORY MANAGER
	createMemoryManager((void*)HEAP_START_ADDRESS, HEAP_SIZE);
	startScheduler(idle);
	if(createSemaphoresManager() == NULL) {
		printStr("Error initializing semaphore manager\n", RED);
		return -1;
	}
	createProcess("shell", (processFun) sampleCodeModuleAddress, 0, NULL, 0, 1, 0, 1);
	load_idt();
	clear_buffer();
	_sti();
	while (1) {
		_hlt();
	}

	printStr("DEATH ZONE\n", RED);
	// This point should never be reached
	return 0;
}



/*
int main()
{	
	load_idt();
	fontSizeUp(2);
	printStr(" TPE ARQUI \n", WHITE);
	fontSizeDown(2);

	createMemoryManager((void*)HEAP_START_ADDRESS, HEAP_SIZE);
	setup_timer(18);
	clear_buffer();
	
	((EntryPoint)sampleCodeModuleAddress)(); // Llamada al userland
	clearScreen(0);
	
	return 0;
}



*/