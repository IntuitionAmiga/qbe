.section .text,"ax"
.global constant
.type constant,@function
.align 16
constant:
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f0, (r30)
.L1:
	rts
.size constant,.-constant
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 8
.L__ie64_fconst_0:
	dc.q 0x3ff8000000000000
