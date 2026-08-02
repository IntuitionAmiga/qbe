align 16
f:
	sub.q r31, r31, #8
	lea r30, 0(r31)
	store.q r3, (r30)
	lea r3, 0(r31)
	store.l r2, (r3)
	lea r2, 0(r31)
	load.l r2, (r2)
	store.l r2, (r1)
.L1:
	add.q r31, r31, #8
	rts
align 16
g:
	sub.q r31, r31, #8
	lea r30, 0(r31)
	store.q r1, (r30)
	lea r1, 0(r31)
	li r2, #2
	jsr f
	lea r1, 0(r31)
	load.l r1, (r1)
	sext.l r1, r1
	move.l r1, r1
	sext.l r1, r1
	add.q r31, r31, #8
	rts
