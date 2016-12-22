ifneq ($(KERNELRELEASE),)
	include Kbuild
else
KERNEL_HEADERS=/lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNEL_HEADERS) M=$(PWD) modules

clean:
	make -C $(KERNEL_HEADERS) M=$(PWD) clean
endif
