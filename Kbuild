obj-m += neopixel.o

neopixel-objs := src/main.o
neopixel-objs += src/fs.o
neopixel-objs += platforms/src/BCM2835.o

EXTRA_CFLAGS := -I$(src)/include
EXTRA_CFLAGS += -I$(src)/platforms/include
