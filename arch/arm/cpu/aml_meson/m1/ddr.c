//#include "pctl.h"
//#include "c_arc_pointer_reg.h"
#include <asm/arch/am_regs.h>
#include <asm/arch/am_reg_addr.h>
#include <asm/arch/pctl.h>
#include <asm/arch/romboot.h>
#include <memtest.h>
#include <config.h>
#include <common.h>
char *num2hexstr(unsigned num,unsigned len);
static void delayusec(unsigned long delay)
{
	unsigned usec;
	while(delay>0){
		usec=delay>50000?50000:delay;
		WRITE_CBUS_REG(ISA_TIMERD,usec);
		while((READ_CBUS_REG(ISA_TIMERD)&0xffff)>0);
		delay-=usec;
	}
}
#define DDR_16BIT_LOW  0x300
#define DDR_32BIT	   0xf00
T_ROM_BOOT_RETURN_INFO * romboot_info=(T_ROM_BOOT_RETURN_INFO *)C_ROM_BOOT_DEBUG;
#define JTAG_ENABLE 0x51


#define APB_Wr(addr, data) WRITE_APB_REG(addr,data)
#define APB_Rd(addr) READ_APB_REG(addr)

#define NOP_CMD   0
#define PREA_CMD   1
#define REF_CMD   2
#define MRS_CMD   3
#define ZQ_SHORT_CMD  4
#define ZQ_LONG_CMD   5
#define SFT_RESET_CMD  6
#if 1
static void load_mcmd(unsigned val)
{
	APB_Wr(PCTL_MCMD_ADDR, val);
	while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 );
}
#define load_nop()  load_mcmd((1<<31)|(1<<20)|NOP_CMD)
#define load_prea()  	load_mcmd((1<<31)|(1<<20)|PREA_CMD)
#define load_ref()  	load_mcmd((1<<31)|(1<<20)|REF_CMD)

#define load_mrs(a,b)	load_mcmd((1 << 31) | \
								  (8 << 24) | \
								  (1 << 20) | \
							(a << 17) | \
						   (b << 4) | \
								   MRS_CMD )
#else
static inline void load_nop(void)
{
	APB_Wr(PCTL_MCMD_ADDR, (1 << 31) |
                           (1 << 20) |   //rank select
                          NOP_CMD );
  	while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 );
}

static inline void load_prea(void)
{
  	APB_Wr(PCTL_MCMD_ADDR, (1 << 31) |
    	                   (1 << 20) |   //rank select
                         PREA_CMD );
  	while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 );
}

static inline void load_mrs(int mrs_num, int mrs_value)
{
  	APB_Wr(PCTL_MCMD_ADDR, (1 << 31) |
                           (8 << 24) |	 //cmd_add_del at least 200 cycles 
                           (1 << 20) |   //rank select
                     (mrs_num << 17) |
                    (mrs_value << 4) |
                             MRS_CMD );
  	while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 );
}

