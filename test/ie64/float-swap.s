align 16
float_swap:
	beq r1, r0, .L3
.L1:
	sub.l r2, r1, #1
	beq r2, r0, .L3
.L2:
	sub.q r31, r31, #16
	fstore f0, (r31)
	fmov f0, f1
	fload f1, (r31)
	add.q r31, r31, #16
	bra .L1
.L3:
	rts
align 16
double_swap:
	beq r1, r0, .L7
.L5:
	sub.l r2, r1, #1
	beq r2, r0, .L7
.L6:
	sub.q r31, r31, #16
	dstore f0, (r31)
	dmov f0, f2
	dload f2, (r31)
	add.q r31, r31, #16
	bra .L5
.L7:
	rts
