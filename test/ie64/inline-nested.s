.section .text,"ax"
.local leaf
.type leaf,@function
.align 16
leaf:
	and.q r2, r2, #0xffff
	store.w r2, (r1)
.L1:
	rts
.size leaf,.-leaf
.section .text,"ax"
.local middle
.type middle,@function
.align 16
middle:
	sub.q r31, r31, #8
	and.q r2, r2, #0xffff
	store.w r2, (r1)
.L3:
	add.q r31, r31, #8
	rts
.size middle,.-middle
.section .text,"ax"
.global top
.type top,@function
.align 16
top:
	sub.q r31, r31, #8
	and.q r2, r2, #0xffff
	store.w r2, (r1)
.L5:
	add.q r31, r31, #8
	rts
.size top,.-top
