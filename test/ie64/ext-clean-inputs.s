.section .text,"ax"
.global shr_signed_byte
.type shr_signed_byte,@function
.align 16
shr_signed_byte:
	sext.b r1, r1
	move.l r1, r1
	lsr.l r1, r1, #4
	rts
.size shr_signed_byte,.-shr_signed_byte
.section .text,"ax"
.global shr_param
.type shr_param,@function
.align 16
shr_param:
	move.l r1, r1
	lsr.l r1, r1, #3
	rts
.size shr_param,.-shr_param
.section .text,"ax"
.global sdiv_sum
.type sdiv_sum,@function
.align 16
sdiv_sum:
	add.l r1, r1, r2
	sext.l r2, r3
	sext.l r1, r1
	divs.l r1, r1, r2
	sext.l r1, r1
	rts
.size sdiv_sum,.-sdiv_sum
.section .text,"ax"
.global sdiv_neg_const
.type sdiv_neg_const,@function
.align 16
sdiv_neg_const:
	moveq r2, #-5
	sext.l r1, r1
	divs.l r1, r1, r2
	sext.l r1, r1
	rts
.size sdiv_neg_const,.-sdiv_neg_const
.section .text,"ax"
.global udiv_sum
.type udiv_sum,@function
.align 16
udiv_sum:
	sub.l r1, r1, r2
	move.l r2, r3
	divu.l r1, r1, r2
	sext.l r1, r1
	rts
.size udiv_sum,.-udiv_sum
.section .text,"ax"
.global swtod_sum
.type swtod_sum,@function
.align 16
swtod_sum:
	add.l r1, r1, r2
	sext.l r1, r1
	dcvtif f0, r1
.L6:
	rts
.size swtod_sum,.-swtod_sum
