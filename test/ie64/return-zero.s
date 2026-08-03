.section .text,"ax"
.global main
.type main,@function
.align 16
main:
	li r1, #0
	sext.l r1, r1
	rts
.size main,.-main
