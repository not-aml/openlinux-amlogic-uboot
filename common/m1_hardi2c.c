#define I2C_B

#ifdef I2C_A
#define  I2C_CONTROL_REG      (volatile unsigned long *)0xc1108500
#define  I2C_SLAVE_ADDR       (volatile unsigned long *)0xc1108504
#define  I2C_TOKEN_LIST_REG0  (volatile unsigned long *)0xc1108508
#define  I2C_TOKEN_LIST_REG1  (volatile unsigned long *)0xc110850c
#define  I2C_TOKEN_WDATA_REG0 (volatile unsigned long *)0xc1108510
#define  I2C_TOKEN_WDATA_REG1 (volatile unsigned long *)0xc1108514
#define  I2C_TOKEN_RDATA_REG0 (volatile unsigned long *)0xc1108518
#define  I2C_TOKEN_RDATA_REG1 (volatile unsigned long *)0xc110851c
#elif defined I2C_B
#define  I2C_CONTROL_REG      (volatile unsigned long *)0xc1108520
#define  I2C_SLAVE_ADDR       (volatile unsigned long *)0xc1108524
#define  I2C_TOKEN_LIST_REG0  (volatile unsigned long *)0xc1108528
#define  I2C_TOKEN_LIST_REG1  (volatile unsigned long *)0xc110852c
#define  I2C_TOKEN_WDATA_REG0 (volatile unsigned long *)0xc1108530
#define  I2C_TOKEN_WDATA_REG1 (volatile unsigned long *)0xc1108534
#define  I2C_TOKEN_RDATA_REG0 (volatile unsigned long *)0xc1108538
#define  I2C_TOKEN_RDATA_REG1 (volatile unsigned long *)0xc110853c
#endif

#define  I2C_END               0x0
#define  I2C_START             0x1
#define  I2C_SLAVE_ADDR_WRITE  0x2
#define  I2C_SLAVE_ADDR_READ   0x3
#define  I2C_DATA              0x4
#define  I2C_DATA_LAST         0x5
#define  I2C_STOP              0x6

typedef char byte;

void hard_i2c_init(unsigned speed)
{
    unsigned delay = 117;//112;//400K //450;//clk81=180M //469;//clk81=187.5M  i2c speed is 100K
    
//    if(speed)
//        delay = (1800 * 25 / speed) & 0x3ff; //(1875 * 25 / speed) & 0x3ff;        
        
    (*I2C_CONTROL_REG) = ((*I2C_CONTROL_REG) & ~(0x3FF << 12)) | (delay << 12); 
}

byte hard_i2c_read8(byte SlaveAddr, byte RegAddr)
{    
    // Set the I2C Address
    (*I2C_SLAVE_ADDR) = ((*I2C_SLAVE_ADDR) & ~0xff) | SlaveAddr;
    // Fill the token registers
    (*I2C_TOKEN_LIST_REG0) = (I2C_END  << 24)             |
                             (I2C_DATA_LAST << 20)        |  // this is a data read
                             (I2C_SLAVE_ADDR_READ << 16)  |
                             (I2C_START << 12)            |
                             (I2C_DATA << 8)              |  // This is a data write
                             (I2C_SLAVE_ADDR_WRITE << 4)  |
                             (I2C_START << 0);

    // Fill the write data registers
    (*I2C_TOKEN_WDATA_REG0) = (RegAddr << 0);
    // Start and Wait
    (*I2C_CONTROL_REG) &= ~(1 << 0);   // Clear the start bit
    (*I2C_CONTROL_REG) |= (1 << 0);   // Set the start bit
    while( (*I2C_CONTROL_REG) & (1 << 2) ) {}

    return( (byte)((*I2C_TOKEN_RDATA_REG0) & 0xFF) );
}

void hard_i2c_write8(byte SlaveAddr, byte RegAddr, byte Data)
{
    // Set the I2C Address
    (*I2C_SLAVE_ADDR) = ((*I2C_SLAVE_ADDR) & ~0xff) | SlaveAddr;
    // Fill the token registers
    (*I2C_TOKEN_LIST_REG0) = (I2C_END  << 16)             |
                             (I2C_DATA << 12)             |    // Write Data
                             (I2C_DATA << 8)              |    // Write RegAddr
                             (I2C_SLAVE_ADDR_WRITE << 4)  |
                             (I2C_START << 0);

    // Fill the write data registers
    (*I2C_TOKEN_WDATA_REG0) = (Data << 8) | (RegAddr << 0);
    // Start and Wait
    (*I2C_CONTROL_REG) &= ~(1 << 0);   // Clear the start bit
    (*I2C_CONTROL_REG) |= (1 << 0);   // Set the start bit
    while( (*I2C_CONTROL_REG) & (1 << 2) ) {}
}

