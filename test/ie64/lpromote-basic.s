.section .text,"ax"
.global acc
.type acc,@function
.align 16
acc:
	move.q r5, r3
	move.q r30, r2
	move.q r2, r4
	move.q r4, r30
	move.l r10, r2
	lsl.q r2, r10, #1
	add.q r8, r4, r2
	moveq r9, #0
	moveq r13, #0
	bls r10, r9, .L8
.L3:
	lsl.q r2, r9, #2
	add.q r7, r1, r2
	store.l r13, (r7)
	moveq r6, #0
	move.q r2, r4
	move.q r3, r5
	beq r2, r8, .L7
.L6:
	load.w r11, (r2)
	sext.w r11, r11
	load.w r12, (r3)
	sext.w r12, r12
	mulu.l r11, r11, r12
	add.l r6, r11, r6
	store.l r6, (r7)
	add.q r2, r2, #2
	add.q r3, r3, #2
	bne r2, r8, .L6
.L7:
	add.l r9, r9, #1
	bhi r10, r9, .L3
.L8:
	rts
.size acc,.-acc
.section .text,"ax"
.global global
.type global,@function
.align 16
global:
	move.l r3, r1
	move.l r30, #lo32(total+0)
	movt r30, #hi32(total+0)
	load.q r2, (r30)
	moveq r1, #0
	bls r3, r1, .L13
.L12:
	move.q r4, r2
	add.q r2, r1, r4
	add.l r1, r1, #1
	bhi r3, r1, .L12
.L13:
	move.l r30, #lo32(total+0)
	movt r30, #hi32(total+0)
	store.q r2, (r30)
	moveq r1, #0
	rts
.size global,.-global
.section .text,"ax"
.global exitjoin
.type exitjoin,@function
.align 16
exitjoin:
	move.l r4, r2
	move.l r2, r3
	beq r2, r0, .L21
.L15:
	moveq r2, #1
	store.l r2, 8(r1)
	moveq r3, #0
	moveq r2, #1
	moveq r5, #1
	bls r4, r3, .L20
.L18:
	load.w r5, (r1)
	sext.w r5, r5
	add.l r2, r5, r2
	add.l r3, r3, #1
	move.l r5, r2
	bhi r4, r3, .L18
.L20:
	store.l r5, 8(r1)
.L21:
	load.l r1, 8(r1)
	sext.l r1, r1
	rts
.size exitjoin,.-exitjoin
