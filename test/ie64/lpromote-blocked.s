.section .text,"ax"
.global samewidth
.type samewidth,@function
.align 16
samewidth:
	moveq r4, #0
	store.l r4, (r1)
	move.l r3, r3
	lsl.q r3, r3, #2
	add.q r4, r2, r3
	moveq r3, #0
	beq r2, r4, .L4
.L3:
	load.l r5, (r2)
	add.l r3, r5, r3
	store.l r3, (r1)
	add.q r2, r2, #4
	bne r2, r4, .L3
.L4:
	rts
.size samewidth,.-samewidth
.section .text,"ax"
.global byte
.type byte,@function
.align 16
byte:
	moveq r4, #0
	store.l r4, (r1)
	move.l r3, r3
	add.q r4, r2, r3
	moveq r3, #0
	beq r2, r4, .L9
.L8:
	load.b r5, (r2)
	add.l r3, r5, r3
	store.l r3, (r1)
	add.q r2, r2, #1
	bne r2, r4, .L8
.L9:
	rts
.size byte,.-byte
.section .text,"ax"
.global call
.type call,@function
.align 16
call:
	sub.q r31, r31, #24
	store.q r18, 0(r31)
	store.q r19, 8(r31)
	store.q r20, 16(r31)
	move.l r19, r2
	moveq r2, #0
	store.l r2, (r1)
	moveq r18, #0
	bls r19, r18, .L14
.L13:
	move.q r20, r1
	move.l r1, r18
	jsr f
	move.l r2, r1
	move.q r1, r20
	load.l r3, (r1)
	add.l r2, r2, r3
	store.l r2, (r1)
	add.l r18, r18, #1
	bhi r19, r18, .L13
.L14:
	load.q r18, 0(r31)
	load.q r19, 8(r31)
	load.q r20, 16(r31)
	add.q r31, r31, #24
	rts
.size call,.-call
.section .text,"ax"
.global unproven
.type unproven,@function
.align 16
unproven:
	move.l r3, r3
	lsl.q r3, r3, #1
	add.q r3, r2, r3
	beq r2, r3, .L18
.L17:
	load.w r4, (r2)
	sext.w r4, r4
	load.l r5, (r1)
	add.l r4, r4, r5
	store.l r4, (r1)
	add.q r2, r2, #2
	bne r2, r3, .L17
.L18:
	rts
.size unproven,.-unproven
.section .text,"ax"
.global volatile
.type volatile,@function
.align 16
volatile:
	moveq r4, #0
	store.l r4, (r1)
	move.l r3, r3
	lsl.q r3, r3, #1
	add.q r3, r2, r3
	beq r2, r3, .L22
.L21:
	load.w r4, (r2)
	sext.w r4, r4
	load.l r5, (r1)
	add.l r4, r4, r5
	store.l r4, (r1)
	add.q r2, r2, #2
	bne r2, r3, .L21
.L22:
	rts
.size volatile,.-volatile
