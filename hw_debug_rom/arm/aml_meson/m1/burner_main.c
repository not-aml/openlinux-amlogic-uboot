// Local defines for AHB and some CBUS Registers 
#include "c_arc_pointer_reg.h" 
#include <common.h>


// -----------------------------------------------
// The following is used establish a mechanism
// for the ARC to communicate with stimulus.v
// You can add defines as needed (to here and
// in stimulus.v)
// -----------------------------------------------
#define BOOT_CODE_SIZE                  (0x1b90 / 4)
#define ALL_CODE_SIZE                   (0x2700 / 4)


// --------------------------------
// Function prototypes
// --------------------------------
void copy_spi_to_ddr( unsigned *addr_s, unsigned *addr_d, int bytes_length);

int SPI_wr_status(int data);
extern int SPI_chip_erase(void);
extern int SPI_sector_erase(unsigned addr);
extern int SPI_page_program(unsigned * addr_source, unsigned spi_addr, int byte_length);
extern void    stimulus_print_num_hex( unsigned long data );

#define stimulus_display(a,b)
#define stimulus_print(a)
#define stimulus_finish_fail(a)
#define stimulus_finish_pass()
// --------------------------------------------------------
//                     C_Entry
// --------------------------------------------------------
unsigned id;

void main(void) 
{


    asm("ISB");
    asm("DMB");
    *(P_HHI_MPEG_CLK_CNTL)&=~(1<<8);
    // disable SPI flash AHB request.
    (*P_SPI_FLASH_CTRL) = (*P_SPI_FLASH_CTRL) & 0xfffdffff ;

    *(P_SPI_FLASH_CTRL) = 0xea313;    // system clock / 4.
    *(P_SPI_FLASH_CTRL1) = 0x501f0080;
    
    // enable the HOLD and WP pin.
    (*P_PERIPHS_PIN_MUX_1)  |= (1 << 29) | (1<<23)|(1<<25)|(1<<27); 
    (*P_PERIPHS_PIN_MUX_6) =0;
    (*P_PERIPHS_PIN_MUX_11) =0;
    serial_init_with_clk(CONFIG_CRYSTAL_MHZ*1000000);
    serial_puts("\nChip Erase ...\n");
    SPI_chip_erase();
    serial_puts("\nprogram ...\n");
    SPI_page_program(rawData,0,sizeof(rawData));
    serial_puts("\nfinish ...\n");
    
}
        




