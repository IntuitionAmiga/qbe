.section .text,"ax"
.global pick
.type pick,@function
.align 16
pick:
	sext.l r3, r1
	sext.l r4, r2
	bge r3, r4, .L1
.L2:
	add.l r1, r1, #100
	mulu.l r1, r1, #3
.L3:
	sext.l r1, r1
	rts
.L1:
	sub.l r1, r1, r2
	bra .L3
.size pick,.-pick
