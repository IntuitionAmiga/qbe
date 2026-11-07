.section .text,"ax"
.global callee
.type callee,@function
.align 16
callee:
	load.l r1, 8(r31)
	sext.l r1, r1
	rts
.size callee,.-callee
.section .text,"ax"
.global caller
.type caller,@function
.align 16
caller:
	sub.q r31, r31, #8
	store.q r25, 0(r31)
	move.q r25, r31
	sub.q r31, r31, #16
	move.q r8, r31
	moveq r1, #1
	moveq r2, #2
	moveq r3, #3
	moveq r4, #4
	moveq r5, #5
	moveq r6, #6
	moveq r7, #7
	store.q r7, (r8)
	jsr callee
	add.q r31, r31, #16
	sext.l r1, r1
	move.q r31, r25
	load.q r25, 0(r31)
	add.q r31, r31, #8
	rts
.size caller,.-caller
