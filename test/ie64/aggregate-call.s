align 16
f:
.L1:
	rts
align 16
g:
	sub.q r31, r31, #8
	lea r30, 0(r31)
	store.q r1, (r30)
	move.q r30, #s+0
	load.l r1, (r30)
	lea r2, 0(r31)
	store.l r1, (r2)
	lea r1, 0(r31)
	jsr f
.L3:
	add.q r31, r31, #8
	rts
align 4
s:
	ds.b 4
