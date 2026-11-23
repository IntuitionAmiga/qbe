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
	move.q r30, r1
	move.q r1, r5
	move.q r5, r30
	bne r5, r0, .L19
.L17:
	move.q r3, r4
	move.l r3, r3
	bra .L20
.L19:
	move.l r3, r3
.L20:
	bne r2, r0, .L24
.L21:
	move.q r3, r1
	move.l r3, r3
.L24:
	bne r3, r0, .L26
.L25:
	moveq r1, #6
	rts
.L26:
	moveq r1, #5
	rts
.size wide_phi_chain,.-wide_phi_chain
.section .text,"ax"
.global narrow_cond
.type narrow_cond,@function
.align 16
narrow_cond:
	load.l r1, (r1)
	bne r1, r0, .L29
.L28:
	moveq r1, #6
	rts
.L29:
	moveq r1, #5
	rts
.size narrow_cond,.-narrow_cond
.section .text,"ax"
.global narrow_cmp
.type narrow_cmp,@function
.align 16
narrow_cmp:
	bge r2, r3, .L32
.L31:
	bne r1, r0, .L33
.L32:
	moveq r1, #6
	rts
.L33:
	moveq r1, #5
	rts
.size narrow_cmp,.-narrow_cmp
.section .text,"ax"
.global wide_callee
.type wide_callee,@function
.align 16
wide_callee:
	bne r1, r0, .L36
.L35:
	moveq r1, #6
	rts
.L36:
	moveq r1, #5
	rts
.size wide_callee,.-wide_callee
.section .text,"ax"
.global wide_call
.type wide_call,@function
.align 16
wide_call:
	sub.q r31, r31, #8
	move.l r1, r1
	jsr wide_callee
	sext.l r1, r1
	add.q r31, r31, #8
	rts
.size wide_call,.-wide_call
.section .text,"ax"
.global wide_call_phi
.type wide_call_phi,@function
.align 16
wide_call_phi:
	sub.q r31, r31, #8
	move.q r30, r1
	move.q r1, r3
	move.q r3, r30
	move.q r30, r2
	move.q r2, r1
	move.q r1, r30
	bne r3, r0, .L41
.L39:
	move.q r1, r2
	move.l r1, r1
	bra .L42
.L41:
	move.l r1, r1
.L42:
	jsr wide_callee
	sext.l r1, r1
	add.q r31, r31, #8
	rts
.size wide_call_phi,.-wide_call_phi
.section .text,"ax"
.global wide_const
.type wide_const,@function
.align 16
wide_const:
	moveq r1, #6
	rts
.size wide_const,.-wide_const
.section .text,"ax"
.global wide_symbol_phi
.type wide_symbol_phi,@function
.align 16
wide_symbol_phi:
	bne r1, r0, .L46
.L45:
	move.l r1, r2
	bra .L47
.L46:
	move.l r1, #lo32(sym+0)
	movt r1, #hi32(sym+0)
	move.l r1, r1
.L47:
	move.l r2, r1
	bne r2, r0, .L49
.L48:
	moveq r1, #6
	rts
.L49:
	moveq r1, #5
	rts
.size wide_symbol_phi,.-wide_symbol_phi
.section .text,"ax"
.global wide_symbol
.type wide_symbol,@function
.align 16
wide_symbol:
	move.l r1, #lo32(sym+0)
	movt r1, #hi32(sym+0)
	move.l r1, r1
	bne r1, r0, .L52
.L51:
	moveq r1, #6
	rts
.L52:
	moveq r1, #5
	rts
.size wide_symbol,.-wide_symbol
.section .text,"ax"
.global wide_false_edge
.type wide_false_edge,@function
.align 16
wide_false_edge:
	move.l r2, r1
	bne r2, r0, .L55
.L54:
	rts
.L55:
	moveq r1, #5
	rts
.size wide_false_edge,.-wide_false_edge
.section .data,"aw"
.local sym
.type sym,@object
.align 8
sym:
	dc.l 0
.size sym,4
