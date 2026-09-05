.section .text,"ax"
.local f
.type f,@function
.align 16
f:
	sub.q r31, r31, #8
	lea r3, 0(r31)
	move.q r30, r1
	move.q r1, r2
	move.q r2, r30
	store.l r1, (r3)
	load.l r1, (r3)
	store.l r1, (r2)
.L1:
	add.q r31, r31, #8
	rts
.size f,.-f
.section .text,"ax"
.global g
.type g,@function
.align 16
g:
	sub.q r31, r31, #24
	store.q r18, 8(r31)
	lea r18, 0(r31)
	move.q r1, r18
	li r2, #2
	jsr f
	load.l r1, (r18)
	sext.l r1, r1
	move.l r1, r1
	sext.l r1, r1
	load.q r18, 8(r31)
	add.q r31, r31, #24
	rts
.size g,.-g
