include Makefile.inc

KERNEL=kernel.bin

# Filter memory manager source based on MM flag
ifeq ($(MM),BUDDY)
    MM_SOURCE=Memorymanager/BuddyMemoryManager.c
else
    MM_SOURCE=Memorymanager/bitmapMemoryManager.c
endif

SOURCES=$(wildcard *.c) $(MM_SOURCE) $(wildcard interruptions/*.c) $(wildcard drivers/*.c) $(wildcard processes/*.c) $(wildcard semaphores/*.c) $(wildcard syscall/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)


%.o: %.c
	$(GCC) $(GCCFLAGS) $(MM:%=-D%) -I./include -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o *.o Memorymanager/*.o *.bin

.PHONY: all clean