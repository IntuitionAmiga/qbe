align 16
dynamic_alloc:
	sub.q r31, r31, #8
	store.q r25, 0(r31)
	move.q r25, r31
	add.q r1, r1, #15
	and.q r1, r1, #-16
	sub.q r31, r31, r1
	move.q r1, r31
	li r2, #42
	store.q r2, (r1)
	jsr use
	move.q r1, r1
	move.q r31, r25
	load.q r25, 0(r31)
	add.q r31, r31, #8
	rts
align 16
use:
	move.q r1, r1
	rts
