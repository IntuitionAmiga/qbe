.section .text,"ax"
.global phialloc
.type phialloc,@function
.align 16
phialloc:
	sub.q r31, r31, #72
	lea r2, 0(r31)
	move.l r4, r1
	move.q r1, r2
	moveq r2, #0
	lea r5, 32(r31)
	sext.l r3, r2
	sext.l r6, r4
	moveq r7, #7
	bge r3, r6, .L5
.L3:
	add.l r2, r2, #1
	add.q r3, r1, #4
	store.l r7, (r1)
	beq r2, r0, .L6
.L4:
	move.q r1, r3
	lea r5, 32(r31)
	sext.l r3, r2
	sext.l r6, r4
	blt r3, r6, .L3
.L5:
	move.q r1, r5
.L6:
	add.q r31, r31, #72
	rts
.size phialloc,.-phialloc
.section .text,"ax"
.global retalloc
.type retalloc,@function
.align 16
retalloc:
	sub.q r31, r31, #8
	lea r1, 0(r31)
	moveq r2, #42
	store.q r2, (r1)
	add.q r31, r31, #8
	rts
.size retalloc,.-retalloc
