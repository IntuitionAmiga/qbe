align 16
callee:
	fload f0, 16(r31)
.L1:
	rts
align 16
caller:
	sub.q r31, r31, #24
	store.q r25, 0(r31)
	fstore f8, 8(r31)
	move.q r25, r31
	load.l r8, 32(r25)
	fload f8, 40(r25)
	sub.q r31, r31, #16
	move.q r7, r31
	store.q r8, (r7)
	add.q r7, r7, #8
	fstore f8, (r7)
	jsr callee
	add.q r31, r31, #16
.L3:
	move.q r31, r25
	load.q r25, 0(r31)
	fload f8, 8(r31)
	add.q r31, r31, #24
	rts
