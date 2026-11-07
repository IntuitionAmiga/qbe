.section .text,"ax"
.global greater_than_zero
.type greater_than_zero,@function
.align 16
greater_than_zero:
	sext.l r1, r1
	moveq r2, #0
	moveq r30, #0
	ble r1, r2, .Lcmp0
	moveq r30, #1
.Lcmp0:
	move.q r1, r30
	rts
.size greater_than_zero,.-greater_than_zero
