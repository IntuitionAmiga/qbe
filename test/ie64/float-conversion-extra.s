.section .text,"ax"
.global extend_float
.type extend_float,@function
.align 16
extend_float:
	fcvtsd f0, f0
.L1:
	rts
.size extend_float,.-extend_float
.section .text,"ax"
.global truncate_double
.type truncate_double,@function
.align 16
truncate_double:
	fcvtds f0, f0
.L3:
	rts
.size truncate_double,.-truncate_double
.section .text,"ax"
.global truncate_constant
.type truncate_constant,@function
.align 16
truncate_constant:
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	fload f0, (r30)
.L5:
	rts
.size truncate_constant,.-truncate_constant
.section .rodata,"a"
.local .L__ie64_fconst_0
align 4
.L__ie64_fconst_0:
	dc.l 0x3f000000
