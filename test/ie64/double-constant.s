align 16
constant:
	move.q r30, #__ie64_fconst_0
	dload f0, (r30)
.L1:
	rts
align 8
__ie64_fconst_0:
	dc.q 0x3ff8000000000000