static inline void load_ref(void )
{
	APB_Wr(PCTL_MCMD_ADDR, (1 << 31) |
                           (1 << 20) |   //rank select
                            REF_CMD );
    while ( APB_Rd(PCTL_MCMD_ADDR) & 0x80000000 );
}
#endif
void init_pctl(struct ddr_timing_set * timing_reg)
{
    int mrs0_value;
    int mrs1_value;
    int mrs2_value;
    int mrs3_value;
    *(volatile unsigned long *)(0xc1104408) = 1<<3; // reset mmc

    mrs0_value =  (0 << 12 ) |              //0 fast, 1 slow.
                  (timing_reg->t_wr <<9 ) |  //wr cycles.
                  (0  << 8 )   |            //DLL reset.
                  (0  << 7)    |            //0= Normal 1=Test.
                  (timing_reg->cl <<4) |     //cas latency.
                  (0 << 3) |                //burst type,  0:sequential 1 Interleave.
                  2 ;                       //burst length  : 010 : 4,  011: 8.

    mrs1_value =  (0 << 12 )  |             // qoff 0=op buf enabled : 1=op buf disabled  (A12)
                  (0 << 11 )  |             // rdqs enable 0=disabled : 1=enabled         (A11)
                  (0 << 10 )  |             // DQSN enable 1=disabled : 0=enabled         (A10)
                  (0 << 7  )  |             // ocd_program;      // 0, 1, 2, 3, or 7      (A9-A7)
                  (timing_reg->t_al << 3) |  //additive_latency; // 0 - 4                  (A5-A3)
				  ( 0 << 6)   |
                  ( 1 << 2)   |             //rtt_nominal;      // 50, 75, or 150 ohms    (A6:A2)
                  ( 0 << 1 )  |             //ocd_impedence;    // 0=full : 1=reduced strength (A1)
                  ( 0 << 0 ) ;              // dll_enable;       // 0=enable : 1=disable       (A0)
    mrs2_value = 0;
    mrs3_value = 0;

    //if ((bond_ctrl & 0x100) != 0) { //bond_ctrl0 1 = 16bits mode. 0 = 32bits mode.
        // 16 bits, enable byte lane 0 1.
        
//    }

    //write memory timing registers
    APB_Wr(PCTL_TOGCNT1U_ADDR, 		timing_reg->t_1us_pck);
    APB_Wr(PCTL_TINIT_ADDR, 		timing_reg->t_init_us);
    APB_Wr(PCTL_TOGCNT100N_ADDR, 	timing_reg->t_100ns_pck);
    APB_Wr(PCTL_TREFI_ADDR, 		timing_reg->t_refi_100ns);

    APB_Wr(PCTL_TRSTH_ADDR, 0);       // 0 for ddr2
    while (!(APB_Rd(PCTL_POWSTAT_ADDR) & 2)) {} // wait for dll lock

    APB_Wr(PCTL_POWCTL_ADDR, 1);            // start memory power up sequence
    while (!(APB_Rd(PCTL_POWSTAT_ADDR) & 1)) {} // wait for memory power up

    APB_Wr(PCTL_ODTCFG_ADDR, 8);         //configure ODT

    //configure DDR2 SDRAM parameter.
    APB_Wr(PCTL_TMRD_ADDR, 	timing_reg->t_mrd);
    APB_Wr(PCTL_TRFC_ADDR, 	timing_reg->t_rfc);
    APB_Wr(PCTL_TRP_ADDR, 	timing_reg->t_rp);
    APB_Wr(PCTL_TAL_ADDR,  	timing_reg->t_al);
    APB_Wr(PCTL_TCWL_ADDR,  timing_reg->cl-1 + timing_reg->t_al);
    APB_Wr(PCTL_TCL_ADDR, 	timing_reg->cl);
    APB_Wr(PCTL_TRAS_ADDR, 	timing_reg->t_ras);
    APB_Wr(PCTL_TRC_ADDR, 	timing_reg->t_rc);
    APB_Wr(PCTL_TRCD_ADDR, 	timing_reg->t_rcd);
    APB_Wr(PCTL_TRRD_ADDR, 	timing_reg->t_rrd);
    APB_Wr(PCTL_TRTP_ADDR, 	timing_reg->t_rtp);
    APB_Wr(PCTL_TWR_ADDR, 	timing_reg->t_wr);
    APB_Wr(PCTL_TWTR_ADDR, 	timing_reg->t_wtr);
    APB_Wr(PCTL_TEXSR_ADDR, timing_reg->t_exsr);
    APB_Wr(PCTL_TXP_ADDR, 	timing_reg->t_xp);
    APB_Wr(PCTL_TDQS_ADDR, 	timing_reg->t_dqs);
    //configure the PCTL for DDR2 SDRAM burst length = 4
    APB_Wr(PCTL_MCFG_ADDR, 0 |     // burst length = 4
                    (0 << 2) |     // bl8int_en.   enable bl8 interrupt function.
                     (1 <<18));    // tFAW.

    // Don't do it for gates simulation.
	APB_Wr(PCTL_ZQCR_ADDR,   (1 << 24) | 0x33dd);

    // initialize SDRAM
    load_nop();
    load_prea();
    load_mrs(2, mrs2_value);
    load_mrs(3, mrs3_value);
    mrs1_value = mrs1_value & 0xfffffffe; //dll enable
    load_mrs(1, mrs1_value);
    mrs0_value = mrs0_value | (1 << 8);    // dll reset.
    load_mrs(0, mrs0_value);
    load_prea();
    load_ref();
    load_ref();
    mrs0_value = mrs0_value & (~(1 << 8));    // dll reset.
    load_mrs(0, mrs0_value);
    mrs1_value = (mrs1_value & (~(7 << 7))) | (7 << 7 );  //OCD default.
    load_mrs(1, mrs1_value);
    mrs1_value = (mrs1_value & (~(7 << 7))) | (0 << 7 );  // ocd_exit;
    load_mrs(1, mrs1_value);
}

