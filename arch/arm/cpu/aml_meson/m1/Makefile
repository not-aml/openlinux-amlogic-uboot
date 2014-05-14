

include $(TOPDIR)/config.mk

LIB	=  $(obj)lib$(SOC).a

SOBJS	:= mmutable.o
COBJS	+= timer.o soc.o board.o sdio.o
COBJS   += serial.o clock.o pinmux.o clk_set.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))

all:	 $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
