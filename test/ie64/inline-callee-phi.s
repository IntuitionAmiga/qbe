.section .text,"ax"
.local sign
.type sign,@function
.align 16
sign:
	sext.l r1, r1
	blt r1, r0, .L2
.L1:
	moveq r1, #1
	bra .L3
.L2:
	moveq r1, #0
.L3:
	rts
.size sign,.-sign
.section .text,"ax"
.global use
.type use,@function
.align 16
use:
	sext.l r1, r1
	blt r1, r0, .L6
.L5:
	moveq r1, #1
	bra .L7
.L6:
	moveq r1, #0
.L7:
	sext.l r2, r2
	blt r2, r0, .L9
.L8:
	moveq r2, #1
	bra .L10
.L9:
	moveq r2, #0
.L10:
	add.l r1, r1, r2
	sext.l r1, r1
	rts
.size use,.-use
