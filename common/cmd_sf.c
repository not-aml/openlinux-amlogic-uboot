/*
 * Command for accessing SPI flash.
 *
 * Copyright (C) 2008 Atmel Corporation
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <spi_flash.h>

#include <asm/io.h>

#ifndef CONFIG_SF_DEFAULT_SPEED
# define CONFIG_SF_DEFAULT_SPEED	1000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
# define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#endif

static struct spi_flash *flash;

static int do_spi_flash_probe(int argc, char *argv[])
{
	unsigned int bus = 0;
	unsigned int cs;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	char *endp;
	struct spi_flash *new;

	if (argc < 2)
		goto usage;

	cs = simple_strtoul(argv[1], &endp, 0);
	if (*argv[1] == 0 || (*endp != 0 && *endp != ':'))
		goto usage;
	if (*endp == ':') {
		if (endp[1] == 0)
			goto usage;

		bus = cs;
		cs = simple_strtoul(endp + 1, &endp, 0);
		if (*endp != 0)
			goto usage;
	}

	if (argc >= 3) {
		speed = simple_strtoul(argv[2], &endp, 0);
		if (*argv[2] == 0 || *endp != 0)
			goto usage;
	}
	if (argc >= 4) {
		mode = simple_strtoul(argv[3], &endp, 16);
		if (*argv[3] == 0 || *endp != 0)
			goto usage;
	}

	new = spi_flash_probe(bus, cs, speed, mode);
	if (!new) {
		printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
		return 1;
	}

	if (flash)
		spi_flash_free(flash);
	flash = new;

	printf("%u KiB %s at %u:%u is now current device\n",
			flash->size >> 10, flash->name, bus, cs);

	return 0;

usage:
	puts("Usage: sf probe [bus:]cs [hz] [mode]\n");
	return 1;
}

static int do_spi_flash_read_write(int argc, char *argv[])
{
	unsigned long addr;
	unsigned long offset;
	unsigned long len;
	void *buf;
	char *endp;
	int ret;

	if (argc < 4)
		goto usage;

	addr = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		goto usage;
	offset = simple_strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		goto usage;
	len = simple_strtoul(argv[3], &endp, 16);
	if (*argv[3] == 0 || *endp != 0)
		goto usage;

	buf = map_physmem(addr, len, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	if (strcmp(argv[0], "read") == 0)
		ret = spi_flash_read(flash, offset, len, buf);
	else
		ret = spi_flash_write(flash, offset, len, buf);

	unmap_physmem(buf, len);

	if (ret) {
		printf("SPI flash %s failed\n", argv[0]);
		return 1;
	}

	return 0;

usage:
	printf("Usage: sf %s addr offset len\n", argv[0]);
	return 1;
}

static int do_spi_flash_erase(int argc, char *argv[])
{
	unsigned long offset;
	unsigned long len;
	char *endp;
	int ret;

	if (argc < 3)
		goto usage;

	offset = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		goto usage;
	len = simple_strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		goto usage;

	ret = spi_flash_erase(flash, offset, len);
	if (ret) {
		printf("SPI flash %s failed\n", argv[0]);
		return 1;
	}

	return 0;

usage:
	puts("Usage: sf erase offset len\n");
	return 1;
}

static int do_spi_flash(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

	if (strcmp(cmd, "probe") == 0)
		return do_spi_flash_probe(argc - 1, argv + 1);

	/* The remaining commands require a selected device */
	if (!flash) {
		puts("No SPI flash selected. Please run `sf probe'\n");
		return 1;
	}

	if (strcmp(cmd, "read") == 0 || strcmp(cmd, "write") == 0)
		return do_spi_flash_read_write(argc - 1, argv + 1);
	if (strcmp(cmd, "erase") == 0)
		return do_spi_flash_erase(argc - 1, argv + 1);

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	sf,	5,	1,	do_spi_flash,
	"SPI flash sub-system",
	"probe [bus:]cs [hz] [mode]	- init flash device on given SPI bus\n"
	"				  and chip select\n"
	"sf read addr offset len 	- read `len' bytes starting at\n"
	"				  `offset' to memory at `addr'\n"
	"sf write addr offset len	- write `len' bytes from memory\n"
	"				  at `addr' to flash at `offset'\n"
	"sf erase offset len		- erase `len' bytes from `offset'"
);

#define creg(a) (*(volatile unsigned*)(0xc1100000 + ((a)<<2)))
#define reg(a) (*(volatile unsigned*)(a))

