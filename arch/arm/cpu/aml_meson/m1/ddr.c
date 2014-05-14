//#include "pctl.h"
//#include "c_arc_pointer_reg.h"
#include <asm/arch/am_regs.h>
#include <asm/arch/am_reg_addr.h>
#include <asm/arch/pctl.h>
#include <asm/arch/romboot.h>
//#include <memtest.h>
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


#define reg(a) (*(volatile unsigned*)(a))

#define DDR_PLL   0xc11041a0
#define SYS_PLL   0xc1104168
#define OTHER_PLL 0xc11041c0

static void get_num(unsigned num, unsigned char *n)
{
    *n = 0;
    *(n+1) = 0;
    *(n+2) = 0;
    *(n+3) = 0;
    
    while(num>=1000){
        ++(*(n+3));
        num -= 1000;
    }
    
    while(num>=100){
        ++(*(n+2));
        num -= 100;
    }
    
    while(num>=10){
        ++(*(n+1));
        num -= 10;
    }
    
    *n = num;
}

static unsigned get_clk(unsigned pll)
{
    unsigned m_val, m_clk, m, n, xd, od;
    
    m_val = reg(pll);
    m = m_val & 0x1ff;
    //n = (m_val >> 9) & 0x1f;
    //xd = (m_val >> 20) & 0x1ff;
    //od = 1 << ((m_val >> 16) & 3);
//    if(pll == OTHER_PLL)
//        od = (m_val >> 16) & 1;
//    else
        od = (m_val >> 16) & 3;
    //m_clk = (24 * m / n / xd) >> od;
    
    m_clk = (24 * m) >> od;
//    if(pll == DDR_PLL)
//        m_clk = (24 * m) >> od;
//    else if(pll == SYS_PLL)
//        m_clk = ((24 * m) >> od) >> 1;//n=2
//    else if(pll == OTHER_PLL)
//        m_clk = ((24 * m) >> od) >> 2;//n=4
    
    return m_clk;
}

static void plusc(unsigned pll)
{
    unsigned m_val;
    m_val = reg(pll) + 1;
    
    delayusec(10000);
    
    reg(pll) = m_val;
    
    delayusec(10000);
}

static void show_clk(unsigned pll)
{
    unsigned clk;
    unsigned char c[4];
    
    clk = get_clk(pll);
    serial_puts("\nclock[");
    serial_put_dword(pll);
    serial_puts("] is ");
    get_num(clk , c);
    serial_putc('0'+ c[3]);
    serial_putc('0'+ c[2]);
    serial_putc('0'+ c[1]);
    serial_putc('0'+ c[0]);
    serial_puts("MHz[");
    serial_put_dword(reg(pll));
    serial_puts("]\n");
}

void clk_setc(void)
{

    int n;
    int cha;
    int d10, d, s;
    unsigned m_pll;
    
    m_pll = DDR_PLL;
    
//    serial_puts("\nSelect type of PLL for test: 1, SYS PLL(A9). 2, OTHER PLL(clk81). 3, DDR PLL.\n");
//    s = serial_getc();
//    serial_putc(s);
//    if(s == '1'){
//        m_pll = SYS_PLL;
//    }
//    else if(s == '2'){
//        m_pll = OTHER_PLL;
//    }
//    else if(s == '3'){
//        m_pll = DDR_PLL;
//    }
//    else{
//        return;
//    }
    
    show_clk(m_pll);
    
    serial_puts("Enter the frequency to increase the level of(00-99):\n");
    cha = serial_getc();
    serial_putc(cha);
    d10 = (cha - '0') * 10;
    cha = serial_getc();
    serial_putc(cha);
    d = cha - '0';
    
    n = d10 + d;
    
    while(n) {        
        plusc(m_pll);
        --n;
    }
    
//    if(s == '1'){
//        __hw_setting.a9_clk += (6000000 * (d10 + d));
//        romboot_info->a9_clk = __hw_setting.a9_clk;
//    }
//    else if(s == '2'){
//        __hw_setting.clk81 += (3000000 * (d10 + d));
//        romboot_info->clk81 = __hw_setting.clk81;
//        serial_init_with_clk(__hw_setting.clk81);
//    }
//    
//    delayusec(10000);
    show_clk(m_pll);
}

#define DDR_16BIT_LOW  0x300
#define DDR_32BIT	   0xf00
T_ROM_BOOT_RETURN_INFO * romboot_info=(T_ROM_BOOT_RETURN_INFO *)C_ROM_BOOT_DEBUG;



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
    mrs2_value = (1<<3)|(1<<7);//0;
    mrs3_value = 0;

    //if ((bond_ctrl & 0x100) != 0) { //bond_ctrl0 1 = 16bits mode. 0 = 32bits mode.
        // 16 bits, enable byte lane 0 1.
        