int ddr_phy_data_training(void)
{
    volatile int  data_temp;

    APB_Wr(PCTL_DTUWD0_ADDR, 0xdd22ee11);
    APB_Wr(PCTL_DTUWD1_ADDR, 0x7788bb44);
    APB_Wr(PCTL_DTUWD2_ADDR, 0xdd22ee11);
    APB_Wr(PCTL_DTUWD3_ADDR, 0x7788bb44);
#if 1
    APB_Wr(PCTL_DTUWACTL_ADDR, 0x300 |          // col addr
                               (0x7<<10) |      // bank addr
                               (0x1fff <<13) |  // row addr
                               (0 <<30 ));      // rank addr
    APB_Wr(PCTL_DTURACTL_ADDR, 0x300 |          // col addr
                               (0x7<<10) |      // bank addr
                               (0x1fff <<13) |  // row addr
                               (0 <<30 ));      // rank addr
#else
    APB_Wr(PCTL_DTUWACTL_ADDR, 0);      // rank addr
    APB_Wr(PCTL_DTURACTL_ADDR, 0);      // rank addr
#endif
    APB_Wr(PCTL_RSLR0_ADDR, (0 << 0) |          // system additional latency.
                            (0 << 3) |
                            (0 << 6) |
                            (0 << 9) );
#if 0

    APB_Wr(PCTL_RDGR0_ADDR, (0 << 0) |          // DQS GATing phase.
                            (0 << 2) |
                            (0 << 4) |
                            (0 << 6) );

    APB_Wr(PCTL_DLLCR0_ADDR, (APB_Rd(PCTL_DLLCR0_ADDR) & 0xfffc3fff) |
                             (3 << 14 ));
 
    APB_Wr(PCTL_DLLCR1_ADDR, (APB_Rd(PCTL_DLLCR1_ADDR) & 0xfffc3fff) |
                             (3 << 14 ));
 
    APB_Wr(PCTL_DLLCR2_ADDR, (APB_Rd(PCTL_DLLCR2_ADDR) & 0xfffc3fff) |
                             (3 << 14 ));
 
    APB_Wr(PCTL_DLLCR3_ADDR, (APB_Rd(PCTL_DLLCR3_ADDR) & 0xfffc3fff) |
                             (3 << 14 ));
 
    APB_Wr(PCTL_DQSTR_ADDR,  (0 << 0 ) |
                             (0 << 3 ) |
                             (0 << 6 ) |
                             (0 << 9 ));
 
    APB_Wr(PCTL_DQSNTR_ADDR, (0 << 0 ) |
                             (0 << 3 ) |
                             (0 << 6 ) |
                             (0 << 9 ));
 
    APB_Wr(PCTL_DQTR0_ADDR,  (0 << 0 ) |
                             (0 << 4 ) |
                             (0 << 8 ) |
                             (0 << 12 ) |
                             (0 << 16) |
                             (0 << 20) |
                             (0 << 24) |
                             (0 << 28) );
 
    APB_Wr(PCTL_DQTR1_ADDR,  (0 << 0 ) |
                             (0 << 4 ) |
                             (0 << 8 ) |
                             (0 << 12 ) |
                             (0 << 16) |
                             (0 << 20) |
                             (0 << 24) |
                             (0 << 28) );
 
    APB_Wr(PCTL_DQTR2_ADDR,  (0 << 0 ) |
                             (0 << 4 ) |
                             (0 << 8 ) |
                             (0 << 12 ) |
                             (0 << 16) |
                             (0 << 20) |
                             (0 << 24) |
                             (0 << 28) );
 
    APB_Wr(PCTL_DQTR3_ADDR,  (0 << 0 ) |
                             (0 << 4 ) |
                             (0 << 8 ) |
                             (0 << 12 ) |
                             (0 << 16) |
                             (0 << 20) |
                             (0 << 24) |
                             (0 << 28) );
#endif
    // hardware build in  data training.
    APB_Wr(PCTL_PHYCR_ADDR, APB_Rd(PCTL_PHYCR_ADDR) | (1<<31));
 
    APB_Wr(PCTL_SCTL_ADDR, 1); // init: 0, cfg: 1, go: 2, sleep: 3, wakeup: 4         rddata = 1;        // init: 0, cfg: 1, cfg_req: 2, access: 3, access_req: 4, low_power: 5, low_power_enter_req: 6, low_power_exit_req: 7
    while ((APB_Rd(PCTL_STAT_ADDR) & 0x7 ) != 1 ) {}
 
    APB_Wr(PCTL_SCTL_ADDR, 2); // init: 0, cfg: 1, go: 2, sleep: 3, wakeup: 4
    while ((APB_Rd(PCTL_STAT_ADDR) & 0x7 ) != 3 ) {}
    while (APB_Rd(PCTL_PHYCR_ADDR) & 0x80000000 ); // waiting the data trainning finish.
    data_temp = APB_Rd(PCTL_PHYSR_ADDR)& 0x00140000;//change from 0x340000 to 0x140000 , wont test bit21
    return data_temp;
}
#if 0
static unsigned char action[8]={0xf,0,0xf,1,0xf,4,0xf,0xf};
static unsigned char next_stat[5]={0,1,3,5,3};
void switch_to_init()
{
	unsigned stat,t,a;
	while(stat=APB_Rd(PCTL_STAT_ADDR)){
		a=action[stat];
		if(a>4)
			continue;
#ifdef AML_DEBUG_ROM

		serial_put_num(stat);
		serial_puts("\n");
#endif
		t=next_stat[a];
		APB_Wr(PCTL_SCTL_ADDR, t);
		while(APB_Rd(PCTL_STAT_ADDR)!=t);
	};
}
#endif
unsigned initial_ddr (struct ddr_timing_set * timing_reg,unsigned lane)
{
	int i;
    


#ifdef AML_DEBUG_ROM
    serial_puts("\nddr pll=");
    serial_put_hex(__hw_setting.ddr_pll_cntl);
    serial_puts("size=");
#endif
    APB_Wr(PCTL_IOCR_ADDR, (APB_Rd(PCTL_IOCR_ADDR) & 0xffff00ff) | lane);
//    if(lane==0x300)
//    {
//    	APB_Wr(PCTL_IOCR_ADDR, (APB_Rd(PCTL_IOCR_ADDR) & 0xffff00ff) | lane);
//    }else{
//
//    }


    delayusec(100000);//delay 10ms
    init_pctl(timing_reg);
    delayusec(100000);//delay 10ms
    i=ddr_phy_data_training();
    if(lane==0x300)
    {
#ifdef AML_DEBUG_ROM
    	serial_puts("128M\n");
#endif

	    APB_Wr(MMC_DDR_CTRL,APB_Rd(MMC_DDR_CTRL)|(1<<7));

    }

	else{

#ifdef AML_DEBUG_ROM
		serial_puts("256M\n");
#endif

	    APB_Wr(MMC_DDR_CTRL,APB_Rd(MMC_DDR_CTRL)&(~(1<<7)));
	}

    return i;

}


