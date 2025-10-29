

all:  bootloader kernel userland image

bootloader:
	$(MAKE) -C Bootloader MM=$(MM) all

kernel:
	$(MAKE) -C Kernel MM=$(MM) all

userland:
	$(MAKE) -C Userland MM=$(MM) all

image: kernel bootloader userland
	$(MAKE) -C Image MM=$(MM) all

clean:
	$(MAKE) -C Bootloader clean
	$(MAKE) -C Image clean
	$(MAKE) -C Kernel clean
	$(MAKE) -C Userland clean

.PHONY: bootloader image collections kernel userland all clean