#define delay_us(a) udelay(a)
//static void delay_us(unsigned count)
//{
//    int i ;
//    for(; count > 0 ; count--) 
//      for(i=450; i > 0; i--) 
//        {;}
//}
/*
static int gmtest_end_check(void)
{
    int ch;
    printf("Press 'Q' to end test, another to continue.\n");
    ch = getch();
    //scanf("%d", &ch);
    if(ch == 'q' || ch == 'Q')
        return 1;
    else
        return 0;
}
*/
static void wifi_sdio_test(unsigned long n)
{
    //int n;
    
    /***Clear pinmux***/    
    reg(0xc11080bc) &= ~((1<<16) | (1<<22));reg(0xc11080d4) &= ~(1<<0);reg(0xc11080d8) &= ~(1<<0);//J20 GPIO_C0 WIFI/BT_WAKE
    reg(0xc11080dc) &= ~(1<<6);reg(0xc11080b8) &= ~((1<<16) | (1<<20));reg(0xc11080c8) &= ~(1<<27);//W12 GPIO_B7 SD_D3_C
    reg(0xc11080dc) &= ~(1<<7);reg(0xc11080b8) &= ~((1<<17) | (1<<21));//Y12 GPIO_B6 SD_D2_C
    reg(0xc11080dc) &= ~(1<<8);reg(0xc11080b8) &= ~((1<<18) | (1<<22));//W11 GPIO_B5 SD_D1_C
    reg(0xc11080dc) &= ~(1<<9);reg(0xc11080b8) &= ~((1<<19) | (1<<23));//Y11 GPIO_B4 SD_D0_C
    reg(0xc11080b8) &= ~(0xf<<8);reg(0xc11080c8) &= ~(1<<30);//W10 GPIO_B3 SD_CLK_C
    reg(0xc11080b8) &= ~(0xf<<12);reg(0xc11080c8) &= ~(1<<31);//Y10 GPIO_B2 SD_CMD_C
    
    /***Pull low***/
    creg(0x2010) &= ~(1<<0);//GPIOC0
    creg(0x200d) &= ~(0x3f<<21);//GPIOB2-B7
    
    /***Output enable***/
    creg(0x200c) &= ~(0x3f<<21);//GPIOB2-B7
    creg(0x200f) &= ~(1<<0);//GPIOC0
    
    delay_us(10000);
    
    //n = 3;
    
pstart:
    /***Pull high***/
    creg(0x2010) |= (1<<0);//GPIOC0
    creg(0x200d) |= (0x3f<<21);//GPIOB2-B7
    delay_us(10000);
    /***Pull low***/
    creg(0x2010) &= ~(1<<0);//GPIOC0
    creg(0x200d) &= ~(0x3f<<21);//GPIOB2-B7
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x3f<<21);//GPIOB2-B7
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x3f<<21);//GPIOB2-B7
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x1f<<21);//GPIOB2-B6
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x1f<<21);//GPIOB2-B6
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0xf<<21);//GPIOB2-B5
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0xf<<21);//GPIOB2-B5
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x7<<21);//GPIOB2-B4
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x7<<21);//GPIOB2-B4
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x3<<21);//GPIOB2-B3
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x3<<21);//GPIOB2-B3
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x1<<21);//GPIOB2
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x1<<21);//GPIOB2
    delay_us(30000);
    
    if(--n)
        goto pstart;
//    else{
//        if(gmtest_end_check()){
//            n = 3;
//            goto pstart;
//        }
//    }
}

static void i2c_test(unsigned long n)
{    
    /***Clear pinmux***/    
    reg(0xc11080b8) &= ~(0x3f<<0);reg(0xc11080c8) &= ~(3<<28);reg(0xc11080b4) &= ~(1<<31);//AA10 & AB10     GPIOB0(SCL) & GPIOB1(SDA)     HW_I2C_SDA & HW_I2C_SCL
    
    /***Pull low***/    
    creg(0x200d) &= ~(0x3<<19);//GPIOB0-B1
    
    /***Output enable***/
    creg(0x200c) &= ~(0x3<<19);//GPIOB0-B1
    
    delay_us(10000);
    
    do{    
        /***Pull high***/
        creg(0x200d) |= (0x3<<19);//GPIOB0-B1
        delay_us(10000);
        /***Pull low***/
        creg(0x200d) &= ~(0x3<<19);//GPIOB0-B1
        delay_us(10000);
        /***Pull high***/
        creg(0x200d) |= (0x1<<19);//GPIOB0
        delay_us(10000);
        /***Pull low***/
        creg(0x200d) &= ~(0x1<<19);//GPIOB0
        delay_us(30000);    
    }while(--n);
}

