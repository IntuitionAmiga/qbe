.section .text,"ax"
.global wide_cond
.type wide_cond,@function
.align 16
wide_cond:
	move.l r1, r1
	bne r1, r0, .L2
.L1:
	moveq r1, #6
	rts
.L2:
	moveq r1, #5
	rts
.size wide_cond,.-wide_cond
.section .text,"ax"
.global wide_copy
.type wide_copy,@function
.align 16
wide_copy:
	move.l r1, r1
	bne r1, r0, .L5
.L4:
	moveq r1, #6
	rts
.L5:
	moveq r1, #5
	rts
.size wide_copy,.-wide_copy
.section .text,"ax"
.global wide_forward
.type wide_forward,@function
.align 16
wide_forward:
	sub.q r31, r31, #8
	lea r3, 0(r31)
	add.q r4, r2, r3
	moveq r2, #0
	store.l r2, (r4)
	store.l r1, (r3)
	move.l r1, r1
	bne r1, r0, .L8
.L7:
	moveq r1, #6
	add.q r31, r31, #8
	rts
.L8:
	moveq r1, #5
	add.q r31, r31, #8
	rts
.size wide_forward,.-wide_forward
.section .text,"ax"
.global wide_phi
.type wide_phi,@function
.align 16
wide_phi:
	bne r1, r0, .L12
.L10:
	move.q r2, r3
	move.l r1, r2
	bra .L13
.L12:
	move.l r1, r2
.L13:
	move.l r2, r1
	bne r2, r0, .L15
.L14:
	moveq r1, #6
	rts
.L15:
	moveq r1, #5
	rts
.size wide_phi,.-wide_phi
.section .text,"ax"
.global wide_phi_chain
.type wide_phi_chain,@function
.align 16
wide_phi_chain:
	move.q r2, r4
	bne r1, r0, .L19
.L17:
	move.q r3, r2
	move.l r1, r3
	bra .L20
.L19:
	move.l r1, r3
.L20:
	move.l r3, r1
	bne r3, r0, .L23
.L22:
	moveq r1, #6
	rts
.L23:
	moveq r1, #5
	rts
.size wide_phi_chain,.-wide_phi_chain
.section .text,"ax"
.global narrow_cond
.type narrow_cond,@function
.align 16
narrow_cond:
	bge r2, r3, .L26
.L25:
	bne r1, r0, .L27
.L26:
	moveq r1, #6
	rts
.L27:
	moveq r1, #5
	rts
.size narrow_cond,.-narrow_cond
