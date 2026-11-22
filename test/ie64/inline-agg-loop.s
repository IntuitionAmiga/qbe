.section .text,"ax"
.local step
.type step,@function
.align 16
step:
	sub.q r31, r31, #8
	lea r4, 0(r31)
	move.q r30, r2
	move.q r2, r3
	move.q r3, r30
	move.q r30, r1
	move.q r1, r2
	move.q r2, r30
	load.q r3, (r3)
	add.q r1, r1, r3
	store.q r1, (r4)
	load.q r1, (r4)
	store.q r1, (r2)
	add.q r31, r31, #8
	rts
.size step,.-step
.section .text,"ax"
.global loop
.type loop,@function
.align 16
loop:
	sub.q r31, r31, #24
	move.q r3, r1
	moveq r1, #1
	moveq r6, #3
.L4:
	lea r2, 8(r31)
	load.q r3, (r3)
	add.q r3, r1, r3
	store.q r3, (r31)
	load.q r4, (r31)
	store.q r4, (r2)
	add.q r1, r1, #1
	bge r1, r6, .L6
.L5:
	move.q r3, r2
	bra .L4
.L6:
	move.q r1, r3
	add.q r31, r31, #24
	rts
.size loop,.-loop
