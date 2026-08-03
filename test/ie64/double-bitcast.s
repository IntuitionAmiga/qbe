.section .text,"ax"
.global bits_to_double
.type bits_to_double,@function
.align 16
bits_to_double:
	sub.q r31, r31, #16
	store.q r1, (r31)
	dload f0, (r31)
	add.q r31, r31, #16
.L1:
	rts
.size bits_to_double,.-bits_to_double
.section .text,"ax"
.global double_to_bits
.type double_to_bits,@function
.align 16
double_to_bits:
	sub.q r31, r31, #16
	dstore f0, (r31)
	load.q r1, (r31)
	add.q r31, r31, #16
	move.q r1, r1
	rts
.size double_to_bits,.-double_to_bits
