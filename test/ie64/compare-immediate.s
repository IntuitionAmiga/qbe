.section .text,"ax"
.global greater_than_zero
.type greater_than_zero,@function
.align 16
greater_than_zero:
	sext.l r1, r1
	ble r1, r0, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	rts
.size greater_than_zero,.-greater_than_zero
