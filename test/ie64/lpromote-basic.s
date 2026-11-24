.section .text,"ax"
.global acc
.type acc,@function
.align 16
acc:
	move.l r7, r4
	moveq r8, #0
	moveq r11, #0
	bls r7, r8, .L8
.L3:
	lsl.q r4, r8, #2
	add.q r6, r1, r4
	store.l r11, (r6)
	moveq r5, #0
	moveq r4, #0
	bls r7, r5, .L7
.L6:
	move.q r9, r5
	lsl.q r10, r9, #1
	add.q r9, r2, r10
	add.q r10, r3, r10
	load.w r9, (r9)
	sext.w r9, r9
	load.w r10, (r10)
	sext.w r10, r10
	mulu.l r9, r9, r10
	add.l r4, r9, r4
	store.l r4, (r6)
	add.l r5, r5, #1
	bhi r7, r5, .L6
.L7:
	add.l r8, r8, #1
	bhi r7, r8, .L3
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
