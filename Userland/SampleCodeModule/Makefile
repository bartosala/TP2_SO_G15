include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
TEST_SOURCES=$(wildcard tests/*.c)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
TEST_OBJECTS=$(TEST_SOURCES:.c=.o)
INCLUDE=./include

all: tests $(MODULE)

$(MODULE): $(OBJECTS) $(OBJECTS_ASM) $(TEST_OBJECTS)
	$(GCC) $(GCCFLAGS) -T sampleCodeModule.ld _loader.c $(OBJECTS) $(OBJECTS_ASM) $(TEST_OBJECTS) -o ../$(MODULE) -I $(INCLUDE)

tests:
	cd tests && make all

%.o: %.c
	$(GCC) $(GCCFLAGS) -I$(INCLUDE) -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf asm/*.o *.o
	cd tests && make clean && cd ..

.PHONY: all clean tests