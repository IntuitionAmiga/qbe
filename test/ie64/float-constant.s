.section .text,"ax"
.global constant
.type constant,@function
.align 16
constant:
	fmovecr f1, #13
	moveq r30, #1069547520
	fmovi f0, r30
	fadd f0, f0, f1
	rts
.size constant,.-constant
