.section .text,"ax"
.global scale
.type scale,@function
.align 16
scale:
	lsl.q r1, r1, #3
	lsl.l r3, r2, #4
	add.q r1, r1, r3
	mulu.q r1, r1, #12
	mulu.l r2, r2, #2147483648
	add.q r1, r1, r2
	rts
.size scale,.-scale
