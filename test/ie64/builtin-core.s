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
	rts
.size f,.-f
