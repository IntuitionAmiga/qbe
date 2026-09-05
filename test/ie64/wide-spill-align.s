.section .text,"ax"
.global wide_spill
.type wide_spill,@function
.align 16
wide_spill:
	sub.q r31, r31, #136
	store.q r18, 80(r31)
	store.q r19, 88(r31)
	store.q r20, 96(r31)
	store.q r21, 104(r31)
	store.q r22, 112(r31)
	store.q r23, 120(r31)
	store.q r24, 128(r31)
	lea r1, 0(r31)
	move.l r30, #lo32(g0+0)
	movt r30, #hi32(g0+0)
	load.q r23, (r30)
	move.l r30, #lo32(g1+0)
	movt r30, #hi32(g1+0)
	load.q r24, (r30)
	move.l r30, #lo32(g2+0)
	movt r30, #hi32(g2+0)
	load.q r21, (r30)
	move.l r30, #lo32(g3+0)
	movt r30, #hi32(g3+0)
	load.q r22, (r30)
	move.l r30, #lo32(g4+0)
	movt r30, #hi32(g4+0)
	load.q r19, (r30)
	move.l r30, #lo32(g5+0)
	movt r30, #hi32(g5+0)
	load.q r20, (r30)
	move.l r30, #lo32(g6+0)
	movt r30, #hi32(g6+0)
	load.q r18, (r30)
	move.l r30, #lo32(g7+0)
	movt r30, #hi32(g7+0)
	load.q r6, (r30)
	lea r30, 8(r31)
	store.q r6, (r30)
	move.l r30, #lo32(g8+0)
	movt r30, #hi32(g8+0)
	load.q r3, (r30)
	lea r30, 16(r31)
	store.q r3, (r30)
	move.l r30, #lo32(g9+0)
	movt r30, #hi32(g9+0)
	load.q r10, (r30)
	lea r30, 24(r31)
	store.q r10, (r30)
	move.l r30, #lo32(g10+0)
	movt r30, #hi32(g10+0)
	load.q r5, (r30)
	lea r30, 32(r31)
	store.q r5, (r30)
	move.l r30, #lo32(g11+0)
	movt r30, #hi32(g11+0)
	load.q r9, (r30)
	lea r30, 40(r31)
	store.q r9, (r30)
	move.l r30, #lo32(g12+0)
	movt r30, #hi32(g12+0)
	load.q r2, (r30)
	lea r30, 48(r31)
	store.q r2, (r30)
	move.l r30, #lo32(g13+0)
	movt r30, #hi32(g13+0)
	load.q r8, (r30)
	lea r30, 56(r31)
	store.q r8, (r30)
	jsr sink
	load.q r8, 56(r31)
	load.q r2, 48(r31)
	load.q r9, 40(r31)
	load.q r5, 32(r31)
	load.q r10, 24(r31)
	load.q r3, 16(r31)
	load.q r6, 8(r31)
	add.q r1, r23, r24
	add.q r7, r21, r22
	add.q r4, r19, r20
	add.q r6, r18, r6
	add.q r3, r3, r10
	add.q r5, r5, r9
	add.q r2, r2, r8
	add.q r1, r1, r7
	add.q r4, r4, r6
	add.q r3, r3, r5
	add.q r1, r1, r4
	add.q r2, r2, r3
	add.q r1, r1, r2
	move.q r1, r1
	load.q r18, 80(r31)
	load.q r19, 88(r31)
	load.q r20, 96(r31)
	load.q r21, 104(r31)
	load.q r22, 112(r31)
	load.q r23, 120(r31)
	load.q r24, 128(r31)
	add.q r31, r31, #136
	rts
.size wide_spill,.-wide_spill
