OBJS += vectors.o
OBJS += string.o
OBJS += cortex.o
OBJS += time.o
OBJS += timer.o
OBJS += util.o
OBJS-$(debug) += console.o
OBJS += main.o
OBJS += gpio.o

SUBDIRS += mcu usb

# Avoid infinite loops due to GCC noticing code that can be replaced by a call
# to a standard library function... within our implementation of that function.
util.o: CFLAGS += -fno-tree-loop-distribute-patterns
