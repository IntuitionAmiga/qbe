.section .text,"ax"
.global cursor
.type cursor,@function
.align 16
cursor:
	sub.q r31, r31, #40
	lea r2, 16(r31)
	lea r1, 48(r31)
	store.q r1, (r31)
	moveq r1, #77
	store.q r1, 8(r31)
	load.q r3, (r31)
	load.q r5, (r31)
	load.q r1, (r5)
	add.q r5, r5, #8
	store.q r5, (r31)
	load.q r5, (r1)
	store.q r5, (r2)
	load.q r1, 8(r1)
	store.q r1, 8(r2)
	load.q r1, (r31)
	sub.q r1, r1, r3
	add.q r1, r1, #154
	add.q r1, r2, r1
	add.q r31, r31, #40
	rts
.size cursor,.-cursor
