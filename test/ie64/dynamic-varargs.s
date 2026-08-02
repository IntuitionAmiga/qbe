align 16
dynamic_varargs:
	sub.q r31, r31, #24
	store.q r18, 8(r31)
	store.q r25, 16(r31)
	move.q r25, r31
	add.q r1, r1, #15
	and.q r1, r1, #-16
	sub.q r31, r31, r1
	move.q r1, r31
	lea r30, 0(r25)
	store.q r2, (r30)
	lea r2, 40(r25)
	lea r3, 0(r25)
	store.q r2, (r3)
	lea r2, 0(r25)
	load.q r2, (r2)
	load.q r18, (r2)
	add.q r2, r2, #8
	lea r3, 0(r25)
	store.q r2, (r3)
	store.q r18, (r1)
	jsr use
	move.q r1, r18
	move.q r31, r25
	load.q r18, 8(r31)
	load.q r25, 16(r31)
	add.q r31, r31, #24
	rts
align 16
use:
.L2:
	rts
