align 16
f:
	sub.q r31, r31, #8
	lea r30, 0(r31)
	store.q r1, (r30)
	lea r1, 16(r31)
	lea r2, 0(r31)
	store.q r1, (r2)
	lea r1, 0(r31)
	load.q r2, (r1)
	load.l r1, (r2)
	add.q r2, r2, #8
	lea r3, 0(r31)
	store.q r2, (r3)
	lea r2, 0(r31)
	load.q r2, (r2)
	add.q r2, r2, #8
	lea r3, 0(r31)
	store.q r2, (r3)
	move.l r1, r1
	sext.l r1, r1
	add.q r31, r31, #8
	rts
align 16
g:
	sub.q r31, r31, #8
	store.q r25, 0(r31)
	move.q r25, r31
	sub.q r31, r31, #16
	move.q r2, r31
	li r1, #1
	li r3, #2
	store.q r3, (r2)
	add.q r2, r2, #8
	move.q r30, #__ie64_fconst_0
	dload f0, (r30)
	dstore f0, (r2)
	jsr f
	add.q r31, r31, #16
	move.l r1, r1
	sext.l r1, r1
	move.q r31, r25
	load.q r25, 0(r31)
	add.q r31, r31, #8
	rts
align 8
__ie64_fconst_0:
	dc.q 0x4008000000000000
