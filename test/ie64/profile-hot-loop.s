.section .text,"ax"
.global run
.type run,@function
.align 16
run:
	beq r1, r0, .L1
.L3:
	mulu.l r1, r2, #7
.L4:
	sext.l r1, r1
	rts
.L1:
	moveq r1, #0
.L2:
	add.l r1, r1, #1
	sext.l r3, r1
	sext.l r4, r2
	bge r3, r4, .L4
	bra .L2
.size run,.-run
