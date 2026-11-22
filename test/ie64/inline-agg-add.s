.section .text,"ax"
.local vadd
.type vadd,@function
.align 16
vadd:
	sub.q r31, r31, #24
	move.q r10, r2
	move.q r2, r1
	move.q r9, r3
	fload f0, (r10)
	fload f1, (r9)
	fadd f0, f0, f1
	fstore f0, (r31)
	fload f0, 4(r10)
	fload f1, 4(r9)
	fadd f0, f0, f1
	fstore f0, 4(r31)
	fload f0, 8(r10)
	fload f1, 8(r9)
	fadd f0, f0, f1
	fstore f0, 8(r31)
	fload f0, 12(r10)
	fload f1, 12(r9)
	fadd f0, f0, f1
	fstore f0, 12(r31)
	load.q r1, (r31)
	store.q r1, (r2)
	load.q r1, 8(r31)
	store.q r1, 8(r2)
	add.q r31, r31, #24
	rts
.size vadd,.-vadd
.section .text,"ax"
.global caller
.type caller,@function
.align 16
caller:
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	fload f1, (r30)
	fadd f3, f0, f1
	move.l r30, #lo32(.L__ie64_fconst_1+0)
	movt r30, #hi32(.L__ie64_fconst_1+0)
	fload f1, (r30)
	fadd f2, f0, f1
	move.l r30, #lo32(.L__ie64_fconst_2+0)
	movt r30, #hi32(.L__ie64_fconst_2+0)
	fload f1, (r30)
	fadd f1, f0, f1
	move.l r30, #lo32(.L__ie64_fconst_3+0)
	movt r30, #hi32(.L__ie64_fconst_3+0)
	fload f4, (r30)
	fadd f0, f0, f4
	fmovo r2, f3
	store.l r2, (r1)
	fmovo r2, f2
	store.l r2, 4(r1)
	fmovo r2, f1
	store.l r2, 8(r1)
	add.q r2, r1, #12
	fmovo r1, f0
	store.l r1, (r2)
	rts
.size caller,.-caller
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 4
.L__ie64_fconst_0:
	dc.l 0x3f800000
.section .rodata,"a"
.local .L__ie64_fconst_1
.align 4
.L__ie64_fconst_1:
	dc.l 0x40000000
.section .rodata,"a"
.local .L__ie64_fconst_2
.align 4
.L__ie64_fconst_2:
	dc.l 0x40400000
.section .rodata,"a"
.local .L__ie64_fconst_3
.align 4
.L__ie64_fconst_3:
	dc.l 0x40800000
