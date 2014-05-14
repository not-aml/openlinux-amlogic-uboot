/* 
 * act8942 i2c interface
 * Copyright (C) 2011 Amlogic, Inc.
 *
 *
 * Author:  elvis yu<elvis.yu@amlogic.com>
 */  

#include <asm/arch/i2c.h>
#include <aml_i2c.h>
#include <act8942.h>

//#define __DBG__
#ifdef __DBG__
#define debug(fmt,args...) do { printf("[DEBUG]: FILE:%s:%d, FUNC:%s--- "fmt"\n",\
                                                     __FILE__,__LINE__,__func__,## args);} \
                                         while (0)
#else
#define debug(fmt,args...)
#endif

void act8942_init()
{
    debug("act8942 reset, address=0x%x", ACT8942_ADDR);
}

static unsigned char act8942_i2c_read(unsigned char reg)
{
    unsigned char val = 0;
    struct i2c_msg msgs[] = {
        {
            .addr = ACT8942_ADDR,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = ACT8942_ADDR,
            .flags = I2C_M_RD,
            .len = 1,
            .buf = &val,
        }
    };

    if ( aml_i2c_xfer(msgs, 2)< 0) {
        printf("%s: i2c transfer failed\n", __FUNCTION__);
    }

    return val;
}

static void act8942_i2c_write(unsigned char reg, unsigned char val)
{
    unsigned char buff[2];
    buff[0] = reg;
    buff[1] = val;
    struct i2c_msg msg[] = {
        {
        .addr = ACT8942_ADDR,
        .flags = 0,
        .len = 2,
        .buf = buff,
        }
    };

    if (aml_i2c_xfer(msg, 1) < 0) {
        printf("%s: i2c transfer failed\n", __FUNCTION__);
    }
}

inline void	act8942_dump(void)
{
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_SYS_ADDR, act8942_i2c_read(ACT8942_SYS_ADDR));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_SYS_ADDR+1, act8942_i2c_read(ACT8942_SYS_ADDR+1));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG1_ADDR, act8942_i2c_read(ACT8942_REG1_ADDR));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG1_ADDR+1, act8942_i2c_read(ACT8942_REG1_ADDR+1));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG1_ADDR+2, act8942_i2c_read(ACT8942_REG1_ADDR+2));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG2_ADDR, act8942_i2c_read(ACT8942_REG2_ADDR));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG2_ADDR+1, act8942_i2c_read(ACT8942_REG2_ADDR+1));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG2_ADDR+2, act8942_i2c_read(ACT8942_REG2_ADDR+2));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG3_ADDR, act8942_i2c_read(ACT8942_REG3_ADDR));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG3_ADDR+1, act8942_i2c_read(ACT8942_REG3_ADDR+1));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG3_ADDR+2, act8942_i2c_read(ACT8942_REG3_ADDR+2));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG4_ADDR, act8942_i2c_read(ACT8942_REG4_ADDR));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG4_ADDR+1, act8942_i2c_read(ACT8942_REG4_ADDR+1));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG5_ADDR, act8942_i2c_read(ACT8942_REG5_ADDR));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG5_ADDR+1, act8942_i2c_read(ACT8942_REG5_ADDR+1));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG6_ADDR, act8942_i2c_read(ACT8942_REG6_ADDR));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG6_ADDR+1, act8942_i2c_read(ACT8942_REG6_ADDR+1));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG7_ADDR, act8942_i2c_read(ACT8942_REG7_ADDR));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_REG7_ADDR+1, act8942_i2c_read(ACT8942_REG7_ADDR+1));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_APCH_ADDR, act8942_i2c_read(ACT8942_APCH_ADDR));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_APCH_ADDR+1, act8942_i2c_read(ACT8942_APCH_ADDR+1));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_APCH_ADDR+8, act8942_i2c_read(ACT8942_APCH_ADDR+8));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_APCH_ADDR+9, act8942_i2c_read(ACT8942_APCH_ADDR+9));
	printf("act8942: [0x%x] : 0x%x\n", ACT8942_APCH_ADDR+0xa, act8942_i2c_read(ACT8942_APCH_ADDR+0xa));
}
