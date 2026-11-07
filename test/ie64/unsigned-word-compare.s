.section .text,"ax"
.global equal_min_signed
.type equal_min_signed,@function
.align 16
equal_min_signed:
	move.l r1, r1
	move.l r2, #0x80000000
	moveq r30, #0
	bne r1, r2, .Lcmp0
	moveq r30, #1
.Lcmp0:
	move.q r1, r30
	rts
.size equal_min_signed,.-equal_min_signed
