.section .text,"ax"
.local envonly
.type envonly,@function
.align 16
envonly:
	move.q r1, r28
	rts
.size envonly,.-envonly
.section .text,"ax"
.local envfull
.type envfull,@function
.align 16
envfull:
	move.q r2, r1
	load.q r1, 8(r31)
	add.q r2, r28, r2
	add.q r2, r6, r2
	add.q r1, r1, r2
	rts
.size envfull,.-envfull
.section .text,"ax"
.global callenv
.type callenv,@function
.align 16
callenv:
	sub.q r31, r31, #8
	store.q r25, 0(r31)
	move.q r25, r31
	move.q r28, r1
	jsr envonly
	move.q r28, r1
	sub.q r31, r31, #16
	move.q r8, r31
	moveq r1, #1
	moveq r2, #2
	moveq r3, #3
	moveq r4, #4
	moveq r5, #5
	moveq r6, #6
	moveq r7, #7
	store.q r7, (r8)
	jsr envfull
	add.q r31, r31, #16
	move.q r31, r25
	load.q r25, 0(r31)
	add.q r31, r31, #8
	rts
.size callenv,.-callenv
