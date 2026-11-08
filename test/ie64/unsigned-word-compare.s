.section .text,"ax"
.global equal_min_signed
.type equal_min_signed,@function
.align 16
equal_min_signed:
	move.l r1, r1
	move.l r2, #0x80000000
	bne r1, r2, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	rts
.size equal_min_signed,.-equal_min_signed
