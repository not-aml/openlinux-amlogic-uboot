/*
 *
 * arch/arm/include/asm/arch-m1/am_reg_addr.h
 *
 *  Copyright (C) 2010 AMLOGIC, INC.
 *
 * License terms: GNU General Public License (GPL) version 2
 * Basic register address definitions in physical memory and
 * some block defintions for core devices like the timer.
 */
#ifndef __MACH_MESSON_REGS_ADDR_H_
#define __MACH_MESSON_REGS_ADDR_H_
#include <asm/arch/am_regs.h>
/** Pin mux convert */
#define P_PERIPHS_PIN_MUX_0      CBUS_REG_ADDR(PERIPHS_PIN_MUX_0 )
#define P_PERIPHS_PIN_MUX_1      CBUS_REG_ADDR(PERIPHS_PIN_MUX_1 )
#define P_PERIPHS_PIN_MUX_2      CBUS_REG_ADDR(PERIPHS_PIN_MUX_2 )
#define P_PERIPHS_PIN_MUX_3      CBUS_REG_ADDR(PERIPHS_PIN_MUX_3 )
#define P_PERIPHS_PIN_MUX_4      CBUS_REG_ADDR(PERIPHS_PIN_MUX_4 )
#define P_PERIPHS_PIN_MUX_5      CBUS_REG_ADDR(PERIPHS_PIN_MUX_5 )
#define P_PERIPHS_PIN_MUX_6      CBUS_REG_ADDR(PERIPHS_PIN_MUX_6 )
#define P_PERIPHS_PIN_MUX_7      CBUS_REG_ADDR(PERIPHS_PIN_MUX_7 )
#define P_PERIPHS_PIN_MUX_8      CBUS_REG_ADDR(PERIPHS_PIN_MUX_8 )
#define P_PERIPHS_PIN_MUX_9      CBUS_REG_ADDR(PERIPHS_PIN_MUX_9 )
#define P_PERIPHS_PIN_MUX_10     CBUS_REG_ADDR(PERIPHS_PIN_MUX_10)
#define P_PERIPHS_PIN_MUX_11     CBUS_REG_ADDR(PERIPHS_PIN_MUX_11)
#define P_PERIPHS_PIN_MUX_12     CBUS_REG_ADDR(PERIPHS_PIN_MUX_12)


#define P_PREG_JTAG_GPIO_EN_N    CBUS_REG_ADDR(PREG_JTAG_GPIO_ADDR)
#define P_PREG_JTAG_GPIO_O    	 CBUS_REG_ADDR(PREG_JTAG_GPIO_ADDR)
#define P_PREG_JTAG_GPIO_I    	 CBUS_REG_ADDR(PREG_JTAG_GPIO_ADDR)
#define P_PREG_AGPIO_EN_N        CBUS_REG_ADDR(PREG_EGPIO_EN_N)
#define P_PREG_AGPIO_O           CBUS_REG_ADDR(PREG_EGPIO_O     )
#define P_PREG_AGPIO_I           CBUS_REG_ADDR(PREG_EGPIO_I     )

#define P_PREG_BGPIO_EN_N        CBUS_REG_ADDR(PREG_EGPIO_EN_N)
#define P_PREG_BGPIO_O           CBUS_REG_ADDR(PREG_EGPIO_O     )
#define P_PREG_BGPIO_I           CBUS_REG_ADDR(PREG_EGPIO_I     )

#define P_PREG_CGPIO_EN_N        CBUS_REG_ADDR(PREG_FGPIO_EN_N  )
#define P_PREG_CGPIO_O           CBUS_REG_ADDR(PREG_FGPIO_O     )
#define P_PREG_CGPIO_I           CBUS_REG_ADDR(PREG_FGPIO_I     )

#define P_PREG_DGPIO_EN_N        CBUS_REG_ADDR(PREG_GGPIO_EN_N  )
#define P_PREG_DGPIO_O           CBUS_REG_ADDR(PREG_GGPIO_O     )
#define P_PREG_DGPIO_I           CBUS_REG_ADDR(PREG_GGPIO_I     )

#define P_PREG_EGPIO_EN_N        CBUS_REG_ADDR(PREG_HGPIO_EN_N  )
#define P_PREG_EGPIO_O           CBUS_REG_ADDR(PREG_HGPIO_O     )
#define P_PREG_EGPIO_I           CBUS_REG_ADDR(PREG_HGPIO_I     )


#define P_NDMA_AES_CONTROL		 CBUS_REG_ADDR(NDMA_AES_CONTROL)
#define P_PREG_CTLREG0_ADDR      CBUS_REG_ADDR(PREG_CTLREG0_ADDR)

#define P_HHI_JTAG_CONFIG		 CBUS_REG_ADDR(HHI_JTAG_CONFIG)

#define P_HHI_MPEG_CLK_CNTL      CBUS_REG_ADDR(HHI_MPEG_CLK_CNTL)

