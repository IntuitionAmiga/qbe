.section .text,"ax"
.global constant
.type constant,@function
.align 16
constant:
	fmovecr f0, #9
	rts
.size constant,.-constant
