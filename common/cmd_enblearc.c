#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/firm/reg_addr.h>
static void enable_arc(void * arccode)
{
    spi_init();
    //copy ARM code
    memcpy(0x49008000,0x49000000,16*1024);
    writel((0x49008000>>14)&0xf,0xc810001c);
    //copy ARC code
    memcpy(0x49008000,arccode,16*1024);
    writel(0x1<<4,0xc8100020);
    writel(0x7fffffff,P_AO_RTI_STATUS_REG0);
    serial_puts("start up ARC\n");
    writel(0x51001,0xc8100030);
    writel(1,P_AO_RTI_STATUS_REG1);
    writel(0x1,0xc1109964);
    writel(0x0,0xc1109964);
    unsigned a,b;
    unsigned timer_base;
    a=b=0x7fffffff;
    printf("ARM is Live\n");
    timer_base=get_timer(0);
    do{
        a=readl(P_AO_RTI_STATUS_REG0);
        if((a&0x80000000)|| ((a==b)&&(get_timer(timer_base)<10000000)))
        {
            continue;
        }
        timer_base=get_timer(0);
        b=a;
        printf("ARM is Live%d\n",a);
        switch(a&0xffff)
        {
            case 0: 
                printf("ARM Exit Sleep Mode\n");
                break;
            
        }
       
        
    }while(a);
    
    
}
int do_enablearc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    char * ep;
    unsigned addr=(unsigned)simple_strtoul (argv[1], &ep, 16);
    printf("Boot ARC From 0x%x\n",addr);
    enable_arc(addr);
    __asm__ __volatile__("wfi");
	return 0;
}

U_BOOT_CMD(
	arc,	2,	1,	do_enablearc,
	"Enable ARC CPU",
	"addr\n"
	"    - enable ARC CPU"
);