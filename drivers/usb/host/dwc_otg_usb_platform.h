/* platform header */
/*
 * (C) Copyright 2010 Amlogic, Inc
 *
 * Victor Wan, victor.wan@amlogic.com, 
 * 2010-03-24 @ Shanghai
 *
 */

 #ifndef __PLATFORM_H__
 #define __PLATFORM_H__


#define PERI_BASE_ADDR               0xc1100000

#define WRITE_PERI_REG(reg, val) *(volatile unsigned *)(PERI_BASE_ADDR + ((reg)<<2)) = (val)
#define READ_PERI_REG(reg) (*(volatile unsigned *)(PERI_BASE_ADDR + ((reg)<<2)))

#define CLEAR_PERIPHS_REG_BITS(reg, mask) WRITE_PERI_REG(reg, (READ_PERI_REG(reg)&(~(mask))))
#define SET_PERIPHS_REG_BITS(reg, mask)   WRITE_PERI_REG(reg, (READ_PERI_REG(reg)|(mask)))

#define PREI_USB_PHY_REG              0x2100 //0xC1108400
#define PREI_USB_PHY_A_REG1           0x2101
#define PREI_USB_PHY_B_REG1           0x2102

#define PREI_USB_PHY_A_POR      (1 << 0)
#define PREI_USB_PHY_B_POR      (1 << 1)
//#define PREI_USB_PHY_CLK_EN			(1 << 6) no CLK_EN in A1H
#define PREI_USB_PHY_CLK_SEL    (7 << 5) 
#define PREI_USB_PHY_CLK_GATE 	(1 << 8) 
#define PREI_USB_PHY_B_AHB_RSET     (1 << 11)
#define PREI_USB_PHY_B_CLK_RSET     (1 << 12)
#define PREI_USB_PHY_B_PLL_RSET     (1 << 13)
#define PREI_USB_PHY_A_AHB_RSET     (1 << 17)
#define PREI_USB_PHY_A_CLK_RSET     (1 << 18)
#define PREI_USB_PHY_A_PLL_RSET     (1 << 19)
#define PREI_USB_PHY_A_DRV_VBUS     (1 << 20)
#define PREI_USB_PHY_B_DRV_VBUS			(1 << 21)
#define PREI_USB_PHY_B_CLK_DETECT   (1 << 22)
#define PREI_USB_PHY_CLK_DIV        (0x7f << 24)
#define PREI_USB_PHY_A_CLK_DETECT   (1 << 31)

#define USB_PHY_TUNE_MASK_REFCLKDIV  (3 << 29)
#define USB_PHY_TUNE_MASK_REFCLKSEL  (3 << 27 )
#define USB_PHY_TUNE_MASK_SQRX          (7 << 16 )
#define USB_PHY_TUNE_MASK_TXVREF       (15 << 5)
#define USB_PHY_TUNE_MASK_OTGDISABLE    (1 << 2)
#define USB_PHY_TUNE_MASK_RISETIME  (3 << 9 )
#define USB_PHY_TUNE_MASK_VBUS_THRE (7 << 19)

#define USB_PHY_TUNE_SHIFT_REFCLKDIV  (29)
#define USB_PHY_TUNE_SHIFT_REFCLKSEL  (27)
#define USB_PHY_TUNE_SHIFT_SQRX          (16)
#define USB_PHY_TUNE_SHIFT_TXVREF       (5)
#define USB_PHY_TUNE_SHIFT_OTGDISABLE    (2)
#define USB_PHY_TUNE_SHIFT_RISETIME  (9)
#define USB_PHY_TUNE_SHIFT_VBUS_THRE (19)

#define PREI_TIMER_E_COUNT	0x2655

#define PREI_ARC_INTR0_MASK	0x2612
#define PREI_ARM_INTR0_MASK 0x2692

#define USB_PHY_A_INTR_BIT	(1 << 30)
#define USB_PHY_B_INTR_BIT	(1 << 31)
/*
#define ISABASE                      0x01000000

#define WRITE_ISA_REG(reg, val) *(volatile unsigned *)(ISABASE + (reg)) = (val)
#define READ_ISA_REG(reg) (*(volatile unsigned *)(ISABASE + (reg)))

#define CLEAR_ISA_REG_MASK(reg, mask) WRITE_ISA_REG(reg, (READ_ISA_REG(reg)&(~mask)))
#define SET_ISA_REG_MASK(reg, mask)   WRITE_ISA_REG(reg, (READ_ISA_REG(reg)|(mask)))

#define AHB_BRIDGE_CTRL0              0x2680
    #define AHB_BRIDGE_USB1_RD_CACHE_BIT  8
    #define AHB_BRIDGE_USB0_RD_CACHE_BIT  6	
#define AHB_BRIDGE_CTRL1              0x2685
//SET_ISA_REG_MASK(AHB_BRIDGE_CTRL1,(1 << 31));

#define IREG_TIMER_E_COUNT            0x2655
*/

//#define flush_cpu_cache() do{_invalidate_dcache();SET_ISA_REG_MASK(AHB_BRIDGE_CTRL1,(1 << 31));}while(0)
#define flush_cpu_cache() 

#define dwc_write_reg32(x, v) 	(*(volatile uint32_t *)(x + DWC_REG_BASE))=v
#define dwc_read_reg32(x) (*(volatile uint32_t*)(x + DWC_REG_BASE))
// void dwc_modify_reg32( volatile uint32_t *_reg, const uint32_t _clear_mask, const uint32_t _set_mask) 
#define dwc_modify_reg32(x, c, s) 	(*(volatile uint32_t *)(x + DWC_REG_BASE))=(dwc_read_reg32(x) & ~c |s)


void set_usb_phy_config(int cfg);


#endif
