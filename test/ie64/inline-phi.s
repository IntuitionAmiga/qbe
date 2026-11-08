.section .text,"ax"
.local twice
.type twice,@function
.align 16
twice:
	add.l r1, r1, r1
	sext.l r1, r1
	rts
.size twice,.-twice
.section .text,"ax"
.global caller
.type caller,@function
.align 16
caller:
	add.l r1, r1, r1
	bne r2, r0, .L3
.L2:
	sub.l r1, r1, #1
	bra .L4
.L3:
	add.l r1, r1, r1
.L4:
	sext.l r1, r1
	rts
.size caller,.-caller