void pll_initial(void) {
    
    delayusec(10000);//delay 10ms
//    writel((1<<22)|500000,P_WATCHDOG_TC);
	
	//set sys clk -- A9 CLK
	WRITE_CBUS_REG(HHI_SYS_PLL_CNTL,__hw_setting.sys_pll_cntl);
	WRITE_MPEG_REG(HHI_A9_CLK_CNTL, // A9 clk set to system clock/2
			(0 << 10) | // 0 - sys_pll_clk, 1 - audio_pll_clk
			(1 << 0 ) | // 1 - sys/audio pll clk, 0 - XTAL
			(1 << 4 ) | // APB_CLK_ENABLE
			(1 << 5 ) | // AT_CLK_ENABLE
			(0 << 2 ) | // div1
			(1 << 7 )); // Connect A9 to the PLL divider output
	romboot_info->a9_clk = __hw_setting.a9_clk;
	/**
	 * clk81 settings From kernel
	 */
	WRITE_MPEG_REG(HHI_OTHER_PLL_CNTL, __hw_setting.other_pll_cntl); // other PLL
	WRITE_MPEG_REG(HHI_MPEG_CLK_CNTL, // MPEG clk81 set to other/2
			__hw_setting.mpeg_clk_cntl);
	
	/*
	Demod PLL,1.2G for SATA/Ethernet
	*/
	CLEAR_CBUS_REG_MASK(HHI_DEMOD_PLL_CNTL,0xFFFFFFFF);
	SET_CBUS_REG_MASK(HHI_DEMOD_PLL_CNTL,__hw_setting.demod_pll400m_cntl);		
	/*Demod PLL,400M,300M,240M*/		
	CLEAR_CBUS_REG_MASK(HHI_DEMOD_PLL_CNTL3,0xFFFF0000);
  	SET_CBUS_REG_MASK(HHI_DEMOD_PLL_CNTL3,0x0C850000);//400M 300M 240M
    
	romboot_info->clk81 = __hw_setting.clk81;
	writel(__hw_setting.spi_setting,P_SPI_FLASH_CTRL);
	WRITE_CBUS_REG(HHI_DDR_PLL_CNTL,__hw_setting.ddr_pll_cntl);

    delayusec(100000);
    
    
	
#ifdef AML_DEBUG_ROM
	serial_init_with_clk(__hw_setting.clk81);
#endif
}
extern void wait(void);
static char buf[64]="0000";
const char str[]="0123456789abcdef";
char *num2hexstr(unsigned num,unsigned len)
{

	int i;
	for(i=0;i<len;i++)
	{
		buf[i]=str[(num>>((len-1-i)<<2))&0xf];
	}
	buf[len]=0;
	return buf;
}
void serial_put_hex(unsigned num)
{
	serial_puts(num2hexstr(num,8));
}
#ifdef AML_DEBUG_ROM

