.section .text,"ax"
.global f
.type f,@function
.align 16
f:
	move.l r1, r1
	li r2, #3
	move.l r2, r2
	beq r1, r2, .L2
.L1:
	li r1, #0
	sext.l r1, r1
	rts
.L2:
	li r1, #1
	sext.l r1, r1
	rts
.size f,.-f
