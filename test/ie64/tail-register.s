.section .text,"ax"
.global forward
.type forward,@function
.align 16
forward:
	move.q r30, r1
	move.q r1, r2
	move.q r2, r30
	bra callee
.size forward,.-forward
.section .text,"ax"
.global stackarg
.type stackarg,@function
.align 16
stackarg:
	sub.q r31, r31, #8
	store.q r25, 0(r31)
	move.q r25, r31
	move.q r7, r1
	sub.q r31, r31, #16
	move.q r8, r31
	moveq r1, #1
	moveq r2, #2
	moveq r3, #3
	moveq r4, #4
	moveq r5, #5
	moveq r6, #6
	store.q r7, (r8)
	jsr callee7
	add.q r31, r31, #16
	move.q r31, r25
	load.q r25, 0(r31)
	add.q r31, r31, #8
	rts
.size stackarg,.-stackarg
.section .text,"ax"
.global localaddr
.type localaddr,@function
.align 16
localaddr:
	sub.q r31, r31, #8
	lea r2, 0(r31)
	move.q r30, r1
	move.q r1, r2
	move.q r2, r30
	store.q r2, (r1)
	jsr consume
	add.q r31, r31, #8
	rts
.size localaddr,.-localaddr
