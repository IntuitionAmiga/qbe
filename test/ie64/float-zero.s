.section .text,"ax"
.global single_zero
.type single_zero,@function
.align 16
single_zero:
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	fload f0, (r30)
.L1:
	rts
.size single_zero,.-single_zero
.section .text,"ax"
.global double_zero
.type double_zero,@function
.align 16
double_zero:
	move.l r30, #lo32(.L__ie64_fconst_1+0)
	movt r30, #hi32(.L__ie64_fconst_1+0)
	dload f0, (r30)
.L3:
	rts
.size double_zero,.-double_zero
.section .text,"ax"
.global both_zero
.type both_zero,@function
.align 16
both_zero:
	sub.q r31, r31, #8
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	fload f0, (r30)
	move.l r30, #lo32(.L__ie64_fconst_1+0)
	movt r30, #hi32(.L__ie64_fconst_1+0)
	dload f2, (r30)
	jsr consume_zeros
.L5:
	add.q r31, r31, #8
	rts
.size both_zero,.-both_zero
.section .text,"ax"
.local consume_zeros
.type consume_zeros,@function
.align 16
consume_zeros:
.L7:
	rts
.size consume_zeros,.-consume_zeros
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 4
.L__ie64_fconst_0:
	dc.l 0x00000000
.section .rodata,"a"
.local .L__ie64_fconst_1
.align 8
.L__ie64_fconst_1:
	dc.q 0x0000000000000000
