#include <common.h>
#include <asm/io.h>
#include <usb.h>
#include <malloc.h>
#include <linux/list.h>
#include <asm/arch/am_regs.h>


#include "dwc_otg_usb_platform.h"
static char * clock_src_name[]={
		"XTAL input",
		"XTAL input divided by 2",
		"other PLL",
		"DDR PLL",
		"dmod PLL"
};
/*
   cfg = 0 : XTAL input
   cfg = 1 : XTAL input divided by 2
   cfg = 2 : other PLL
   cfg = 3 : DDR pll
   cfg = 4 : dmod pll
  */
 
void set_usb_phy_config(int cfg)
{

    int divider =0;
    int clk_sel = cfg;
    int i;
    int time_dly = 50000;


	// ------------------------------------------------------------
	//  CLK_SEL: These bits select the source for the 12Mhz: 
	// 0 = XTAL input (24, 25, 27Mhz)
	// 1 = XTAL input divided by 2
	// 2 = other PLL output
	// 3 = DDR pll clock (typically 400mhz)
	// 4 = demod 240Mhz PLL output
	CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_CLK_SEL);
	//clk_sel = 0; // 24M CLK 
	//clk_sel = 1; // 12M, Phy default setting is 12Mhz
	//clk_sel = 2; // other PLL, 540M
	//clk_sel = 3; // DDR, 369M
	//clk_sel = 4; // demod, 240M
	
	printf("clk_sel: %s\n",clock_src_name[clk_sel]);
	SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, (clk_sel<<5 ));

  if(clk_sel == 3)//DDR runing 396MHz (396/(32+1)=12)
 	{
 		CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG,PREI_USB_PHY_CLK_DIV);
 		SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, (32 << 24));
  }else if(clk_sel == 2)//Other PLL running at 540M (540/(44+1)=12)
 	{
 		CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG,PREI_USB_PHY_CLK_DIV);
 		SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, (44 << 24));
  }else if(clk_sel == 4)// demod 240M (240/(19+1) = 12)
 	{
 		CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG,PREI_USB_PHY_CLK_DIV);
 		SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, (19 << 24));
  }
	// Open clock gate, to enable CLOCK to usb phy 
	SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_CLK_GATE);
	i=0;
	while(i++<time_dly){};
	
//#ifndef __USE_PORT_B
    /*  Reset USB PHY A  */
    SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_A_AHB_RSET);
    i=0;
    while(i++<time_dly){};  
    CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_A_AHB_RSET);
    i=0;
    while(i++<time_dly){};
    SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_A_CLK_RSET);
    i=0;
    while(i++<time_dly){};      
    CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_A_CLK_RSET);
    i=0;
    while(i++<time_dly){};
    SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_A_PLL_RSET);
    i=0;
    while(i++<time_dly){};
    CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_A_PLL_RSET);
    i=0;
    while(i++<time_dly){};

    // ------------------------------------------------------------ 
    // Reset the PHY A by setting POR high for 10uS.
    SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_A_POR);
    i=0;
    while(i++<time_dly){};
    // Set POR to the PHY high

    CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_A_POR);
    i=0;
    while(i++<time_dly){};
    
    SET_PERIPHS_REG_BITS(PREI_ARC_INTR0_MASK,USB_PHY_B_INTR_BIT);

//#else
    /* Reset USB PHY B */
    SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_B_AHB_RSET);
    i=0;
    while(i++<time_dly){};
    CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_B_AHB_RSET);
    i=0;
    while(i++<time_dly){};
    SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_B_CLK_RSET);
    i=0;
    while(i++<time_dly){};
    CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_B_CLK_RSET);
    i=0;
    while(i++<time_dly){};
    SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_B_PLL_RSET);
    i=0;
    while(i++<time_dly){};
    CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_B_PLL_RSET);
    i=0;
    while(i++<time_dly){};

    // ------------------------------------------------------------ 
    // Reset the PHY B by setting POR high for 10uS.
    SET_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_B_POR);
    i=0;
    while(i++<time_dly){};

    // Set POR to the PHY high
    CLEAR_PERIPHS_REG_BITS(PREI_USB_PHY_REG, PREI_USB_PHY_B_POR);
    i=0;
    while(i++<time_dly){};

    SET_PERIPHS_REG_BITS(PREI_ARC_INTR0_MASK,USB_PHY_A_INTR_BIT);
	
