.section .text,"ax"
.local callee
.type callee,@function
.align 16
callee:
	sub.q r31, r31, #408
	lea r2, 0(r31)
	sext.l r3, r1
	lsl.q r3, r3, #2
	add.q r2, r2, r3
	store.l r1, (r2)
	sext.l r1, r1
	add.q r31, r31, #408
	rts
.size callee,.-callee
.section .text,"ax"
.global caller
.type caller,@function
.align 16
caller:
	sub.q r31, r31, #424
	lea r3, 0(r31)
	lea r2, 400(r31)
	sext.l r4, r1
	lsl.q r5, r4, #3
	add.q r2, r2, r5
	moveq r5, #0
	store.q r5, (r2)
	moveq r5, #77
	store.l r5, (r2)
	lsl.q r4, r4, #2
	add.q r3, r3, r4
	store.l r1, (r3)
	load.l r2, (r2)
	add.l r1, r1, r2
	sext.l r1, r1
	add.q r31, r31, #424
	rts
.size caller,.-caller
