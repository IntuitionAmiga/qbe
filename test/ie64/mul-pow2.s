.section .text,"ax"
.global scale
.type scale,@function
.align 16
scale:
	lsl.q r3, r1, #3
	lsl.l r4, r2, #4
	add.q r3, r3, r4
	mulu.q r3, r3, #12
	mulu.l r4, r2, #2147483648
	add.q r1, r3, r4
	rts
.size scale,.-scale
