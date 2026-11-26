.section .text,"ax"
.global arithmetic
.type arithmetic,@function
.align 16
arithmetic:
	fsub f2, f0, f1
	fmul f1, f2, f1
	fdiv f0, f1, f0
	rts
.size arithmetic,.-arithmetic
