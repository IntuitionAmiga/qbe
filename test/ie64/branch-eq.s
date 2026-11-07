.section .text,"ax"
.global f
.type f,@function
.align 16
f:
	move.l r1, r1
	moveq r2, #3
	beq r1, r2, .L2
.L1:
	moveq r1, #0
	rts
.L2:
	moveq r1, #1
	rts
.size f,.-f