char *num2str(int num)
{

	int i;
	
	for(i=0;i<64;i++) buf[i]='0';
	for(i=7;i>=0 && num>0;i--)
	{
		buf[i]=str[num%10];
		num=num/10;
	}
	if(i==7)
	   i=6;
	buf[8]=0;
	return &buf[i+1];
}

void serial_put_num(int num)
{
	serial_puts(num2str(num));
}

#endif
#ifdef AML_DEBUG_ROM
extern unsigned char auto_run[4];
extern unsigned cur_char;
int get_next_char(void)
{
	int ch;
	if(cur_char<4)
	{
		ch=(int)auto_run[cur_char];
		cur_char++;
	}
	else{
		while(!serial_tstc());
		ch=serial_getc();
	}
	return ch;

}
void caculate_sum(void )
{
	int i;
	unsigned short sum=0;
	unsigned * magic;
	volatile unsigned short *buf=(volatile unsigned short*)AHB_SRAM_BASE;
	// Calculate sum
	for(i=0;i<0x1b0/2;i++)
	{
		sum^=buf[i];
	}

	for(i=256;i<READ_SIZE/2;i++)
	{
		sum^=buf[i];
	}
	buf[0x1b8/2]=sum;
	magic=(unsigned *)&buf[0x1b0/2];
//	magic[0]=MAGIC_WORD1;
//	magic[1]=MAGIC_WORD2;
}
int ch2d(int ch)
{
	if(ch>='0'&&ch<='9')
		return ch - '0';
	if(ch>='a'&&ch<='f')
		return ch - 'a' + 10;
	return 0x10;
}
void modify_setting(unsigned short addr,unsigned short val)
{
	unsigned short * p=(unsigned short *)&__hw_setting;
	p[addr]=val;
}
void display_setting(unsigned short addr)
{
	unsigned short * p=(unsigned short *)&__hw_setting;
	serial_puts("\n");
	serial_puts(num2hexstr(p[addr],4));
}

