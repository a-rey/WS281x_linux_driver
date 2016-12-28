obj-m += ws281x.o

ws281x-objs := src/main.o
ws281x-objs += src/fs.o
ws281x-objs += platforms/src/BCM2835.o

EXTRA_CFLAGS := -I$(src)/include
EXTRA_CFLAGS += -I$(src)/platforms/include
