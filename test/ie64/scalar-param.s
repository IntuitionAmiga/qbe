.section .text,"ax"
.global inc
.type inc,@function
.align 16
inc:
	add.l r1, r1, #1
	move.l r1, r1
	sext.l r1, r1
	rts
.size inc,.-inc
