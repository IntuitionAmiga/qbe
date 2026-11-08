.section .text,"ax"
.global f
.type f,@function
.align 16
f:
	sub.q r31, r31, #72
	store.q r18, 16(r31)
	store.q r19, 24(r31)
	store.q r20, 32(r31)
	store.q r21, 40(r31)
	store.q r22, 48(r31)
	store.q r23, 56(r31)
	store.q r24, 64(r31)
	move.q r18, r1
	move.q r20, r2
	store.l r3, 8(r31)
	move.q r1, r4
	move.q r19, r5
	move.q r23, r6
	load.q r22, 80(r31)
	load.q r21, 88(r31)
	jsr g
	move.q r30, r1
	move.q r1, r19
	move.q r19, r30
	jsr g
	move.q r24, r1
	move.q r1, r23
	jsr g
	move.q r23, r1
	move.q r1, r22
	jsr g
	move.q r22, r1
	move.q r1, r21
	jsr g
	move.q r21, r1
	move.q r1, r19
	add.q r2, r1, r24
	add.q r2, r23, r2
	add.q r2, r22, r2
	add.q r2, r21, r2
	store.q r2, 0(r31)
	move.q r19, r1
	move.q r1, r2
	jsr g
	move.q r2, r20
	move.q r3, r1
	move.q r1, r19
	move.q r19, r2
	load.q r2, 0(r31)
	load.l r20, 8(r31)
	add.q r2, r2, r3
	add.q r1, r1, r2
	add.q r1, r24, r1
	add.q r1, r23, r1
	add.q r1, r22, r1
	add.q r1, r21, r1
	jsr h
	move.l r3, r20
	move.q r2, r19
	move.l r4, r1
	move.q r1, r18
	beq r4, r0, .L5
.L1:
	add.l r18, r3, r4
.L2:
	sext.l r3, r18
	ble r3, r0, .L6
.L3:
	move.q r19, r1
	moveq r1, #32
	move.q r20, r2
	move.q r2, r19
	jsr (r20)
	move.q r2, r20
	move.q r3, r1
	move.q r1, r19
	blt r3, r0, .L5
.L4:
	sub.l r18, r18, #1
	bra .L2
.L5:
	moveq r1, #-1
	load.q r18, 16(r31)
	load.q r19, 24(r31)
	load.q r20, 32(r31)
	load.q r21, 40(r31)
	load.q r22, 48(r31)
	load.q r23, 56(r31)
	load.q r24, 64(r31)
	add.q r31, r31, #72
	rts
.L6:
	sext.l r1, r18
	load.q r18, 16(r31)
	load.q r19, 24(r31)
	load.q r20, 32(r31)
	load.q r21, 40(r31)
	load.q r22, 48(r31)
	load.q r23, 56(r31)
	load.q r24, 64(r31)
	add.q r31, r31, #72
	rts
.size f,.-f