//#endif

    
	//To optimize transfers, the AHB Bridge will automatically request a certain number of bytes 
	// from the DDR controller. The number of bytes to request depends on the AHB master characteristics. 
	// These bits control the number of bytes to be read from the DDR for a single AHB read request:
	// 00 = 32 bytes
	// 01 = 64 bytes
	// 10 = 96 bytes
	// 11 = 128 bytes
	//*USB default RD cache is 128 bytes. But in this config, WIFI dongle and Flash disk write is not stable.
	//*We should workaround change to 32 bytes. Perhaps performance will be lower.
	//CLEAR_ISA_REG_MASK(AHB_BRIDGE_CTRL0,(3 << AHB_BRIDGE_USB0_RD_CACHE_BIT));
	//CLEAR_ISA_REG_MASK(AHB_BRIDGE_CTRL0,(3 << AHB_BRIDGE_USB1_RD_CACHE_BIT));
  

}
#if 0

#define PERI_BASE_ADDR               0x01200000

#define WRITE_PERI_REG(reg, val) *(volatile unsigned *)(PERI_BASE_ADDR + (reg)) = (val)
#define READ_PERI_REG(reg) (readw(PERI_BASE_ADDR + (reg)))

#define SET_PERI_REG_MASK(reg, mask)   WRITE_PERI_REG(reg, (READ_PERI_REG(reg)|(mask)))
#define CLEAR_PERI_REG_MASK(reg, mask) WRITE_PERI_REG(reg, (READ_PERI_REG(reg)&(~mask)))
#define PREG_GPIOC_OE           (0x158)
#define PREG_GPIOC_OUT_LEVEL    (0x15c)
#define PREG_GPIOC_IN_LEVEL     (0x160)

#define DC_IVDC_REG 0x47
#define DC_CTRL_REG 0x48

typedef enum{	
	GPIOC_0 = 0,
	GPIOC_1,
	GPIOC_2,
	GPIOC_3,
	GPIOC_4,
	GPIOC_5,
	GPIOC_6,
	GPIOC_7,
	GPIOC_8,
	GPIOC_9,
	GPIOC_10,
	GPIOC_11,
	GPIOC_12,
	GPIOC_13,
	GPIOC_14,
	GPIOC_15,
	GPIOC_16,
	GPIOC_17,
	GPIOC_18,
	GPIOC_19,
	GPIOC_20
}PREG_GPIOC_Pad_t;



static void
flip_peri_bit(unsigned reg, unsigned mask, unsigned high)
{
    if(high) {
        SET_PERI_REG_MASK(reg, mask);
    } else {
        CLEAR_PERI_REG_MASK(reg, mask);
    }
}

/* GPIOC Group */
void gpioc_ctrl_mode(unsigned pin, int mode)
{
    flip_peri_bit(PREG_GPIOC_OE, 1<<pin, mode ); /* 0 as output;1 as input */
}

void gpioc_set_level(unsigned pin, int level)
{
    flip_peri_bit(PREG_GPIOC_OUT_LEVEL, 1<<pin, level); /* send out level */
}
#endif

/**
 * Control USB VBus Power by GPIO Line.
 * bsp_init() must be called first.
 * @param [in] is_power_on TRUE of Power on VBus, FALSE of Power off VBus
 */
void
bsp_set_dwc_chip_0_gpio_power(char is_power_on)
{
  #if 0
	if(is_power_on)
	{
		gpioc_ctrl_mode(GPIOC_10, 0);
		gpioc_set_level(GPIOC_10, 0);
	}
	else
	{
		gpioc_ctrl_mode(GPIOC_10, 0);
		gpioc_set_level(GPIOC_10, 1);
	}
  #endif
#if 0

    int padindex,ret;
    char *pad_name = CONFIG_USB_POWER_PAD;
  
    padindex = search_pad(pad_name);
    ret = switch_padsig(padindex, MODULE_GPIO, 0, 1);
    if(ret)
        return -1;
    if(is_power_on)    
        ctrl_padlevel(padindex, 0, 0);
    else
        ctrl_padlevel(padindex, 0, 1);
#endif /* 0 */
  
}

