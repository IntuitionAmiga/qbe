align 16
roundtrip:
	move.q r30, #g+0
	fstore f0, (r30)
.L1:
	rts
align 4
g:
	dc.l 0
