align 16
toint:
	fcvtfi r1, f0
	move.l r1, r1
	sext.l r1, r1
	rts
align 16
tofloat:
	fcvtif f0, r1
.L2:
	rts
