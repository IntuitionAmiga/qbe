align 16
aggregate_vaarg:
	sub.q r31, r31, #24
	lea r30, 0(r31)
	store.q r1, (r30)
	lea r1, 32(r31)
	lea r2, 0(r31)
	store.q r1, (r2)
	lea r30, 8(r31)
	store.q r1, (r30)
	lea r2, 0(r31)
	load.q r3, (r2)
	load.q r2, (r3)
	add.q r3, r3, #8
	lea r4, 0(r31)
	store.q r3, (r4)
	load.q r3, (r2)
	lea r4, 8(r31)
	store.q r3, (r4)
	add.q r2, r2, #8
	lea r3, 8(r31)
	add.q r3, r3, #8
	load.q r2, (r2)
	store.q r2, (r3)
	move.q r1, r1
	add.q r31, r31, #24
	rts
