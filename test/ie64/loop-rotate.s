.section .text,"ax"
.global sum
.type sum,@function
.align 16
sum:
	moveq r4, #0
	moveq r3, #0
	sext.l r5, r3
	sext.l r6, r2
	bge r5, r6, .L4
.L3:
	sext.l r5, r3
	lsl.q r5, r5, #2
	add.q r5, r1, r5
	load.l r5, (r5)
	add.l r4, r4, r5
	add.l r3, r3, #1
	sext.l r5, r3
	sext.l r6, r2
	blt r5, r6, .L3
.L4:
	sext.l r1, r4
	rts
.size sum,.-sum
