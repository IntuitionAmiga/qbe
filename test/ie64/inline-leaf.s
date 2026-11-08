.section .text,"ax"
.local max
.type max,@function
.align 16
max:
	sext.l r3, r1
	sext.l r4, r2
	bgt r3, r4, .L2
.L1:
	sext.l r1, r2
	rts
.L2:
	sext.l r1, r1
	rts
.size max,.-max
.section .text,"ax"
.local scaled
.type scaled,@function
.align 16
scaled:
	mulu.q r2, r2, #4
	add.q r1, r1, r2
	rts
.size scaled,.-scaled
.section .text,"ax"
.local nonleaf
.type nonleaf,@function
.align 16
nonleaf:
	sub.q r31, r31, #8
	sext.l r2, r1
	moveq r3, #3
	bgt r2, r3, .L6
.L5:
	moveq r1, #3
.L6:
	jsr external
	sext.l r1, r1
	add.q r31, r31, #8
	rts
.size nonleaf,.-nonleaf
.section .text,"ax"
.global caller
.type caller,@function
.align 16
caller:
	sub.q r31, r31, #8
	sext.l r4, r1
	sext.l r5, r2
	bgt r4, r5, .L9
.L8:
	move.l r1, r2
.L9:
	load.l r2, 12(r3)
	add.l r1, r2, r1
	jsr nonleaf
	sext.l r1, r1
	add.q r31, r31, #8
	rts
.size caller,.-caller