static void ir_test(unsigned long n)
{    
    /***Clear pinmux***/    
    reg(0xc11080c8) &= ~(1<<9);reg(0xc11080c4) &= ~(1<<31);//A14 GPIOE21 IR
    
    /***Pull low***/    
    creg(0x2016) &= ~(1<<21);//GPIOE21
    
    /***Output enable***/
    creg(0x2015) &= ~(1<<21);//GPIOE21
    
    delay_us(10000);
    
    do{    
        /***Pull high***/
        creg(0x2016) |= (1<<21);//GPIOE21
        delay_us(10000);
        /***Pull low***/
        creg(0x2016) &= ~(1<<21);//GPIOE21
        delay_us(30000);    
    }while(--n);
}

static void itu601_test(unsigned long n)
{    
    /***Clear pinmux***/    
    reg(0xc11080bc) &= ~(0x1ffffff<<0);reg(0xc11080b8) &= ~(0xf<<24);reg(0xc11080c0) &= ~(0x3<<26);reg(0xc11080d4) &= ~(0xfff<<1);reg(0xc11080d8) &= ~(0xfff<<1);//GPIO C1-C12
    
    /***Pull low***/    
    creg(0x2010) &= ~(0xfff<<1);//GPIO C1-C12
    
    /***Output enable***/
    creg(0x200f) &= ~(0xfff<<1);//GPIO C1-C12
    
    delay_us(10000);
    
    do{    
        /***Pull high***/
        creg(0x2010) |= (0xfff<<1);//GPIO C1-C12
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0xfff<<1);//GPIO C1-C12
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x7ff<<1);//GPIO C1-C11
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x7ff<<1);//GPIO C1-C11
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x3ff<<1);//GPIO C1-C10
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x3ff<<1);//GPIO C1-C10
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x1ff<<1);//GPIO C1-C9
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x1ff<<1);//GPIO C1-C9
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0xff<<1);//GPIO C1-C8
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0xff<<1);//GPIO C1-C8
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x7f<<1);//GPIO C1-C7
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x7f<<1);//GPIO C1-C7
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x3f<<1);//GPIO C1-C6
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x3f<<1);//GPIO C1-C6
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x1f<<1);//GPIO C1-C5
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x1f<<1);//GPIO C1-C5
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0xf<<1);//GPIO C1-C4
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0xf<<1);//GPIO C1-C4
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x7<<1);//GPIO C1-C3
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x7<<1);//GPIO C1-C3
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x3<<1);//GPIO C1-C2
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x3<<1);//GPIO C1-C2
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (1<<1);//GPIO C1
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(1<<1);//GPIO C1
        delay_us(10000);
        delay_us(30000);    
    }while(--n);
}

#define EIO_ID 0x44

static void eio_test(unsigned long n)
{
    unsigned char i;
    char buff;
    /***Clear pinmux***/    
    reg(0xc11080e0) &= ~(1<<31);reg(0xc11080c4) &= ~((1<<16)|(1<<22));reg(0xc11080d0) &= ~(1<<19);//GPIO D12
    
    reg(0xc110804c) &= ~(1<<10);
    reg(0xc1108048) &= ~(1<<10);
    delay_us(20000);
    reg(0xc110804c) |= (1<<10);
    delay_us(20000);
    
    hard_i2c_init(400);
    reg(0xc11080b8) &= ~(0x3f<<0);reg(0xc11080c8) &= ~(3<<28);reg(0xc11080b4) &= ~(1<<31);//AA10 & AB10     GPIOB0(SCL) & GPIOB1(SDA)     HW_I2C_SDA & HW_I2C_SCL
    reg(0xc11080b8) |= ((1<<2) | (1<<5));
    
    delay_us(10000);
    
    hard_i2c_write8(EIO_ID, 0x04, 0);
    hard_i2c_write8(EIO_ID, 0x05, 0);
    hard_i2c_write8(EIO_ID, 0x06, 0);
    hard_i2c_write8(EIO_ID, 0x0c, 0);
    hard_i2c_write8(EIO_ID, 0x0d, 0);
    hard_i2c_write8(EIO_ID, 0x0e, 0);
    delay_us(10000);
    
    do{
        buff = 0xff;
        
        hard_i2c_write8(EIO_ID, 0x04, 0xff);
        hard_i2c_write8(EIO_ID, 0x05, 0xff);
        hard_i2c_write8(EIO_ID, 0x06, 0xff);
        delay_us(10000);
        hard_i2c_write8(EIO_ID, 0x04, 0);
        hard_i2c_write8(EIO_ID, 0x05, 0);
        hard_i2c_write8(EIO_ID, 0x06, 0);
        delay_us(10000);
        
        for(i=0;i<7;i++){
            buff &= ~(1<<i);
            hard_i2c_write8(EIO_ID, 0x04, buff);
            hard_i2c_write8(EIO_ID, 0x05, buff);
            hard_i2c_write8(EIO_ID, 0x06, buff);
            delay_us(10000);
            hard_i2c_write8(EIO_ID, 0x04, 0);
            hard_i2c_write8(EIO_ID, 0x05, 0);
            hard_i2c_write8(EIO_ID, 0x06, 0);
            delay_us(10000);
        }
        
        delay_us(30000);
    }while(--n);
}

