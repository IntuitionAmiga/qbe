.section .text,"ax"
.local swap
.type swap,@function
.align 16
swap:
	xchg r1, (r1), r2
	rts
.size swap,.-swap
.section .text,"ax"
.local control
.type control,@function
.align 16
control:
	mfcr r1, cr6
	rts
.size control,.-control
.section .text,"ax"
.local round
.type round,@function
.align 16
round:
	moveq r1, #2
	fmovcc r1
	rts
.size round,.-round
.section .text,"ax"
.local mask
.type mask,@function
.align 16
mask:
	sei
	rts
.size mask,.-mask
.section .text,"ax"
.global user
.type user,@function
.align 16
user:
	sub.q r31, r31, #24
	store.q r18, 0(r31)
	store.q r19, 8(r31)
	moveq r2, #1
	jsr swap
	move.q r18, r1
	jsr control
	move.q r19, r1
	jsr round
	jsr mask
	add.q r1, r18, r19
	load.q r18, 0(r31)
	load.q r19, 8(r31)
	add.q r31, r31, #24
	rts
.size user,.-user
