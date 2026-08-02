align 16
single_eq:
	fcmp r29, f0, f1
	fmovsr r30
	and.q r30, r30, #0x1000000
	li r1, #0
	bne r30, r0, .Lcmp0
	beq r29, r0, .Lcmptrue0
	bra .Lcmp0
.Lcmptrue0:
	li r1, #1
.Lcmp0:
	move.l r1, r1
	sext.l r1, r1
	rts
align 16
double_lt:
	dcmp r29, f0, f2
	fmovsr r30
	and.q r30, r30, #0x1000000
	li r1, #0
	bne r30, r0, .Lcmp1
	blt r29, r0, .Lcmptrue1
	bra .Lcmp1
.Lcmptrue1:
	li r1, #1
.Lcmp1:
	move.l r1, r1
	sext.l r1, r1
	rts
align 16
double_ordered:
	dcmp r29, f0, f2
	fmovsr r30
	and.q r30, r30, #0x1000000
	li r1, #0
	beq r30, r0, .Lcmptrue2
	bra .Lcmp2
.Lcmptrue2:
	li r1, #1
.Lcmp2:
	move.l r1, r1
	sext.l r1, r1
	rts
align 16
single_unordered:
	fcmp r29, f0, f1
	fmovsr r30
	and.q r30, r30, #0x1000000
	li r1, #0
	bne r30, r0, .Lcmptrue3
	bra .Lcmp3
.Lcmptrue3:
	li r1, #1
.Lcmp3:
	move.l r1, r1
	sext.l r1, r1
	rts
align 16
double_branch:
	dcmp r29, f0, f2
	fmovsr r30
	and.q r30, r30, #0x1000000
	li r1, #0
	bne r30, r0, .Lcmp4
	blt r29, r0, .Lcmptrue4
	bra .Lcmp4
.Lcmptrue4:
	li r1, #1
.Lcmp4:
	bne r1, r0, .L6
.L5:
	li r1, #0
	sext.l r1, r1
	rts
.L6:
	li r1, #1
	sext.l r1, r1
	rts
