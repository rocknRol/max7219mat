
obj-m += max7219mat_driver.o


KERNEL_DIR ?= $(HOME)/dev/LinuxKernelHacking/raspberrypi3b/linux

all:
	make CONFIG_DEBUG_SECTION_MISMATCH=y -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=$(PWD) modules
