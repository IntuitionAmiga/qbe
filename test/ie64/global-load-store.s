align 16
main:
	move.q r30, #g+0
	load.l r1, (r30)
	sext.l r1, r1
	add.l r1, r1, #4
	move.q r30, #g+0
	store.l r1, (r30)
	move.l r1, r1
	sext.l r1, r1
	rts
align 4
g:
	dc.l 3
