.section .text,"ax"
.global f
.type f,@function
.align 16
f:
	fadd f0, f0, f1
.L1:
	rts
.size f,.-f
.section .text,"ax"
.global g
.type g,@function
.align 16
g:
	sub.q r31, r31, #8
	jsr f
.L3:
	add.q r31, r31, #8
	rts
.size g,.-g
