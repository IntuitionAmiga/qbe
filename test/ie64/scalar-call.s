.section .text,"ax"
.global add
.type add,@function
.align 16
add:
	add.l r1, r1, r2
	sext.l r1, r1
	rts
.size add,.-add
.section .text,"ax"
.global main
.type main,@function
.align 16
main:
	sub.q r31, r31, #8
	moveq r1, #2
	moveq r2, #3
	jsr add
	sext.l r1, r1
	add.q r31, r31, #8
	rts
.size main,.-main
