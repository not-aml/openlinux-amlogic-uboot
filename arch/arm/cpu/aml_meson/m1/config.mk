
CROSS_COMPILE=arm-none-eabi-
ARM_CPU=cortex-a9
PLATFORM_CPPFLAGS += -mcpu=$(ARM_CPU) -mthumb-interwork -O -mno-long-calls  -Wall
#end of SRAM on 6236M
TEXT_BASE=0x87800000
LDSCRIPT := $(SRCTREE)/$(CPUDIR)/m1/u-boot.lds
