.section .text,"ax"
.global single_zero
.type single_zero,@function
.align 16
single_zero:
	fmovecr f0, #7
	rts
.size single_zero,.-single_zero
.section .text,"ax"
.global double_zero
.type double_zero,@function
.align 16
double_zero:
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f0, (r30)
	rts
.size double_zero,.-double_zero
.section .text,"ax"
.global both_zero
.type both_zero,@function
.align 16
both_zero:
	sub.q r31, r31, #8
	fmovecr f0, #7
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f2, (r30)
	jsr consume_zeros
	add.q r31, r31, #8
	rts
.size both_zero,.-both_zero
.section .text,"ax"
.local consume_zeros
.type consume_zeros,@function
.align 16
consume_zeros:
	rts
.size consume_zeros,.-consume_zeros
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 8
.L__ie64_fconst_0:
	dc.q 0x0000000000000000
