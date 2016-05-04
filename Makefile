BINARY = main

OBJS = images.o puff.o

LIBNAME		= opencm3_stm32f1
DEFS		+= -DSTM32F1

FP_FLAGS	?= -msoft-float
ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

OOCD		?= openocd
OOCD_INTERFACE	?= olimex-arm-usb-ocd-h
OOCD_BOARD	?= olimex_stm32_h103

include Makefile.rules

images.bin: imgprep.py images/*
	@python imgprep.py

images.o: images.bin
	@$(OBJCOPY) -I binary -O elf32-littlearm -B arm $< $@
	@$(OBJCOPY) --rename-section .data=.rodata,alloc,load,readonly,data,contents $@ $@

