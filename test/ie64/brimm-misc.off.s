.section .text,"ax"
.global value_only
.type value_only,@function
.align 16
value_only:
	sext.l r1, r1
	moveq r2, #7
	bge r1, r2, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	rts
.size value_only,.-value_only
.section .text,"ax"
.global branch_and_value
.type branch_and_value,@function
.align 16
branch_and_value:
	sext.l r1, r1
	moveq r2, #7
	bge r1, r2, .Lcmp1
	moveq r1, #1
	bra .Lcmpe1
.Lcmp1:
	moveq r1, #0
.Lcmpe1:
	bne r1, r0, .L3
.L2:
	moveq r1, #2
	rts
.L3:
	rts
.size branch_and_value,.-branch_and_value
.section .text,"ax"
.global zero_ext
.type zero_ext,@function
.align 16
zero_ext:
	load.b r1, (r1)
	moveq r2, #100
	bhi r2, r1, .L6
.L5:
	moveq r1, #2
	rts
.L6:
	moveq r1, #1
	rts
.size zero_ext,.-zero_ext
.section .text,"ax"
.global loop
.type loop,@function
.align 16
loop:
	moveq r1, #0
	moveq r4, #100
.L9:
	add.l r1, r1, #1
	sext.l r2, r1
	blt r2, r4, .L9
.L10:
	sext.l r1, r1
	rts
.size loop,.-loop
.section .text,"ax"
.global two_regs
.type two_regs,@function
.align 16
two_regs:
	sext.l r1, r1
	sext.l r2, r2
	blt r1, r2, .L13
.L12:
	moveq r1, #2
	rts
.L13:
	moveq r1, #1
	rts
.size two_regs,.-two_regs
.section .text,"ax"
.global zero
.type zero,@function
.align 16
zero:
	move.l r1, r1
	beq r1, r0, .L16
.L15:
	moveq r1, #2
	rts
.L16:
	moveq r1, #1
	rts
.size zero,.-zero
