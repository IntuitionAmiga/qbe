align 16
f:
	load.q r1, 8(r31)
	load.l r1, (r1)
	sext.l r1, r1
	move.l r1, r1
	sext.l r1, r1
	rts
align 16
g:
	sub.q r31, r31, #24
	store.q r25, 8(r31)
	move.q r25, r31
	sub.q r31, r31, #16
	move.q r8, r31
	li r1, #1
	li r2, #2
	li r3, #3
	li r4, #4
	li r5, #5
	li r6, #6
	lea r30, 0(r25)
	store.q r7, (r30)
	move.q r30, #s+0
	load.l r7, (r30)
	lea r9, 0(r25)
	store.l r7, (r9)
	lea r7, 0(r25)
	store.q r7, (r8)
	jsr f
	add.q r31, r31, #16
	move.l r1, r1
	sext.l r1, r1
	move.q r31, r25
	load.q r25, 8(r31)
	add.q r31, r31, #24
	rts
align 4
s:
	dc.l 7
