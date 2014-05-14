CONFIG_AMLROM_SPL	= y

include $(TOPDIR)/config.mk
ifdef SOC
LDSCRIPT= $(TOPDIR)/romboot_spl/$(ARCH)/$(CPU)/$(SOC)/u-boot.lds
else
LDSCRIPT= $(TOPDIR)/romboot_spl/$(ARCH)/$(CPU)/u-boot.lds
endif
#-Ttext $(TEXT_BASE)
LDFLAGS	= -Bstatic -T $(obj)u-boot.lds  $(PLATFORM_LDFLAGS)
AFLAGS	+= -DCONFIG_AMLROM_SPL -DTEXT_BASE=$(TEXT_BASE) -MD
CFLAGS	+= -DCONFIG_AMLROM_SPL -DTEXT_BASE=$(TEXT_BASE)  -MD


#EXTERN_LIB=/home/yuhao/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.4.1/libgcc.a
EXTERN_LIB=
##/home/yuhao/CodeSourcery/Sourcery_G++_Lite/lib/gcc/arm-none-eabi/4.4.1/libgcc.a

SOBJS	= rom_spl.o 
#start.o cpu_init.o lowlevel_init.o
COBJS	= ddr.o relocate_init.o memtest.o debug_serial.o  lowlevel_init.o 
#nand_boot.o nand_ecc.o s3c64xx.o

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
vpath %.c $(SRCTREE)/$(CPUDIR)/$(SOC) $(SRCTREE)/common
vpath %.c $(SRCTREE)/$(CPUDIR)  $(TOPDIR)/board/$(BOARDDIR)
vpath %.S $(SRCTREE)/$(CPUDIR)/$(SOC)
vpath %.S $(SRCTREE)/$(CPUDIR)	$(TOPDIR)/board/$(BOARDDIR)


ALL	= $(OBJTREE)/u-boot-spl.bin 

all:	 $(ALL)


$(OBJTREE)/u-boot-spl.bin:	$(OBJTREE)/u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(OBJTREE)/u-boot-spl:	$(OBJS) $(obj)u-boot.lds
	$(LD) $(LDFLAGS) $(OBJS) $(EXTERN_LIB)\
		-Map $(@:%=%.map) \
		-o $@

$(obj)u-boot.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@




sinclude $(wildcard $(obj)*.d)

#########################################################################