#define P_MSR_CLK_REG0            CBUS_REG_ADDR(MSR_CLK_REG0)
#define P_MSR_CLK_REG1            CBUS_REG_ADDR(MSR_CLK_REG1)
#define P_MSR_CLK_REG2            CBUS_REG_ADDR(MSR_CLK_REG2)


#define P_SPI_FLASH_CMD                 CBUS_REG_ADDR(SPI_FLASH_CMD   )
#define P_SPI_FLASH_ADDR                CBUS_REG_ADDR(SPI_FLASH_ADDR  )
#define P_SPI_FLASH_CTRL                CBUS_REG_ADDR(SPI_FLASH_CTRL  )
#define P_SPI_FLASH_CTRL1               CBUS_REG_ADDR(SPI_FLASH_CTRL1 )
#define P_SPI_FLASH_STATUS              CBUS_REG_ADDR(SPI_FLASH_STATUS)
#define P_SPI_FLASH_CTRL2               CBUS_REG_ADDR(SPI_FLASH_CTRL2 )
#define P_SPI_FLASH_CLOCK               CBUS_REG_ADDR(SPI_FLASH_CLOCK )
#define P_SPI_FLASH_USER                CBUS_REG_ADDR(SPI_FLASH_USER  )
#define P_SPI_FLASH_USER1               CBUS_REG_ADDR(SPI_FLASH_USER1 )
#define P_SPI_FLASH_USER2               CBUS_REG_ADDR(SPI_FLASH_USER2 )
#define P_SPI_FLASH_USER3               CBUS_REG_ADDR(SPI_FLASH_USER3 )
#define P_SPI_FLASH_USER4               CBUS_REG_ADDR(SPI_FLASH_USER4 )
#define P_SPI_FLASH_SLAVE               CBUS_REG_ADDR(SPI_FLASH_SLAVE )
#define P_SPI_FLASH_SLAVE1              CBUS_REG_ADDR(SPI_FLASH_SLAVE1)
#define P_SPI_FLASH_SLAVE2              CBUS_REG_ADDR(SPI_FLASH_SLAVE2)
#define P_SPI_FLASH_SLAVE3              CBUS_REG_ADDR(SPI_FLASH_SLAVE3)
#define P_SPI_FLASH_C0                  CBUS_REG_ADDR(SPI_FLASH_C0    )
#define P_SPI_FLASH_C1                  CBUS_REG_ADDR(SPI_FLASH_C1    )
#define P_SPI_FLASH_C2                  CBUS_REG_ADDR(SPI_FLASH_C2    )
#define P_SPI_FLASH_C3                  CBUS_REG_ADDR(SPI_FLASH_C3    )
#define P_SPI_FLASH_C4                  CBUS_REG_ADDR(SPI_FLASH_C4    )
#define P_SPI_FLASH_C5                  CBUS_REG_ADDR(SPI_FLASH_C5    )
#define P_SPI_FLASH_C6                  CBUS_REG_ADDR(SPI_FLASH_C6    )
#define P_SPI_FLASH_C7                  CBUS_REG_ADDR(SPI_FLASH_C7    )
#define P_SPI_FLASH_B8                  CBUS_REG_ADDR(SPI_FLASH_B8    )
#define P_SPI_FLASH_B9                  CBUS_REG_ADDR(SPI_FLASH_B9    )
#define P_SPI_FLASH_B10                 CBUS_REG_ADDR(SPI_FLASH_B10   )
#define P_SPI_FLASH_B11                 CBUS_REG_ADDR(SPI_FLASH_B11   )
#define P_SPI_FLASH_B12                 CBUS_REG_ADDR(SPI_FLASH_B12   )
#define P_SPI_FLASH_B13                 CBUS_REG_ADDR(SPI_FLASH_B13   )
#define P_SPI_FLASH_B14                 CBUS_REG_ADDR(SPI_FLASH_B14   )
#define P_SPI_FLASH_B15                 CBUS_REG_ADDR(SPI_FLASH_B15   )




#define P_ISA_TIMER_MUX                 CBUS_REG_ADDR(ISA_TIMER_MUX)
#define P_ISA_TIMERA                    CBUS_REG_ADDR(ISA_TIMERA   )
#define P_ISA_TIMERB                    CBUS_REG_ADDR(ISA_TIMERB   )
#define P_ISA_TIMERC                    CBUS_REG_ADDR(ISA_TIMERC   )
#define P_ISA_TIMERD                    CBUS_REG_ADDR(ISA_TIMERD   )
#define P_ISA_TIMERE                    CBUS_REG_ADDR(ISA_TIMERE   )

#define P_WATCHDOG_TC					CBUS_REG_ADDR(WATCHDOG_TC)
#define P_WATCHDOG_RESET                CBUS_REG_ADDR(WATCHDOG_RESET)


#endif