static void sth1_out32k(void)
{
    /***Clear pinmux***/    
    reg(0xc11080b0) &= ~(1<<16);reg(0xc11080b4) &= ~(1<<5);reg(0xc11080dc) &= ~(1<<1);reg(0xc11080e0) &= ~(1<<14);//GPIOA0
    
    /***Enable 32.768K***/
    reg(0xc11080d0) |= (1<<18);
}

static int do_gmtest(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd;
	
	char *endp;
	unsigned long loop;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	
	  if(argc > 2){
	      loop = simple_strtoul(argv[2], &endp, 10);
        if (*argv[1] == 0 || *endp != 0)
		        loop = 1000;
	  }
    
    if (strcmp(cmd, "0") == 0)
        sth1_out32k();
    else if (strcmp(cmd, "1") == 0)
        wifi_sdio_test(loop);
    else if (strcmp(cmd, "2") == 0)
        i2c_test(loop);
    else if (strcmp(cmd, "3") == 0)
        ir_test(loop);
    else if (strcmp(cmd, "4") == 0)
        itu601_test(loop);
    else if (strcmp(cmd, "5") == 0)
        eio_test(loop);
    else
        goto usage;

    return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	gmtest,	5,	1,	do_gmtest,
	"GPIO modules test",
	"0, STH1 32.768K output.\n"
	"1, wifi sdio test.\n"
	"2, I2C test.\n"
	"3, ir test.\n"
	"4, ITU601 test\n"
	"5, EIO test\n"
	"\n"
	""
);

#define TDATA32F 0xffffffff
#define TDATA32A 0xaaaaaaaa
#define TDATA325 0x55555555

static void ddr_write(void *buff, unsigned m_length)
{
    unsigned *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (unsigned *)buff;
    
    while(m_len)
    {
        for(j=0;j<32;j++)
        {
            if(m_len >= 128)
                n = 32;
            else
                n = m_len>>2;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        *(p+i) = TDATA32F;
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        *(p+i) = 0;
                        break;
                    case 16:
                    case 23:
                    case 31:                    
                        *(p+i) = TDATA32A;
                        break;
                    case 7:
                    case 15:
                    case 24:
                        *(p+i) = TDATA325;
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        *(p+i) = 1<<j;
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        *(p+i) = ~(1<<j);
                        break;
                }
            }
            
            if(m_len > 128)
            {
                m_len -= 128;
                p += 32;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;                
                break;
            }
        }
    }
}

static void ddr_read(void *buff, unsigned m_length)
{
    unsigned *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (unsigned *)buff;
    
    while(m_len)
    {
        for(j=0;j<32;j++)
        {
            if(m_len >= 128)
                n = 32;
            else
                n = m_len>>2;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        if(*(p+i) != TDATA32F)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA32F);
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        if(*(p+i) != 0)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), 0);
                        break;
                    case 16:
                    case 23:
                    case 31:                    
                        if(*(p+i) != TDATA32A)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA32A);
                        break;
                    case 7:
                    case 15:
                    case 24:
                        if(*(p+i) != TDATA325)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA325);
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        if(*(p+i) != 1<<j)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), 1<<j);
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        if(*(p+i) != ~(1<<j))
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), ~(1<<j));
                        break;
                }
            }
            
            if(m_len > 128)
            {
                m_len -= 128;
                p += 32;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}

static int do_ddr_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	//const char *cmd;
	char *endp;
	unsigned long loop;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	//cmd = argv[1];
	
	loop = simple_strtoul(argv[1], &endp, 10);
	 if (*argv[1] == 0 || *endp != 0)
		        loop = 3;
	
	do{
	    printf("Start writing...\n");
	    ddr_write((void *)0x88000000, 0x4000000);
	    printf("End write.\n");
	    printf("Start reading...\n");	    
	    ddr_read((void *)0x88000000, 0x4000000);
	    printf("End read.\n");
	    printf("Start reading...\n");	    
	    ddr_read((void *)0x88000000, 0x4000000);
	    printf("End read.\n");
	    printf("Start reading...\n");	    
	    ddr_read((void *)0x88000000, 0x4000000);
	    printf("End read.\n");
	  }while(--loop);
	  
	  return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	ddrtest,	5,	1,	do_ddr_test,
	"DDR test function",
	"ddrtest loop.\n"
);