//    }
    APB_Wr(PCTL_IOCR_ADDR, (APB_Rd(PCTL_IOCR_ADDR) & 0xffff00ff) | timing_reg->lane);
    //write memory timing registers
    APB_Wr(PCTL_TOGCNT1U_ADDR, 		timing_reg->t_1us_pck);
    APB_Wr(PCTL_TINIT_ADDR, 		timing_reg->t_init_us);
    APB_Wr(PCTL_TOGCNT100N_ADDR, 	timing_reg->t_100ns_pck);
    APB_Wr(PCTL_TREFI_ADDR, 		timing_reg->t_refi_100ns);

    APB_Wr(PCTL_TRSTH_ADDR, 0);       // 0 for ddr2
    while (!(APB_Rd(PCTL_POWSTAT_ADDR) & 2)) {} // wait for dll lock

    APB_Wr(PCTL_POWCTL_ADDR, 1);            // start memory power up sequence
    while (!(APB_Rd(PCTL_POWSTAT_ADDR) & 1)) {} // wait for memory power up

    //APB_Wr(PCTL_ODTCFG_ADDR, 8);         //configure ODT
    APB_Wr(PCTL_ODTCFG_ADDR, 2);         //configure ODT

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

void init_dmc (unsigned short lane)
{
//	serial_puts("Mem=");
//	serial_put_dword(lane);
//	serial_puts("\n");
	if (lane == 0x300) {
		APB_Wr(MMC_DDR_CTRL,APB_Rd(MMC_DDR_CTRL)|(1<<7));
	}
/*
3:2  r/w  2'b01 row size.    00: A0~A11;   01: A0~A12;  10: A0~A13.  11: A0~A14
1:0  r/w  2'b10 col size.    00: A0~A7;  01: A0~A8;   10: A0~A9; 11: A0~A9, A11.
*/
#if PHYS_MEMORY_SIZE==0x20000000
	APB_Wr(MMC_DDR_CTRL, 0xff225a);
#endif
	
//	else {
//		APB_Wr(MMC_DDR_CTRL,APB_Rd(MMC_DDR_CTRL)&(~(1<<7)));
//	}
}




void pll_initial(void) {
    
    delayusec(10000);//delay 10ms
	
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
	writel(__hw_setting.demod_pll400m_cntl,P_HHI_DEMOD_PLL_CNTL);
	/*Demod PLL,400M,300M,240M*/
	unsigned demod_pll_cntl3=(unsigned)P_HHI_DEMOD_PLL_CNTL3;
	writew(0x0C85,demod_pll_cntl3+2);

	romboot_info->clk81 = __hw_setting.clk81;
	writel(__hw_setting.spi_setting,P_SPI_FLASH_CTRL);
	WRITE_CBUS_REG(HHI_DDR_PLL_CNTL,__hw_setting.ddr_pll_cntl);

    delayusec(100000);
    
    
	
#ifdef AML_DEBUG_ROM
	serial_init_with_clk(__hw_setting.clk81);
#endif
}
static int start_ddr_config(void)
{
    unsigned timeout = 10000;
    APB_Rd(PCTL_SCTL_ADDR) = 0x1;
    while((APB_Rd(PCTL_STAT_ADDR) != 0x1) && timeout)
        --timeout;

    return timeout;
}

static int end_ddr_config(void)
{
    unsigned timeout = 10000;
    APB_Rd(PCTL_SCTL_ADDR) = 0x2;
    while((APB_Rd(PCTL_STAT_ADDR) != 0x3) && timeout)
        --timeout;

    return timeout;
}

static void dtu_enable(void)
{
    APB_Rd(PCTL_DTUECTL_ADDR) = 0x1;  // start wr/rd
}

static unsigned char check_dtu(void)
{
    unsigned char r_num = 0;
    volatile char *pr, *pw;
    unsigned char i;

    pr = (volatile char *)APB_REG_ADDR(PCTL_DTURD0_ADDR);
    pw = (volatile char *)APB_REG_ADDR(PCTL_DTUWD0_ADDR);

    for(i=0;i<16;i++)
    {
        if(*(pr+i) == *(pw+i))
            ++r_num;
    }

    return r_num;
}

static unsigned char get_best_dtu(unsigned char* p, unsigned char* best)
{
    unsigned char i;

    for(i=0;i<=9;i++)
    {
        if(*(p+i) + *(p+i+1) + *(p+i+2) == 48)
            goto next;
    }
    return 1;

next:

    for(i=0;i<=8;i++)
    {
        if(*(p+i) + *(p+i+1) + *(p+i+2) + *(p+i+3) == 64)
        {
            if(!i)
                *best = 2;
            else if(i == 8)
                *best = 9;
            else
            {
                if(*(p+i-1)>*(p+i+4))
                    *best = i + 1;
                else
                    *best = i + 2;
            }

            return 0;
        }
    }

    for(i=0;i<=9;i++)
    {
        if(*(p+i) + *(p+i+1) + *(p+i+2) == 48)
        {
            *best = i + 1;
            return 0;
        }
    }

    return 2;
}

