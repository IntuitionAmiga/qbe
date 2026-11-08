.section .text,"ax"
.global aggregate_vaarg
.type aggregate_vaarg,@function
.align 16
aggregate_vaarg:
	sub.q r31, r31, #24
	lea r2, 8(r31)
	lea r1, 32(r31)
	store.q r1, (r31)
	load.q r3, (r31)
	load.q r1, (r3)
	add.q r3, r3, #8
	store.q r3, (r31)
	load.q r3, (r1)
	store.q r3, (r2)
	load.q r1, 8(r1)
	store.q r1, 8(r2)
	move.q r1, r2
	add.q r31, r31, #24
	rts
.size aggregate_vaarg,.-aggregate_vaarg
