.section .text,"ax"
.global eq0_branch
.type eq0_branch,@function
.align 16
eq0_branch:
	move.l r1, r1
	beq r1, r0, .L2
.L1:
	moveq r1, #2
	rts
.L2:
	moveq r1, #1
	rts
.size eq0_branch,.-eq0_branch
.section .text,"ax"
.global slt0_branch
.type slt0_branch,@function
.align 16
slt0_branch:
	sext.l r1, r1
	blt r1, r0, .L5
.L4:
	moveq r1, #2
	rts
.L5:
	moveq r1, #1
	rts
.size slt0_branch,.-slt0_branch
.section .text,"ax"
.global zero_left
.type zero_left,@function
.align 16
zero_left:
	bgt r0, r1, .L8
.L7:
	moveq r1, #2
	rts
.L8:
	moveq r1, #1
	rts
.size zero_left,.-zero_left
.section .text,"ax"
.global ugt0_long
.type ugt0_long,@function
.align 16
ugt0_long:
	bhi r1, r0, .L11
.L10:
	moveq r1, #2
	rts
.L11:
	moveq r1, #1
	rts
.size ugt0_long,.-ugt0_long
.section .text,"ax"
.global uge0_long
.type uge0_long,@function
.align 16
uge0_long:
	bls r0, r1, .L14
.L13:
	sext.l r1, r2
	rts
.L14:
	moveq r1, #1
	rts
.size uge0_long,.-uge0_long
.section .text,"ax"
.global ne0_value
.type ne0_value,@function
.align 16
ne0_value:
	moveq r30, #0
	beq r1, r0, .Lcmp0
	moveq r30, #1
.Lcmp0:
	move.q r1, r30
	rts
.size ne0_value,.-ne0_value
.section .text,"ax"
.global ule0_wvalue
.type ule0_wvalue,@function
.align 16
ule0_wvalue:
	move.l r1, r1
	moveq r30, #0
	bhi r1, r0, .Lcmp1
	moveq r30, #1
.Lcmp1:
	move.q r1, r30
	rts
.size ule0_wvalue,.-ule0_wvalue
.section .text,"ax"
.global ne1_branch
.type ne1_branch,@function
.align 16
ne1_branch:
	moveq r2, #1
	bne r1, r2, .L19
.L18:
	moveq r1, #2
	rts
.L19:
	moveq r1, #1
	rts
.size ne1_branch,.-ne1_branch
