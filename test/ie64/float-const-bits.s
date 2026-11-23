.section .text,"ax"
.global bits_neg_zero
.type bits_neg_zero,@function
.align 16
bits_neg_zero:
	move.l r30, #0x80000000
	fmovi f0, r30
	rts
.size bits_neg_zero,.-bits_neg_zero
.section .text,"ax"
.global bits_nan
.type bits_nan,@function
.align 16
bits_nan:
	moveq r30, #2143289344
	fmovi f0, r30
	rts
.size bits_nan,.-bits_nan
.section .text,"ax"
.global bits_neg_nan
.type bits_neg_nan,@function
.align 16
bits_neg_nan:
	move.l r30, #0xffc00000
	fmovi f0, r30
	rts
.size bits_neg_nan,.-bits_neg_nan
.section .text,"ax"
.global bits_inf
.type bits_inf,@function
.align 16
bits_inf:
	moveq r30, #2139095040
	fmovi f0, r30
	rts
.size bits_inf,.-bits_inf
.section .text,"ax"
.global bits_neg_inf
.type bits_neg_inf,@function
.align 16
bits_neg_inf:
	move.l r30, #0xff800000
	fmovi f0, r30
	rts
.size bits_neg_inf,.-bits_neg_inf
.section .text,"ax"
.global bits_sub_neg
.type bits_sub_neg,@function
.align 16
bits_sub_neg:
	move.l r30, #0x80000001
	fmovi f0, r30
	rts
.size bits_sub_neg,.-bits_sub_neg
.section .text,"ax"
.global bits_sub_big
.type bits_sub_big,@function
.align 16
bits_sub_big:
	moveq r30, #8388607
	fmovi f0, r30
	rts
.size bits_sub_big,.-bits_sub_big
.section .text,"ax"
.global bits_min_normal
.type bits_min_normal,@function
.align 16
bits_min_normal:
	moveq r30, #8388608
	fmovi f0, r30
	rts
.size bits_min_normal,.-bits_min_normal
.section .text,"ax"
.global bits_neg_max
.type bits_neg_max,@function
.align 16
bits_neg_max:
	move.l r30, #0xff7fffff
	fmovi f0, r30
	rts
.size bits_neg_max,.-bits_neg_max
.section .text,"ax"
.global bits_neg_one
.type bits_neg_one,@function
.align 16
bits_neg_one:
	move.l r30, #0xbf800000
	fmovi f0, r30
	rts
.size bits_neg_one,.-bits_neg_one
.section .text,"ax"
.global bits_three
.type bits_three,@function
.align 16
bits_three:
	moveq r30, #1077936128
	fmovi f0, r30
	rts
.size bits_three,.-bits_three
.section .text,"ax"
.global bits_small_int
.type bits_small_int,@function
.align 16
bits_small_int:
	moveq r30, #2
	fmovi f0, r30
	rts
.size bits_small_int,.-bits_small_int
.section .text,"ax"
.global bits_one_third
.type bits_one_third,@function
.align 16
bits_one_third:
	moveq r30, #1051372203
	fmovi f0, r30
	rts
.size bits_one_third,.-bits_one_third
.section .text,"ax"
.global bits_between
.type bits_between,@function
.align 16
bits_between:
	moveq r30, #1077936128
	fmovi f2, r30
	fmul f0, f0, f2
	move.l r30, #0xbe800000
	fmovi f2, r30
	fsub f0, f0, f2
	fdiv f0, f0, f1
	rts
.size bits_between,.-bits_between
.section .text,"ax"
.global bits_union
.type bits_union,@function
.align 16
bits_union:
	fmovecr f0, #0
	rts
.size bits_union,.-bits_union
.section .text,"ax"
.global bits_snan
.type bits_snan,@function
.align 16
bits_snan:
	moveq r30, #2139095041
	fmovi f0, r30
	rts
.size bits_snan,.-bits_snan
.section .text,"ax"
.global double_unchanged
.type double_unchanged,@function
.align 16
double_unchanged:
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f0, (r30)
	rts
.size double_unchanged,.-double_unchanged
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 8
.L__ie64_fconst_0:
	dc.q 0x3ff0000000000000
