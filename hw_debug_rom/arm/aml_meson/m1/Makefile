

include $(TOPDIR)/config.mk
ifdef SOC
LDSCRIPT= $(TOPDIR)/hw_debug_rom/$(ARCH)/$(CPU)/$(SOC)/u-boot.lds
T=$(TOPDIR)/hw_debug_rom/$(ARCH)/$(CPU)/$(SOC)/Makefile
else
LDSCRIPT= $(TOPDIR)/hw_debug_rom/$(ARCH)/$(CPU)/u-boot.lds
T=$(TOPDIR)/hw_debug_rom/$(ARCH)/$(CPU)/Makefile
endif
#-Ttext $(TEXT_BASE)
LDFLAGS	= -Bstatic -T $(obj)u-boot.lds  $(PLATFORM_LDFLAGS)
BURNLDFLAGS	= -Bstatic -T $(obj)burner.lds  $(PLATFORM_LDFLAGS)
AFLAGS	+= -DAML_DEBUG_ROM -DTEXT_BASE=$(TEXT_BASE) -MD -I$(OBJTREE)
CFLAGS	+= -DAML_DEBUG_ROM -DTEXT_BASE=$(TEXT_BASE) -MD -I$(OBJTREE)



SOBJS	= rom_spl.o 
COBJS	= ddr.o  memtest.o debug_serial.o  spi_flash.o pll_test.o 
BURNOBJS=rom_boot.o spi_flash.o burner_main.o debug_serial.o
SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS) run.o )
__OBJS	:= $(SOBJS) $(COBJS)
BURNOBJS:= $(addprefix $(obj),$(SOBJS) $(COBJS) burn.o )
vpath %.c $(SRCTREE)/$(CPUDIR)/$(SOC) $(SRCTREE)/common
vpath %.c $(SRCTREE)/$(CPUDIR)  $(TOPDIR)/board/$(BOARDDIR)
vpath %.S $(SRCTREE)/$(CPUDIR)/$(SOC)
vpath %.S $(SRCTREE)/$(CPUDIR)	$(TOPDIR)/board/$(BOARDDIR)
vpath %.s $(SRCTREE)/$(CPUDIR)/$(SOC)

ALL	= $(OBJTREE)/aml-debug-rom.bin $(OBJTREE)/aml-debug-rom-burner

all:	 $(ALL)
	
$(OBJTREE)/aml-debug-rom.bin:	$(OBJTREE)/aml-debug-rom
	echo a------------ 
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(OBJTREE)/aml-debug-rom:	$(OBJS) $(obj)burner.lds
	echo $@ ------------
	$(LD) $(BURNLDFLAGS) $(OBJS) $(EXTERN_LIB)\
		-Map $(@:%=%.map) \
		-o $@

$(obj)burner.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) -DEND_ADDR=0x49001800  $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@
$(obj)u-boot.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) -DEND_ADDR=0x49001800  $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

$(OBJTREE)/aml-debug-rom-burner:$(OBJTREE)/aml-debug-rom.bin $(BURNOBJS) $(obj)burner.lds
	echo $@ ------------
	$(LD) $(BURNLDFLAGS) $(BURNOBJS) $(EXTERN_LIB)\
		-Map $(@:%=%.map) \
		-o $@
$(obj)burn.c: $(T)
	@echo "unsigned char auto_run[4] __attribute__((section(\".script\"))) =\"SMas\";" >$@
	@echo "unsigned cur_char __attribute__((section(\".script\"))) =0;" >>$@
$(obj)run.c: $(T)
	@echo "unsigned char auto_run[4]__attribute__((section(\".script\"))) =\"  \";" >$@
	@echo "unsigned cur_char __attribute__((section(\".script\"))) =0;" >>$@


sinclude $(wildcard $(obj)*.d)
$(obj)rom_boot.o:$(OBJTREE)/aml-debug-rom.bin
#########################################################################
