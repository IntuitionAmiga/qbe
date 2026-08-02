align 16
bits_to_double:
	sub.q r31, r31, #16
	store.q r1, (r31)
	dload f0, (r31)
	add.q r31, r31, #16
.L1:
	rts
align 16
double_to_bits:
	sub.q r31, r31, #16
	dstore f0, (r31)
	load.q r1, (r31)
	add.q r31, r31, #16
	move.q r1, r1
	rts
