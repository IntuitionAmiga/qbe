align 16
fp_callee:
	fmov f0, f7
.L1:
	rts
align 16
fp_caller:
	sub.q r31, r31, #24
	store.q r25, 0(r31)
	fstore f8, 8(r31)
	fstore f9, 16(r31)
	move.q r25, r31
	dload f8, 32(r25)
	sub.q r31, r31, #16
	move.q r1, r31
	dstore f8, (r1)
	jsr fp_callee
	add.q r31, r31, #16
.L3:
	move.q r31, r25
	load.q r25, 0(r31)
	fload f8, 8(r31)
	fload f9, 16(r31)
	add.q r31, r31, #24
	rts