unsigned short get_short()
{
	int i,cha,d;
	unsigned short addr=0;
	for(i=0;i<4;i++)
	{
		cha=get_next_char();
		d=ch2d(cha);
		if(d<0x10)
		{
			addr|=d<<((3-i)<<2);
			serial_putc(cha);
		}
		else
			i--;
	}
	return addr;
}
#endif
static  void display_error(unsigned por_cfg,char * hint)
{
	serial_puts(hint);
	if(por_cfg)
	{
		serial_puts("fail");
		serial_put_hex(por_cfg);


	}else{
		serial_puts("pass");
	}
	serial_puts("\n");

}
int lowlevel_init(void)
{
   unsigned por_cfg=READ_CBUS_REG(ASSIST_POR_CONFIG);
   if(((por_cfg&POR_ROM_BOOT_ENABLE)==0))
   {
      // take me as a spi rom boot mode
      romboot_info->por_cfg=POR_ROM_BOOT_ENABLE | POR_INTL_SPI;
      romboot_info->boot_id=0;
   } 

   WRITE_CBUS_REG(PAD_PULL_UP_REG0,-1);
   WRITE_CBUS_REG(PAD_PULL_UP_REG1,-1);
   WRITE_CBUS_REG(PAD_PULL_UP_REG2,-1);
   WRITE_CBUS_REG(PAD_PULL_UP_REG3,-1);
   //Adjust 1us timer base
   WRITE_CBUS_REG_BITS(PREG_CTLREG0_ADDR,CONFIG_CRYSTAL_MHZ,4,5);


#ifndef AML_DEBUG_ROM

   pll_initial(); //running under default freq now . Before we fixed the PLL stable problem

   unsigned clk;
       if((readl(P_HHI_MPEG_CLK_CNTL)&(1<<8))==0)
   	{
   		clk=CONFIG_CRYSTAL_MHZ*1000000;
   	}else{
   		clk=romboot_info->clk81;
   	}
       unsigned memory_size;
       memory_size=__hw_setting.ddr.lane==0x300?0x8000000:0x10000000;
   serial_init_with_clk(clk);
   writel((1<<22)|500000,P_WATCHDOG_TC);
   do{
    unsigned ret1,ret2,ret3,ret4;
    ret1=ret2=ret3=ret4=0;
    por_cfg=0;
    ret1=initial_ddr(&__hw_setting.ddr,__hw_setting.ddr.lane);
    display_error(ret1,"DDr Init");
    
#ifdef CONFIG_MEM_TEST
    ret2=memTestDevice((volatile datum *)PHYS_MEMORY_START,memory_size);
    display_error(ret2,"TestDevice");
    por_cfg=ret2;
#endif
    ret3=memTestDataBus((volatile datum *) PHYS_MEMORY_START);
   display_error(ret3,"TestDataBus");
    ret4=memTestAddressBus((volatile datum *) PHYS_MEMORY_START,memory_size);
    
   display_error(ret4
		   ,"TestAddressBus");
    writel(0,P_WATCHDOG_RESET);
    por_cfg=ret1|ret2|ret3|ret4;
    }while(por_cfg!=0);
   return 0;

#else
    unsigned clk;
    if((readl(P_HHI_MPEG_CLK_CNTL)&(1<<8))==0)
	{
		clk=CONFIG_CRYSTAL_MHZ*1000000;
	}else{
		clk=romboot_info->clk81;
	}

   serial_init_with_clk(clk);
   int i;

	unsigned  addr;
	unsigned  val;
	cur_char=0;

	while(1)
	{
		serial_puts("\ncmd>");
		int ch;
		int cha;
		ch=get_next_char();
		serial_putc(ch);
		switch(ch)
		{
//		case 'i':
//		    switch_to_init();
//		    display_error(ddr_phy_data_training(),"\ntrain");
//		    break;
		case 's':
			serial_puts("\nA9Stop");
			wait();
			break;
		case 'A'://Cortex A9
			serial_puts("\nA9JTAG");
			writel(JTAG_ENABLE<<4,P_HHI_JTAG_CONFIG);

			break;
		case 'a'://arc625
			serial_puts("\nARC625 JTAG");
			writel(JTAG_ENABLE<<12|1,P_HHI_JTAG_CONFIG);
			break;
		case 'W'://Wifi ARC
			serial_puts("\nWifi JTAG");

            WRITE_CBUS_REG(HHI_DEMOD_PLL_CNTL, ((4 << 20)|(0<<16)|(0<<15)|(1<<9)|(48<<0)));
            WRITE_CBUS_REG(HHI_WIFI_CLK_CNTL, 0x1);
            WRITE_CBUS_REG(HHI_DEMOD_PLL_CNTL2, READ_CBUS_REG(HHI_DEMOD_PLL_CNTL2)&~((0x1f<<16)|(0x1f<<21)|(0x1f<<26))|((15<<16)|(21<<21)|(10<<26)));
            WRITE_CBUS_REG(HHI_DEMOD_PLL_CNTL3, READ_CBUS_REG(HHI_DEMOD_PLL_CNTL3)&~((0x1f<<16)|(0x1f<<21)|(0x1f<<26))|((5<<16)|(4<<21)|(3<<26)));
           // WRITE_CBUS_REG(PERIPHS_PIN_MUX_9, READ_CBUS_REG(PERIPHS_PIN_MUX_9)|(0x3fffffff));
            WRITE_CBUS_REG(PERIPHS_PIN_MUX_8, READ_CBUS_REG(PERIPHS_PIN_MUX_8)|(0xffff8000));
            writel(0x1,(volatile unsigned long *)0xc93200a0);
            writel(JTAG_ENABLE<<20|2,P_HHI_JTAG_CONFIG);
			break;
		case 'w'://quick Wifi ARC  easy			
		  serial_puts("\nquick Wifi JTAG ");			
		  writel(JTAG_ENABLE<<20|2,P_HHI_JTAG_CONFIG);			
		  break;					
		case 'M':
		   pll_initial(); //running under default freq now . Before we fixed the PLL stable problem
		   por_cfg=initial_ddr(&__hw_setting.ddr,__hw_setting.ddr.lane);
		   display_error(por_cfg,"\nTrainning");
		   pll_clk_list();
		   por_cfg = memTestDataBus((volatile datum *) PHYS_MEMORY_START);
		   display_error(por_cfg,"\nDataBus");
			por_cfg = memTestAddressBus((volatile datum *) PHYS_MEMORY_START,
					__hw_setting.ddr.lane==0x300?0x8000000:0x10000000);
			display_error(por_cfg,"\nAddr Bus");
			break;
        case 'T':
        	por_cfg=(unsigned)memTestDevice((volatile datum *)PHYS_MEMORY_START,__hw_setting.ddr.lane==0x300?0x8000000:0x10000000);
			display_error(por_cfg,"memDev");
			break;
		case 'S':
			serial_puts("\n3 chars:");
			for(i=0;i<3;i++)
			{
				ch=get_next_char();

				serial_putc(ch);
				auto_run[i]=ch&0xff;
			}
			auto_run[3]=0;
			caculate_sum();
			spi_program(AHB_SRAM_BASE,READ_SIZE);
		case 'r':
			setbits_le32(P_WATCHDOG_TC,1<<22);
			break;
		case 'd':
		case 'm':
			addr=get_short();

			if(ch=='d')
			{
				display_setting(addr);
				break;
			}
			val=get_short();

			modify_setting(addr,val);
			break;
		case 'e':
			spi_erase();
			break;

		}
//		serial_puts("\n");

	}
#endif
	return 1;

}


