.section .text,"ax"
.global extend_float
.type extend_float,@function
.align 16
extend_float:
	fcvtsd f0, f0
	rts
.size extend_float,.-extend_float
.section .text,"ax"
.global truncate_double
.type truncate_double,@function
.align 16
truncate_double:
	fcvtds f0, f0
	rts
.size truncate_double,.-truncate_double
.section .text,"ax"
.global truncate_constant
.type truncate_constant,@function
.align 16
truncate_constant:
	fmovecr f0, #13
	rts
.size truncate_constant,.-truncate_constant
