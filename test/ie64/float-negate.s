.section .text,"ax"
.global single_negate
.type single_negate,@function
.align 16
single_negate:
	fneg f0, f0
.L1:
	rts
.size single_negate,.-single_negate
.section .text,"ax"
.global double_negate
.type double_negate,@function
.align 16
double_negate:
	dneg f0, f0
.L3:
	rts
.size double_negate,.-double_negate
