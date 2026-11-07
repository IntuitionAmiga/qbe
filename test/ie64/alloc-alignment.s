.section .text,"ax"
.global alloc_alignment
.type alloc_alignment,@function
.align 16
alloc_alignment:
	sub.q r31, r31, #40
	lea r3, 16(r31)
	lea r2, 8(r31)
	lea r1, 0(r31)
	jsr use
	add.q r31, r31, #40
	rts
.size alloc_alignment,.-alloc_alignment
.section .text,"ax"
.local use
.type use,@function
.align 16
use:
	rts
.size use,.-use
