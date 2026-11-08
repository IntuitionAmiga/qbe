.section .text,"ax"
.global slt_const
.type slt_const,@function
.align 16
slt_const:
	sext.l r1, r1
	moveq r2, #-5
	bge r1, r2, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	rts
.size slt_const,.-slt_const
.section .text,"ax"
.global sgt_const_left
.type sgt_const_left,@function
.align 16
sgt_const_left:
	move.l r2, r1
	moveq r1, #100
	sext.l r2, r2
	ble r1, r2, .Lcmp1
	moveq r1, #1
	bra .Lcmpe1
.Lcmp1:
	moveq r1, #0
.Lcmpe1:
	rts
.size sgt_const_left,.-sgt_const_left
.section .text,"ax"
.global ult_const
.type ult_const,@function
.align 16
ult_const:
	move.l r1, r1
	move.l r2, #0xee6b2800
	bls r2, r1, .Lcmp2
	moveq r1, #1
	bra .Lcmpe2
.Lcmp2:
	moveq r1, #0
.Lcmpe2:
	rts
.size ult_const,.-ult_const
.section .text,"ax"
.global eq_const
.type eq_const,@function
.align 16
eq_const:
	move.l r1, r1
	move.l r2, #0xffffffff
	bne r1, r2, .Lcmp3
	moveq r1, #1
	bra .Lcmpe3
.Lcmp3:
	moveq r1, #0
.Lcmpe3:
	rts
.size eq_const,.-eq_const
.section .text,"ax"
.global both_const
.type both_const,@function
.align 16
both_const:
	moveq r1, #1
	rts
.size both_const,.-both_const
.section .text,"ax"
.global branch_const
.type branch_const,@function
.align 16
branch_const:
	sext.l r1, r1
	moveq r2, #9
	ble r1, r2, .L7
.L6:
	moveq r1, #0
	rts
.L7:
	moveq r1, #1
	rts
.size branch_const,.-branch_const
