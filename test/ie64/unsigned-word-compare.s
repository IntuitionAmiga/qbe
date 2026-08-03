.section .text,"ax"
.global equal_min_signed
.type equal_min_signed,@function
.align 16
equal_min_signed:
	move.l r1, r1
	li r2, #-2147483648
	move.l r2, r2
	li r30, #0
	bne r1, r2, .Lcmp0
	li r30, #1
.Lcmp0:
	move.q r1, r30
	move.l r1, r1
	sext.l r1, r1
	rts
.size equal_min_signed,.-equal_min_signed
