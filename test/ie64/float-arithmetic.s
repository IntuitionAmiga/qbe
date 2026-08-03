.section .text,"ax"
.global arithmetic
.type arithmetic,@function
.align 16
arithmetic:
	fsub f2, f0, f1
	fmul f1, f1, f2
	fdiv f0, f1, f0
.L1:
	rts
.size arithmetic,.-arithmetic
