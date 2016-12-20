KERNEL_HEADERS=/lib/modules/$(shell uname -r)/build

obj-m := main.o

all:
	make -C $(KERNEL_HEADERS) M=$(PWD) modules

clean:
	make -C $(KERNEL_HEADERS) M=$(PWD) clean