align 16
alloc_alignment:
	sub.q r31, r31, #40
	lea r30, 0(r31)
	store.q r1, (r30)
	lea r30, 8(r31)
	store.q r1, (r30)
	lea r30, 16(r31)
	store.q r1, (r30)
	lea r1, 0(r31)
	lea r2, 8(r31)
	lea r3, 16(r31)
	jsr use
	move.q r1, r1
	add.q r31, r31, #40
	rts
align 16
use:
	move.q r1, r1
	rts
