align 16
f:
	move.q r30, #x+0
	load.q r2, (r30)
	move.q r30, #x+0
	load.q r1, (r30)
	add.q r2, r2, r1
	move.q r30, #x+0
	store.q r2, (r30)
	li r2, #3
	move.q r30, #x+0
	store.q r2, (r30)
	move.q r1, r1
	rts
align 16
discard:
	move.q r30, #x+0
	load.q r1, (r30)
	move.q r30, #x+0
	load.q r1, (r30)
.L2:
	rts
align 8
x:
	ds.b 8
