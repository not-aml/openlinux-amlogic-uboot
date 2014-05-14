
include $(TOPDIR)/config.mk
ifneq ($(CONFIG_ENABLE_ARMCC_DEBUGROM),)
ARMCC_$(CONFIG_ENABLE_ARMCC_DEBUGROM)=$(shell echo __CC_ARM | armcc -E -  | tail -n 1)
endif
ifneq ($(ARMCC_y),)
targetComplier=arm
else
targetComplier=gcc
endif

all:	  
	$(MAKE) -f Makefile.$(targetComplier)

