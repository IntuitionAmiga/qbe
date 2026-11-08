.section .text,"ax"
.global chain
.type chain,@function
.align 16
chain:
	sub.q r31, r31, #72
	store.l r1, 48(r31)
	bne r2, r0, .L2
.L1:
	sext.l r1, r1
	add.q r31, r31, #72
	rts
.L2:
	lea r1, 0(r31)
	jsr use
	sext.l r1, r1
	add.q r31, r31, #72
	rts
.size chain,.-chain
