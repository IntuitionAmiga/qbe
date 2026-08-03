.section .text,"ax"
.global double_to_long
.type double_to_long,@function
.align 16
double_to_long:
	dcvtfi r1, f0
	move.q r1, r1
	rts
.size double_to_long,.-double_to_long
.section .text,"ax"
.global long_to_double
.type long_to_double,@function
.align 16
long_to_double:
	dcvtif f0, r1
.L2:
	rts
.size long_to_double,.-long_to_double
.section .text,"ax"
.global single_to_long
.type single_to_long,@function
.align 16
single_to_long:
	fcvtsd f0, f0
	dcvtfi r1, f0
	move.q r1, r1
	rts
.size single_to_long,.-single_to_long
.section .text,"ax"
.global long_to_single
.type long_to_single,@function
.align 16
long_to_single:
	dcvtif f0, r1
	fcvtds f0, f0
.L5:
	rts
.size long_to_single,.-long_to_single
