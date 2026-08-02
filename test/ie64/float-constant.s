align 16
constant:
	move.q r30, #__ie64_fconst_0
	fload f0, (r30)
.L1:
	rts
align 4
__ie64_fconst_0:
	dc.l 0x40000000
