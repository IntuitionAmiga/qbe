.section .text,"ax"
.global run
.type run,@function
.align 16
run:
	bne r1, r0, .L3
.L1:
	moveq r1, #0
.L2:
	add.l r1, r1, #1
	sext.l r3, r1
	sext.l r4, r2
	blt r3, r4, .L2
	bra .L4
.L3:
	mulu.l r1, r2, #7
.L4:
	sext.l r1, r1
	rts
.size run,.-run
