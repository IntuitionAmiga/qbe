.section .text,"ax"
.global pointer_dot
.type pointer_dot,@function
.align 16
pointer_dot:
	move.l r3, r3
	lsl.q r3, r3, #1
	add.q r4, r1, r3
	moveq r3, #0
	beq r1, r4, .L4
.L3:
	load.w r5, (r1)
	sext.w r5, r5
	load.w r6, (r2)
	sext.w r6, r6
	mulu.q r5, r5, r6
	add.q r3, r3, r5
	add.q r1, r1, #2
	add.q r2, r2, #2
	bne r1, r4, .L3
.L4:
	move.q r1, r3
	rts
.size pointer_dot,.-pointer_dot
.section .text,"ax"
.global pointer_bytes
.type pointer_bytes,@function
.align 16
pointer_bytes:
	move.l r2, r2
	add.q r3, r1, r2
	moveq r2, #0
	beq r1, r3, .L9
.L8:
	load.b r4, (r1)
	add.l r2, r2, r4
	add.q r1, r1, #1
	bne r1, r3, .L8
.L9:
	sext.l r1, r2
	rts
.size pointer_bytes,.-pointer_bytes
