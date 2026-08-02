align 16
single_zero:
	move.q r30, #__ie64_fconst_0
	fload f0, (r30)
.L1:
	rts
align 16
double_zero:
	move.q r30, #__ie64_fconst_1
	dload f0, (r30)
.L3:
	rts
align 16
both_zero:
	sub.q r31, r31, #8
	move.q r30, #__ie64_fconst_0
	fload f0, (r30)
	move.q r30, #__ie64_fconst_1
	dload f2, (r30)
	jsr consume_zeros
.L5:
	add.q r31, r31, #8
	rts
align 16
consume_zeros:
.L7:
	rts
align 4
__ie64_fconst_0:
	dc.l 0x00000000
align 8
__ie64_fconst_1:
	dc.q 0x0000000000000000
