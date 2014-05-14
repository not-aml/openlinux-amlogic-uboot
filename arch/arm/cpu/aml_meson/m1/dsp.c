
#include "asm/arch-m1/firm/regs.h"
#include "asm/arch-m1/firm/io.h"



#define S_1K					(1024)
#define S_1M					(S_1K*S_1K)

#define AUDIO_DSP_MEM_SIZE		 S_1M
#define AUDIO_DSP_START_PHY_ADDR 0xa4000000
#define AUDIO_DSP_START_ADDR	AUDIO_DSP_START_PHY_ADDR//((SYS_MEM_START+SYS_MEM_SIZE)-AUDIO_DSP_MEM_SIZE)
#define AUDIO_DSP_END_ADDR		((AUDIO_DSP_START_ADDR+AUDIO_DSP_MEM_SIZE))
#define REG_MEM_SIZE					(S_1K*4)




#define DSP_REG_OFFSET	(AUDIO_DSP_START_ADDR+ AUDIO_DSP_MEM_SIZE-REG_MEM_SIZE)
#define DSP_REG_END			(AUDIO_DSP_START_ADDR+ AUDIO_DSP_MEM_SIZE-4)
#define DSP_REG(x)			(DSP_REG_OFFSET | ((x)<<5))

#define DSP_ARM_REF_CLK_VAL                DSP_REG(16)

#define DSP_WD(reg,val)	({(*((volatile unsigned long *)(reg)))=val;})


#define DSP_MEM_START  		DSP_REG(7)
#define DSP_MEM_END 	 		DSP_REG(8)


#define DSP_STACK_START   DSP_REG(3)
#define DSP_STACK_END   	DSP_REG(4)
#define DSP_GP_STACK_START   DSP_REG(5)
#define DSP_GP_STACK_END   	DSP_REG(6)

int enable_dsp()
{

	unsigned long clk;
	
	clk=READ_CBUS_REG_BITS(PREG_CTLREG0_ADDR,4,5);
	clk=clk*1000*1000;
	printf("clk=%ld\n",clk);
	DSP_WD(DSP_MEM_START,0xa3020000);
	DSP_WD(DSP_MEM_END,0xa3120000);

	DSP_WD(DSP_STACK_START,0xa3000000);
	DSP_WD(DSP_STACK_END,0xa3010000);

	DSP_WD(DSP_GP_STACK_START,0xa3010000);
	DSP_WD(DSP_GP_STACK_END,0xa3020000);

	
    CLEAR_MPEG_REG_MASK(AUD_ARC_CTL, (0xfff << 4));
 //   SET_MPEG_REG_MASK(SDRAM_CTL0,1);//arc mapping to ddr memory
    SET_MPEG_REG_MASK(AUD_ARC_CTL, ((AUDIO_DSP_START_PHY_ADDR)>> 20) << 4);


			SET_MPEG_REG_MASK(AUD_ARC_CTL, 1);
		    CLEAR_MPEG_REG_MASK(AUD_ARC_CTL, 1);

#define RESET_AUD_ARC	(1<<13)
 SET_MPEG_REG_MASK(RESET2_REGISTER, RESET_AUD_ARC);
 SET_MPEG_REG_MASK(RESET2_REGISTER, RESET_AUD_ARC);
  SET_MPEG_REG_MASK(RESET2_REGISTER, RESET_AUD_ARC);
		    //clk=clk_get_sys("a9_clk", NULL);
		DSP_WD(DSP_ARM_REF_CLK_VAL, clk);
	flush_cache(0x84000000, S_1M);
}



