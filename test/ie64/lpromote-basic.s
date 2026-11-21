.section .text,"ax"
.global acc
.type acc,@function
.align 16
acc:
	move.l r6, r4
	moveq r7, #0
	moveq r11, #0
	bls r6, r7, .L9
.L3:
	lsl.q r4, r7, #2
	add.q r8, r1, r4
	store.l r11, (r8)
	moveq r5, #0
	moveq r4, #0
	moveq r9, #0
	bls r6, r5, .L8
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
	add.l r5, r5, #1
	move.l r9, r4
	bhi r6, r5, .L6
.L8:
	store.l r9, (r8)
	add.l r7, r7, #1
	bhi r6, r7, .L3
.L9:
	rts
.size acc,.-acc
.section .text,"ax"
.global global
.type global,@function
.align 16
global:
	move.l r4, r2
	move.l r30, #lo32(total+0)
	movt r30, #hi32(total+0)
	load.q r3, (r30)
	moveq r2, #0
	bls r4, r2, .L14
.L13:
	move.q r5, r3
	lsl.q r3, r2, #1
	add.q r3, r1, r3
	load.w r3, (r3)
	sext.w r3, r3
	sext.l r3, r3
	add.q r3, r3, r5
	add.l r2, r2, #1
	bhi r4, r2, .L13
.L14:
	move.l r30, #lo32(total+0)
	movt r30, #hi32(total+0)
	store.q r3, (r30)
	moveq r1, #0
	rts
.size global,.-global
.section .text,"ax"
.global exitjoin
.type exitjoin,@function
.align 16
exitjoin:
	move.l r5, r3
	move.l r3, r4
	beq r3, r0, .L22
.L16:
	moveq r3, #1
	store.l r3, 8(r1)
	moveq r4, #0
	moveq r3, #1
	moveq r6, #1
	bls r5, r4, .L21
.L19:
	lsl.q r6, r4, #1
	add.q r6, r2, r6
	load.w r6, (r6)
	sext.w r6, r6
	add.l r3, r6, r3
	add.l r4, r4, #1
	move.l r6, r3
	bhi r5, r4, .L19
.L21:
	store.l r6, 8(r1)
.L22:
	load.l r1, 8(r1)
	sext.l r1, r1
	rts
.size exitjoin,.-exitjoin
