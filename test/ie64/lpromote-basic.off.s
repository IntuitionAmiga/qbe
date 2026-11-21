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
	move.l r3, r2
	moveq r2, #0
	bls r3, r2, .L13
.L12:
	lsl.q r4, r2, #1
	add.q r4, r1, r4
	load.w r4, (r4)
	sext.w r4, r4
	sext.l r4, r4
	move.l r30, #lo32(total+0)
	movt r30, #hi32(total+0)
	load.q r5, (r30)
	add.q r4, r4, r5
	move.l r30, #lo32(total+0)
	movt r30, #hi32(total+0)
	store.q r4, (r30)
	add.l r2, r2, #1
	bhi r3, r2, .L12
.L13:
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
	beq r3, r0, .L19
.L15:
	moveq r3, #1
	store.l r3, 8(r1)
	moveq r4, #0
	moveq r3, #1
	bls r5, r4, .L19
.L18:
	lsl.q r6, r4, #1
	add.q r6, r2, r6
	load.w r6, (r6)
	sext.w r6, r6
	add.l r3, r6, r3
	store.l r3, 8(r1)
	add.l r4, r4, #1
	bhi r5, r4, .L18
.L19:
	load.l r1, 8(r1)
	sext.l r1, r1
	rts
.size exitjoin,.-exitjoin
