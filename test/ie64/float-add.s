.section .text,"ax"
.global f
.type f,@function
.align 16
f:
	fadd f0, f0, f1
	rts
.size f,.-f
