align 16
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