#if CONFIG_CRYSTAL_MHZ==24

struct romboot_boot_settings  __hw_setting ={
		.ddr={
                    .cl             =   6,
                    .t_faw          =  20,
                    .t_mrd          =   2,
                    .t_1us_pck      = 400,
                    .t_100ns_pck    =  40,
                    .t_init_us      = 511,
                    .t_ras          =  16,
                    .t_rc           =  22,
                    .t_rcd          =   6,
                    .t_refi_100ns   =  78,
                    .t_rfc          =  51,
                    .t_rp           =   6,
                    .t_rrd          =   4,
                    .t_rtp          =   3,
                    .t_wr           =   6,
                    .t_wtr          =   4,
                    .t_xp           =   2,
                    .t_xsrd         =   0,   // init to 0 so that if only one of them is defined, this is chosen
                    .t_xsnr         =   0,
                    .t_exsr         = 200,
                    .t_al           =   0,   // Additive Latency
                    .t_clr          =   6,   // cas_latency for DDR2 (nclk cycles)
                    .t_dqs          =   2,   // distance between data phases to different ranks
                    .lane			=
#if PHYS_MEMORY_SIZE==0x8000000
		   DDR_16BIT_LOW,
#endif
#if PHYS_MEMORY_SIZE==0x10000000
		   DDR_32BIT,
#endif
				},

				.ddr_pll_cntl=0x110221,//390
				.sys_pll_cntl=0x44b,//900M
				.other_pll_cntl=0x0000042d,//0x2d*24/2=540M
				.mpeg_clk_cntl=	(1 << 12) |                     // select other PLL
						        ((3 - 1) << 0 ) |    // div1
						        (1 << 7 ) |                     // cntl_hi_mpeg_div_en, enable gating
						        (1 << 8 ),                    // Connect clk81 to the PLL divider output
				.demod_pll400m_cntl=(1<<9)	| //n 1200=xtal*m/n 
						(50<<0),	//m 50*24
				.clk81=180000000,//540/3=180M
				.a9_clk=900000000/2,
				.spi_setting=0xea949,

				.nfc_cfg=(((0)&0xf)<<10) | (0<<9) | (0<<5) | 5,
				.sdio_cmd_clk_divide=12,
				.sdio_time_short=(250*180000)/(2*(12)),


};
#endif

