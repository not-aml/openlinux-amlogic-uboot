

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x49000000;

	. = ALIGN(4);
	.text      :
	{
	  *rom_spl.o	(.text)
	  
	  *(.text)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : { 
		*(.data) 
	}
	. = 0x490017f0;
	.script : {
		*(.script)
	}
	_end = .;
}
ASSERT(_end <= END_ADDR, "ROM BOOT bootstrap too big");