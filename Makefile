BINARY = main

OBJS =

LIBNAME		= opencm3_stm32f1
DEFS		+= -DSTM32F1

FP_FLAGS	?= -msoft-float
ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

OOCD		?= openocd
OOCD_INTERFACE	?= olimex-arm-usb-ocd-h
OOCD_BOARD	?= olimex_stm32_h103

include Makefile.rules
