/*  ARC c code header file for spi flash applications.
*/
#ifdef SPI_FLASH
#else 
#define SPI_FLASH

#include "c_arc_pointer_reg.h"
#include <common.h>


int SPI_chip_erase(void);
int SPI_sector_erase(unsigned addr);
int SPI_page_program(unsigned * addr_source, unsigned spi_addr, int byte_length);
int SPI_power_down(void);
int SPI_rel_pd(void);
int SPI_test_usr_cmd(void);





int SPI_page_program(unsigned * addr_source, unsigned spi_addr, int byte_length)
{
   unsigned temp;
   unsigned temp_addr;
   int temp_bl;
   
   temp_addr = spi_addr;
   temp_bl = byte_length;
   if(byte_length&0x1f)
        return -1;
   *P_SPI_FLASH_CTRL = (*P_SPI_FLASH_CTRL) & (~(1 << SPI_ENABLE_AHB )) ; 
   
   while (temp_bl > 0 ) {
     if ( temp_bl >= 32 ) {
        *P_SPI_FLASH_ADDR = (temp_addr & 0xffffff) | 
                          ( 32 << SPI_FLASH_BYTES_LEN );
        *P_SPI_FLASH_C0 = *addr_source++;
        *P_SPI_FLASH_C1 = *addr_source++;
        *P_SPI_FLASH_C2 = *addr_source++;
        *P_SPI_FLASH_C3 = *addr_source++;
        *P_SPI_FLASH_C4 = *addr_source++;
        *P_SPI_FLASH_C5 = *addr_source++;
        *P_SPI_FLASH_C6 = *addr_source++;
        *P_SPI_FLASH_C7 = *addr_source++;
        temp_bl = temp_bl - 32;
        temp_addr = temp_addr + 32;
     }
     *P_SPI_FLASH_CMD  =  1 << SPI_FLASH_WREN;
     while ( (*P_SPI_FLASH_CMD ) !=0 ); 
     *P_SPI_FLASH_CMD = 1 << SPI_FLASH_PP;
     while ( (*P_SPI_FLASH_CMD ) !=0 );
     temp = 1;
     while ( (temp & 1) == 1 ) { 
        *P_SPI_FLASH_CMD = 1 << SPI_FLASH_RDSR;
        while ((*P_SPI_FLASH_CMD ) != 0 ); 
        temp = *P_SPI_FLASH_STATUS ;
     }
  } 
  *(P_SPI_FLASH_CTRL) = (*P_SPI_FLASH_CTRL) | (1 << SPI_ENABLE_AHB ) ; 
  return 0;
}
int SPI_sector_erase(unsigned addr )
{
   unsigned temp;
   *P_SPI_FLASH_CTRL = (*P_SPI_FLASH_CTRL) & (~(1 << SPI_ENABLE_AHB )) ;
   *P_SPI_FLASH_ADDR = addr & 0xffffff;
   *P_SPI_FLASH_CMD  =  1 << SPI_FLASH_WREN;
   while ( (*P_SPI_FLASH_CMD ) !=0 );
   // sector erase  64Kbytes erase is block erase.
   *P_SPI_FLASH_CMD = 1 << SPI_FLASH_BE;
   while ( (*P_SPI_FLASH_CMD ) !=0 );
   // check erase is finished.
  temp = 1;
  while ( (temp & 1) == 1 ) {
     *P_SPI_FLASH_CMD = 1 << SPI_FLASH_RDSR;
     while ((*P_SPI_FLASH_CMD ) != 0 );
     temp = *P_SPI_FLASH_STATUS ;

  }
  *(P_SPI_FLASH_CTRL) = (*P_SPI_FLASH_CTRL) | (1 << SPI_ENABLE_AHB ) ;
  return 0;
}
static void spi_init_timing(void)
{
	    *(P_SPI_FLASH_CTRL) = 0xea949;    // system clock / 10.
	    *(P_SPI_FLASH_CTRL1) = 0x501f0080;

	    // enable the HOLD and WP pin.
	    (*P_PERIPHS_PIN_MUX_1)  |= (1 << 29) | (1<<23)|(1<<25)|(1<<27);
	    (*P_PERIPHS_PIN_MUX_6) =0;
	    (*P_PERIPHS_PIN_MUX_11) =0;
}
void spi_erase()
{
    spi_init_timing();
    serial_puts("\nErase..");
    SPI_sector_erase(0);
}
void spi_program(unsigned addr,unsigned size)
{
	serial_puts("\nProgram..");
    SPI_page_program(addr,0,size);
    serial_puts("\nEnd..\n");

}

#endif