#if CONFIG_CRYSTAL_MHZ==25
struct romboot_boot_settings  __hw_setting ={
		.ddr={
                .cl             =   6,
                .t_faw          =  20,
                .t_mrd          =   2,
                .t_1us_pck      = 400,
                .t_100ns_pck    =  40,
                .t_init_us      = 200,
                .t_ras          =  16,
                .t_rc           =  24,
                .t_rcd          =   6,
                .t_refi_100ns   =  78,
                .t_rfc          =  51,
                .t_rp           =   6,
                .t_rrd          =   4,
                .t_rtp          =   3,
                .t_wr           =   6,
                .t_wtr          =   4,
                .t_xp           =   2,
                .t_xsrd         =   0,   // init to 0 so that if only one of them is defined, this is chosen
                .t_xsnr         =   0,
                .t_exsr         = 200,
                .t_al           =   0,   // Additive Latency
                .t_clr          =   6,   // cas_latency for DDR2 (nclk cycles)
                .t_dqs          =   2,   // distance between data phases to different ranks

                .lane			=
#if PHYS_MEMORY_SIZE==0x8000000
                		DDR_16BIT_LOW,
#endif
#if PHYS_MEMORY_SIZE==0x10000000
                		DDR_32BIT,
#endif
				},
//				unsigned short system_clk;
				.ddr_pll_cntl=0x0011065f,//OD=1<<1,25*32/OD=400Mhz
				.sys_pll_cntl=0x00000224,//25*48=1200Mhz
				//.other_pll_cntl=0x00a6c,//0x6c*25/5=540M
				.other_pll_cntl=0x00000a6c,//0x18*25=540M
				.mpeg_clk_cntl=	(1 << 12) |                     // select other PLL
						        ((3 - 1) << 0 ) |    // div1
						        (1 << 7 ) |                     // cntl_hi_mpeg_div_en, enable gating
						        (1 << 8 ),                     // Connect clk81 to the PLL divider output
				.demod_pll400m_cntl=(1<<9)	| //n 1200=xtal*m/n 
					        	(48<<0),	//m 48*25		        
				//.clk81=180000000,//540/3=180M
				.clk81=180000000,//540/3=200M
				.a9_clk=900000000/2,
				.spi_setting=0xea949,
//					unsigned spi_setting;
				.nfc_cfg=(((0)&0xf)<<10) | (0<<9) | (0<<5) | 5,
				.sdio_cmd_clk_divide=4,
				.sdio_time_short=(250*180000)/(2*(4+1)),
};
#endif
#if CONFIG_CRYSTAL_MHZ==26
struct romboot_boot_settings  __hw_setting ={
		.ddr={
				  .t_wr=6,
				  .cl=6,
				  .t_al=0,
				  .t_1us_pck=400,
				  .t_init_us=200,
				  .t_100ns_pck=40,
				  .t_refi_100ns=78,
				  .t_mrd=2,
				  .t_rfc=51,
				  .t_rp=6,
				  .t_ras=16,
				  .t_rc=22,
				  .t_rcd=6,
				  .t_rrd=4,
				  .t_rtp=3,
				  .t_wtr=4,
				  .t_exsr=200,
				  .t_xp=2,
				  .t_dqs=2,
				},
				.ddr_pll_cntl=0x0011087b,//399.7500
				.sys_pll_cntl=0x00000ae7,//1201.2Mhz
				.other_pll_cntl=0x00000853,//539.5M
				.mpeg_clk_cntl=	(1 << 12) |                     // select other PLL
								((3 - 1) << 0 ) |    // div1
								(1 << 7 ) |                     // cntl_hi_mpeg_div_en, enable gating
								(1 << 8 ),                    // Connect clk81 to the PLL divider output
				.demod_pll400m_cntl=(10<<9)	| //n 1200=xtal*m/n 
						(462<<0),	//m 462*26/10==1201.2;BAD.						
				.clk81=180000000,//540/3=180M
				.a9_clk=1200000000,
				.spi_setting=0xea949,
				.nfc_cfg=(((0)&0xf)<<10) | (0<<9) | (0<<5) | 5,
				.sdio_cmd_clk_divide=4,
				.sdio_time_short=(250*180000)/(2*(4+1)),
};
#endif
#if CONFIG_CRYSTAL_MHZ==27
struct romboot_boot_settings  __hw_setting ={
		.ddr={
				  .t_wr=6,
				  .cl=6,
				  .t_al=0,
				  .t_1us_pck=400,
				  .t_init_us=200,
				  .t_100ns_pck=40,
				  .t_refi_100ns=78,
				  .t_mrd=2,
				  .t_rfc=51,
				  .t_rp=6,
				  .t_ras=16,
				  .t_rc=22,
				  .t_rcd=6,
				  .t_rrd=4,
				  .t_rtp=3,
				  .t_wtr=4,
				  .t_exsr=200,
				  .t_xp=2,
				  .t_dqs=2,
				},
				.ddr_pll_cntl=0x00300ade,//399.6Mhz
				.sys_pll_cntl=0x000ade,//1200Mhz
				.other_pll_cntl=0x00428,//540Mhz
				.mpeg_clk_cntl=	(1 << 12) |                     // select other PLL
								((3 - 1) << 0 ) |    // div1
								(1 << 7 ) |                     // cntl_hi_mpeg_div_en, enable gating
								(1 << 8 ),                     // Connect clk81 to the PLL divider output
				.demod_pll400m_cntl=(9<<9)	| //n 1200=xtal*m/n 
						(400<<0),	//m 400*27/9==1200.						
				.clk81=180000000,//540/3=180M
				.a9_clk=1200000000/2,
				.spi_setting=0xea949,
//					unsigned spi_setting;
				.nfc_cfg=(((0)&0xf)<<10) | (0<<9) | (0<<5) | 5,
				.sdio_cmd_clk_divide=4,
				.sdio_time_short=(250*180000)/(2*(4+1)),
};
#endif


