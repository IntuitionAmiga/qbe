.section .text,"ax"
.global intfirst_s
.type intfirst_s,@function
.align 16
intfirst_s:
	sub.q r31, r31, #8
	moveq r1, #1
	jsr tag
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	fload f0, (r30)
	jsr value
	add.q r31, r31, #8
	rts
.size intfirst_s,.-intfirst_s
.section .text,"ax"
.global floatfirst_s
.type floatfirst_s,@function
.align 16
floatfirst_s:
	sub.q r31, r31, #8
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	fload f0, (r30)
	jsr value
	moveq r1, #1
	jsr tag
	moveq r1, #2
	jsr tag
	add.q r31, r31, #8
	rts
.size floatfirst_s,.-floatfirst_s
.section .text,"ax"
.global intfirst_one
.type intfirst_one,@function
.align 16
intfirst_one:
	sub.q r31, r31, #8
	moveq r1, #1065353216
	jsr tag
	move.l r30, #lo32(.L__ie64_fconst_1+0)
	movt r30, #hi32(.L__ie64_fconst_1+0)
	fload f0, (r30)
	jsr value
	add.q r31, r31, #8
	rts
.size intfirst_one,.-intfirst_one
.section .text,"ax"
.global floatfirst_one
.type floatfirst_one,@function
.align 16
floatfirst_one:
	sub.q r31, r31, #8
	move.l r30, #lo32(.L__ie64_fconst_1+0)
	movt r30, #hi32(.L__ie64_fconst_1+0)
	fload f0, (r30)
	jsr value
	moveq r1, #1065353216
	jsr tag
	add.q r31, r31, #8
	rts
.size floatfirst_one,.-floatfirst_one
.section .text,"ax"
.global intfirst_d
.type intfirst_d,@function
.align 16
intfirst_d:
	sub.q r31, r31, #8
	moveq r1, #1
	jsr tagl
	move.l r30, #lo32(.L__ie64_fconst_2+0)
	movt r30, #hi32(.L__ie64_fconst_2+0)
	dload f0, (r30)
	jsr valued
	add.q r31, r31, #8
	rts
.size intfirst_d,.-intfirst_d
.section .text,"ax"
.global floatfirst_d
.type floatfirst_d,@function
.align 16
floatfirst_d:
	sub.q r31, r31, #8
	move.l r30, #lo32(.L__ie64_fconst_2+0)
	movt r30, #hi32(.L__ie64_fconst_2+0)
	dload f0, (r30)
	jsr valued
	moveq r1, #1
	jsr tagl
	add.q r31, r31, #8
	rts
.size floatfirst_d,.-floatfirst_d
.section .text,"ax"
.global intfirst_done
.type intfirst_done,@function
.align 16
intfirst_done:
	sub.q r31, r31, #8
	move.l r1, #0x00000000
	movt r1, #0x3ff00000
	jsr tagl
	move.l r30, #lo32(.L__ie64_fconst_3+0)
	movt r30, #hi32(.L__ie64_fconst_3+0)
	dload f0, (r30)
	jsr valued
	add.q r31, r31, #8
	rts
.size intfirst_done,.-intfirst_done
.section .text,"ax"
.global floatfirst_done
.type floatfirst_done,@function
.align 16
floatfirst_done:
	sub.q r31, r31, #8
	move.l r30, #lo32(.L__ie64_fconst_3+0)
	movt r30, #hi32(.L__ie64_fconst_3+0)
	dload f0, (r30)
	jsr valued
	move.l r1, #0x00000000
	movt r1, #0x3ff00000
	jsr tagl
	add.q r31, r31, #8
	rts
.size floatfirst_done,.-floatfirst_done
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 4
.L__ie64_fconst_0:
	dc.l 0x00000001
.section .rodata,"a"
.local .L__ie64_fconst_1
.align 4
.L__ie64_fconst_1:
	dc.l 0x3f800000
.section .rodata,"a"
.local .L__ie64_fconst_2
.align 8
.L__ie64_fconst_2:
	dc.q 0x0000000000000001
.section .rodata,"a"
.local .L__ie64_fconst_3
.align 8
.L__ie64_fconst_3:
	dc.q 0x3ff0000000000000
