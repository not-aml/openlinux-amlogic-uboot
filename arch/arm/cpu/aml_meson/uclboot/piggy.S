	.section .piggydata
	.globl	input_data
input_data:
	.incbin	"../../../../../build/piggy.ucl"
	.globl	input_data_end
input_data_end:
