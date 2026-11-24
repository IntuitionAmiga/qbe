.section .text,"ax"
.global union_unknown
.type union_unknown,@function
.align 16
union_unknown:
	move.l r6, r3
	move.l r3, #0x00000000
	movt r3, #0x00000001
	store.q r3, (r1)
	moveq r5, #0
	moveq r4, #0
	move.l r3, #0x00000000
	movt r3, #0x00000001
	move.l r8, #0x00000000
	movt r8, #0x00000001
	bls r6, r4, .L4
.L3:
	add.q r3, r3, r8
	store.q r3, (r1)
	load.l r7, 4(r2)
	add.l r5, r5, r7
	add.l r4, r4, #1
	bhi r6, r4, .L3
.L4:
	sext.l r1, r5
	rts
.size union_unknown,.-union_unknown
.section .text,"ax"
.global union_overlap
.type union_overlap,@function
.align 16
union_overlap:
	move.l r4, r2
	move.l r2, #0x00000000
	movt r2, #0x00000001
	store.q r2, (r1)
	moveq r2, #0
	move.l r3, #0x00000000
	movt r3, #0x00000001
	move.l r6, #0x00000000
	movt r6, #0x00000001
	bls r4, r2, .L9
.L8:
	add.q r5, r3, r6
	store.q r5, (r1)
	store.l r2, 4(r1)
	move.l r3, r2
	add.l r2, r2, #1
	lsl.q r3, r3, #32
	and.q r5, r5, #4294967295
	or.q r3, r3, r5
	bhi r4, r2, .L8
.L9:
	rts
.size union_overlap,.-union_overlap
.section .text,"ax"
.global disjoint
.type disjoint,@function
.align 16
disjoint:
	move.l r5, r2
	move.l r2, #0x00000000
	movt r2, #0x00000001
	store.q r2, (r1)
	moveq r4, #0
	moveq r3, #0
	move.l r2, #0x00000000
	movt r2, #0x00000001
	move.l r6, #0x00000000
	movt r6, #0x00000001
	move.l r8, #0x00000000
	movt r8, #0x00000001
	bls r5, r3, .L15
.L13:
	add.q r2, r2, r8
	load.l r6, 8(r1)
	add.l r4, r4, r6
	add.l r3, r3, #1
	move.q r6, r2
	bhi r5, r3, .L13
.L15:
	store.q r6, (r1)
	sext.l r1, r4
	rts
.size disjoint,.-disjoint
