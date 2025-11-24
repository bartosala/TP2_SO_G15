include ../../Makefile.inc

SOURCES=test_mm.c test_prio.c test_sync.c test_processes.c 
OBJECTS=$(SOURCES:.c=.o)
UTIL=test_util.o
INCLUDE=../include

all: $(UTIL) $(OBJECTS)
	cp $(OBJECTS) $(UTIL) ../

%.o: %.c $(UTIL)
	$(GCC) $(GCCFLAGS) -I$(INCLUDE) -c $< -o $@

$(UTIL): test_util.c
	$(GCC) $(GCCFLAGS) -I$(INCLUDE) -c $< -o $@

clean:
	rm -rf asm/*.o *.o

.PHONY: all clean