static void set_result(unsigned char* res)
{
    APB_Rd(PCTL_RSLR0_ADDR) = ((res[0]>>2)&3) | (((res[1]>>2)&3)<<3) | (((res[2]>>2)&3)<<6) | (((res[3]>>2)&3)<<9);
    APB_Rd(PCTL_RDGR0_ADDR) = (res[0]&3)      | ((res[1]&3)     <<2) | ((res[2]&3)     <<4) | ((res[3]&3)<<6);
}

unsigned initial_ddr (struct ddr_timing_set * timing_reg,unsigned lane)
{
	unsigned char Tra[4];
	unsigned char chk[12];


	int i,j,k;

	//Start manual data trainning
	init_dmc(lane);

	for (k = 0; k < 4; k++) {

		for (i = 0; i < 3; i++) {
			for (j = 0; j < 4; j++) {
				init_pctl(timing_reg);

				if (!start_ddr_config()) {
					return 1;
				}

				// add for DTU
				APB_Wr(PCTL_DTUWD0_ADDR, 0xdd22ee11);
				APB_Wr(PCTL_DTUWD1_ADDR, 0x7788bb44);
				APB_Wr(PCTL_DTUWD2_ADDR, 0xdd22ee11);
				APB_Wr(PCTL_DTUWD3_ADDR, 0x7788bb44);
				APB_Wr(PCTL_DTUWACTL_ADDR, 0x300 | // col addr
						(0x7<<10) | // bank addr
						(0x1fff <<13) | // row addr
						(0 <<30 )); // rank addr
				APB_Wr(PCTL_DTURACTL_ADDR, 0x300 | // col addr
						(0x7<<10) | // bank addr
						(0x1fff <<13) | // row addr
						(0 <<30 )); // rank addr

				APB_Rd(PCTL_DTUCFG_ADDR) = (k << 10) | 0x01; // select byte lane, & enable DTU

				APB_Rd(PCTL_RSLR0_ADDR) = i | (i << 3) | (i << 6) | (i << 9);
				APB_Rd(PCTL_RDGR0_ADDR) = j | (j << 2) | (j << 4) | (j << 6);

				dtu_enable();

				if (!end_ddr_config()) {
					return 1;
				}
				chk[i * 4 + j] = check_dtu();
			}
		}

		if (get_best_dtu(chk, &Tra[k])) {
			Tra[k] = 0;
		}
	}


	init_pctl(timing_reg);
	//_invalidate_dcache();
	if (!start_ddr_config()) {

		return 1;
	}

	set_result(Tra);

	if (!end_ddr_config()) {

		return 1;
	}
	init_dmc(lane);
    return 0;

}


#if CONFIG_CRYSTAL_MHZ==24

struct romboot_boot_settings  __hw_setting ={
		.ddr={
                    .cl             =   6,
                    .t_faw          =  20,
                    .t_mrd          =   2,
                    .t_1us_pck      = 384,
                    .t_100ns_pck    =  39,
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
#if PHYS_MEMORY_SIZE>=0x10000000
		   DDR_32BIT,
#endif
				},

				.ddr_pll_cntl=0x110218,//288    //390
				.sys_pll_cntl=0x44b,//900M
				.other_pll_cntl=0x0000087d,//0x2d*24/2=540M
				.mpeg_clk_cntl=	(1 << 12) |                     // select other PLL
						        ((4- 1) << 0 ) |    // div1
						        (1 << 7 ) |                     // cntl_hi_mpeg_div_en, enable gating
						        (1 << 8 ),                    // Connect clk81 to the PLL divider output
				.demod_pll400m_cntl=(1<<9)	| //n 1200=xtal*m/n 
						(50<<0),	//m 50*24
				.clk81=187500000,//750/4=180M
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
#if PHYS_MEMORY_SIZE>=0x10000000
                		DDR_32BIT,
#endif
				},
//				unsigned short system_clk;
				.ddr_pll_cntl=0x0011065f,//OD=1<<1,25*32/OD=400Mhz
				.sys_pll_cntl=/*0x00000224*//*0x00000230*//*0x00000234*/0x00000238,//25*48=1200Mhz;;450,600,650,700
				//.other_pll_cntl=0x00a6c,//0x6c*25/5=540M
				.other_pll_cntl=0x0000021e,//0x18*25=540M
				.mpeg_clk_cntl=	(1 << 12) |                     // select other PLL
						        ((4 - 1) << 0 ) |    // div1
						        (1 << 7 ) |                     // cntl_hi_mpeg_div_en, enable gating
						        (1 << 8 ),                     // Connect clk81 to the PLL divider output
				.demod_pll400m_cntl=(1<<9)	| //n 1200=xtal*m/n 
					        	(48<<0),	//m 48*25		        
				//.clk81=180000000,//540/3=180M
				.clk81=187500000,//540/3=200M
				.a9_clk=1400000000/2,
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


