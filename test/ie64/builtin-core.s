.section .text,"ax"
.global f
.type f,@function
.align 16
f:
	nop
	sei
	cli
	tlbflush
	suaen
	suadis
	halt
.L1:
	rts
.size f,.-f
