align 16
double_to_ulong:
	move.q r30, #__ie64_fconst_0
	dload f4, (r30)
	dcmp r29, f0, f4
	fmovsr r30
	and.q r30, r30, #0x1000000
	li r1, #0
	bne r30, r0, .Lcmp0
	bge r29, r0, .Lcmptrue0
	bra .Lcmp0
.Lcmptrue0:
	li r1, #1
.Lcmp0:
	and.q r2, r1, #0xffffffff
	dcvtif f2, r2
	dmul f2, f2, f4
	dsub f0, f0, f2
	dcvtfi r1, f0
	lsl.q r2, r2, #63
	or.q r1, r1, r2
	move.q r1, r1
	rts
align 16
single_to_ulong:
	fcvtsd f0, f0
	move.q r30, #__ie64_fconst_0
	dload f4, (r30)
	dcmp r29, f0, f4
	fmovsr r30
	and.q r30, r30, #0x1000000
	li r1, #0
	bne r30, r0, .Lcmp1
	bge r29, r0, .Lcmptrue1
	bra .Lcmp1
.Lcmptrue1:
	li r1, #1
.Lcmp1:
	and.q r2, r1, #0xffffffff
	dcvtif f2, r2
	dmul f2, f2, f4
	dsub f0, f0, f2
	dcvtfi r1, f0
	lsl.q r2, r2, #63
	or.q r1, r1, r2
	move.q r1, r1
	rts
align 16
ulong_to_double:
	and.q r2, r1, #1
	lsr.q r1, r1, #1
	or.q r1, r1, r2
	dcvtif f0, r1
	dadd f0, f0, f0
.L3:
	rts
align 16
ulong_to_single:
	and.q r2, r1, #1
	lsr.q r1, r1, #1
	or.q r1, r1, r2
	dcvtif f0, r1
	dadd f0, f0, f0
	fcvtds f0, f0
.L5:
	rts
align 16
uint_to_double:
	and.q r1, r1, #0xffffffff
	dcvtif f0, r1
.L7:
	rts
align 16
uint_to_single:
	and.q r1, r1, #0xffffffff
	dcvtif f0, r1
	fcvtds f0, f0
.L9:
	rts
align 8
__ie64_fconst_0:
	dc.q 0x43e0000000000000
