align 16
double_to_long:
	dcvtfi r1, f0
	move.q r1, r1
	rts
align 16
long_to_double:
	dcvtif f0, r1
.L2:
	rts
align 16
single_to_long:
	fcvtsd f0, f0
	dcvtfi r1, f0
	move.q r1, r1
	rts
align 16
long_to_single:
	dcvtif f0, r1
	fcvtds f0, f0
.L5:
	rts
