.section .text,"ax"
.global f
.type f,@function
.align 16
f:
	bne r2, r0, .L2
.L1:
	add.q r2, r1, #16
	moveq r1, #7
	store.l r1, (r2)
	moveq r1, #7
	rts
.L2:
	load.l r1, 16(r1)
	sext.l r1, r1
	rts
.size f,.-f
.section .text,"ax"
.global loop
.type loop,@function
.align 16
loop:
	moveq r3, #0
	moveq r4, #0
	sext.l r5, r4
	sext.l r6, r2
	bge r5, r6, .L7
.L6:
	load.l r5, 24(r1)
	add.l r3, r3, r5
	store.l r3, 24(r1)
	add.l r4, r4, #1
	sext.l r5, r4
	sext.l r6, r2
	blt r5, r6, .L6
.L7:
	sext.l r1, r3
	rts
.size loop,.-loop
.section .text,"ax"
.global value
.type value,@function
.align 16
value:
	add.q r1, r1, #8
	bne r3, r0, .L10
.L9:
	load.l r1, (r1)
	sext.l r1, r1
	rts
.L10:
	store.q r1, (r2)
	moveq r1, #0
	rts
.size value,.-value
.section .text,"ax"
.global big
.type big,@function
.align 16
big:
	move.l r3, #0x00000000
	movt r3, #0x00000001
	add.q r1, r1, r3
	bne r2, r0, .L13
.L12:
	moveq r2, #7
	store.l r2, (r1)
	moveq r1, #7
	rts
.L13:
	load.l r1, (r1)
	sext.l r1, r1
	rts
.size big,.-big
