OBJS += config.o
OBJS += core.o
OBJS += cdc_acm.o

OBJS-$(stm32f1) += hw_usbd.o
OBJS-$(stm32f1) += hw_f1.o

$(OBJS) $(OBJS-y): CFLAGS += -include $(SRCDIR)/defs.h
