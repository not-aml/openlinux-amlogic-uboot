   .equ PL310_BASE     , 0xc4200000
   .equ     _STACK_END , 0x49001d80
   .equ     _FIQ_STACK,  0x49001e00
   .equ     _IRQ_STACK,  0x49001f00

    .text
    .extern     main 
    .global     _start

_start:

   NOP
   DMB
   ISB

   NOP
   NOP
   NOP


   
   MSR     CPSR_c, #0xd3    @ or of Mode_SVC, I_Bit, and F_Bit
   LDR     sp, =_STACK_END  @ STACK_END is defined in memmap 

   
   @ write a cbus register
   
   @ ---------------------------------------------
   @ Write this to trigger $top/common/clk_util.v
   @ ---------------------------------------------
   LDR r1, =0xc110995c
   LDR r0, [r1]
   ORR r0, r0, #1 << 0
   STR r0, [r1]

    @ Branch to Main C routine
    LDR       r0, = main
    LDR       sp, =_STACK_END

    MOV       lr, pc
    BX        r0
	b 		  .

    
.section .rodata
.global burn_data
burn_data:
	.incbin "aml-debug-rom.bin"
 
    
