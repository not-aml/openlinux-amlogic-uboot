#ifdef DDR_PCTL
#else
#define APB_Wr(addr, data) *(volatile unsigned long *) (0xc8000000 + addr )=data
#define APB_Rd(addr) *(volatile unsigned long *) (0xc8000000 + addr )

#define DDR_PCTL
#define PCTL_SCFG_ADDR 				0x000
#define PCTL_SCTL_ADDR 				0x004
#define PCTL_STAT_ADDR 				0x008
#define PCTL_MCMD_ADDR 				0x040
#define PCTL_POWCTL_ADDR 			0x044
#define PCTL_POWSTAT_ADDR 			0x048
#define PCTL_MCFG_ADDR 				0x080
#define PCTL_PPCFG_ADDR 			0x084
#define PCTL_MSTAT_ADDR 			0x088
#define PCTL_ODTCFG_ADDR 			0x08c
#define PCTL_DQSECFG_ADDR 			0x090
#define PCTL_DTUPDES_ADDR 			0x094
#define PCTL_DTUNA_ADDR 			0x098
#define PCTL_DTUNE_ADDR 			0x09c
#define PCTL_DTUPRD0_ADDR 			0x0a0
#define PCTL_DTUPRD1_ADDR 			0x0a4
#define PCTL_DTUPRD2_ADDR 			0x0a8
#define PCTL_DTUPRD3_ADDR 			0x0ac
#define PCTL_DTUAWDT_ADDR 			0x0b0
#define PCTL_TOGCNT1U_ADDR 			0x0c0
#define PCTL_TINIT_ADDR 			0x0c4
#define PCTL_TRSTH_ADDR 			0x0c8
#define PCTL_TOGCNT100N_ADDR 		0x0cc
#define PCTL_TREFI_ADDR 			0x0d0
#define PCTL_TMRD_ADDR 				0x0d4
#define PCTL_TRFC_ADDR 				0x0d8
#define PCTL_TRP_ADDR 				0x0dc
#define PCTL_TRTW_ADDR 				0x0e0 //aa
#define PCTL_TAL_ADDR 				0x0e4
#define PCTL_TCL_ADDR 				0x0e8
#define PCTL_TCWL_ADDR 				0x0ec
#define PCTL_TRAS_ADDR 				0x0f0
#define PCTL_TRC_ADDR 				0x0f4
#define PCTL_TRCD_ADDR 				0x0f8
#define PCTL_TRRD_ADDR 				0x0fc
#define PCTL_TRTP_ADDR 				0x100
#define PCTL_TWR_ADDR 				0x104
#define PCTL_TWTR_ADDR 				0x108
#define PCTL_TEXSR_ADDR 			0x10c
#define PCTL_TXP_ADDR 				0x110
#define PCTL_TXPDLL_ADDR 			0x114 //aa
#define PCTL_TZQCS_ADDR 			0x118 //aa
#define PCTL_TZQCSI_ADDR 			0x11c //aa
#define PCTL_TDQS_ADDR 				0x120 
#define PCTL_TCKSRE_ADDR 			0x124
#define PCTL_TCKSRX_ADDR 			0x128
#define PCTL_TCKE_ADDR 				0x12c
#define PCTL_TMOD_ADDR 				0x130
#define PCTL_TSRTL_ADDR 			0x134
#define PCTL_TZQCL_ADDR 			0x138
#define PCTL_DWLCFG0_ADDR 			0x170
#define PCTL_DWLCFG1_ADDR 			0x174
#define PCTL_DWLCFG2_ADDR 			0x178
#define PCTL_DWLCFG3_ADDR 			0x17c
#define PCTL_ECCCFG_ADDR 			0x180
#define PCTL_ECCTST_ADDR 			0x184
#define PCTL_ECCCLR_ADDR 			0x188
#define PCTL_ECCLOG_ADDR 			0x18c
#define PCTL_ADDRMAP_ADDR 			0x1c0
#define PCTL_IDDEC0_ADDR 			0x1c4
#define PCTL_IDDEC1_ADDR 			0x1c8
#define PCTL_DTUWACTL_ADDR 			0x200
#define PCTL_DTURACTL_ADDR 			0x204
#define PCTL_DTUCFG_ADDR 			0x208
#define PCTL_DTUECTL_ADDR 			0x20c
#define PCTL_DTUWD0_ADDR 			0x210
#define PCTL_DTUWD1_ADDR 			0x214
#define PCTL_DTUWD2_ADDR 			0x218
#define PCTL_DTUWD3_ADDR 			0x21c
#define PCTL_DTUWDM_ADDR 			0x220
#define PCTL_DTURD0_ADDR 			0x224
#define PCTL_DTURD1_ADDR 			0x228
#define PCTL_DTURD2_ADDR 			0x22c
#define PCTL_DTURD3_ADDR 			0x230
#define PCTL_DTULFSRWD_ADDR 		0x234
#define PCTL_DTULFSRRD_ADDR 		0x238
#define PCTL_DTUEAF_ADDR 			0x23c
#define PCTL_PHYCR_ADDR 			0x240
#define PCTL_PHYSR_ADDR 			0x244
#define PCTL_IOCR_ADDR 				0x248
#define PCTL_RSLR0_ADDR 			0x24c
#define PCTL_RSLR1_ADDR 			0x250
#define PCTL_RSLR2_ADDR 			0x254
#define PCTL_RSLR3_ADDR 			0x258
#define PCTL_RDGR0_ADDR 			0x25c
#define PCTL_RDGR1_ADDR 			0x260
#define PCTL_RDGR2_ADDR 			0x264
#define PCTL_RDGR3_ADDR 			0x268
#define PCTL_ZQCR_ADDR 				0x26c
#define PCTL_ZQSR_ADDR 				0x270
#define PCTL_DLLCR_ADDR 			0x280
#define PCTL_DLLCR0_ADDR 			0x284
#define PCTL_DLLCR1_ADDR 			0x288
#define PCTL_DLLCR2_ADDR 			0x28c
#define PCTL_DLLCR3_ADDR 			0x290
#define PCTL_DLLCR4_ADDR 			0x294
#define PCTL_DLLCR5_ADDR 			0x298
#define PCTL_DLLCR6_ADDR 			0x29c
#define PCTL_DLLCR7_ADDR 			0x2a0
#define PCTL_DLLCR8_ADDR 			0x2a4
#define PCTL_DLLCR9_ADDR 			0x2a8
#define PCTL_DQTR0_ADDR 			0x2c0
#define PCTL_DQTR1_ADDR 			0x2c4
#define PCTL_DQTR2_ADDR 			0x2c8
#define PCTL_DQTR3_ADDR 			0x2cc
#define PCTL_DQTR4_ADDR 			0x2d0
#define PCTL_DQTR5_ADDR 			0x2d4
#define PCTL_DQTR6_ADDR 			0x2d8
#define PCTL_DQTR7_ADDR 			0x2dc
#define PCTL_DQTR8_ADDR 			0x2e0
#define PCTL_DQSTR_ADDR 			0x2e4
#define PCTL_DQSNTR_ADDR 			0x2e8
#define PCTL_PHYPVTCFG_ADDR 		0x300
#define PCTL_PHYPVTSTAT_ADDR 		0x304
#define PCTL_PHYTUPDON_ADDR 		0x308
#define PCTL_PHYTUPDDLY_ADDR 		0x30c
#define PCTL_PVTTUPDON_ADDR 		0x310
#define PCTL_PVTTUPDDLY_ADDR 		0x314
#define PCTL_PHYPVTUPDI_ADDR 		0x318
#define PCTL_SCHCFG_ADDR 			0x380
#define PCTL_IPVR_ADDR 				0x3f8
#define PCTL_IPTR_ADDR 				0x3fc

#define  PCTL_STAT_INIT          0 
#define  PCTL_STAT_CONFIG        1 
#define  PCTL_STAT_ACCESS        3
#define  PCTL_STAT_LOW_POWER     5 

#define  SCTL_CMD_INIT          0 
#define  SCTL_CMD_CONFIG        1 
#define  SCTL_CMD_GO            2 
#define  SCTL_CMD_SLEEP         3 
#define  SCTL_CMD_WAKEUP        4 

#endif
