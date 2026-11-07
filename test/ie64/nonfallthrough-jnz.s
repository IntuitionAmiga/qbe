.section .text,"ax"
.global nonfallthrough_jnz
.type nonfallthrough_jnz,@function
.align 16
nonfallthrough_jnz:
	bne r1, r0, .L2
.L1:
	moveq r1, #0
	bra .L3
.L2:
	moveq r1, #1
.L3:
	rts
.size nonfallthrough_jnz,.-nonfallthrough_jnz
