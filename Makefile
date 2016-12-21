KERNEL_HEADERS=/lib/modules/$(shell uname -r)/build

obj-m += neopixel.o
neopixel-objs := main.o fs.o BCM2835.o

all:
	make -C $(KERNEL_HEADERS) M=$(PWD) modules

clean:
	make -C $(KERNEL_HEADERS) M=$(PWD) clean
