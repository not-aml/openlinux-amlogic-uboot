

include $(TOPDIR)/config.mk

LIB	= $(obj)ucl_lib_$(CPU).a

START	= start.o
OBJS	= misc.o piggy.o interrupts.o

OBJS	:= $(addprefix $(obj),$(OBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)
#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
