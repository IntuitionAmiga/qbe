.section .text,"ax"
.local f
.type f,@function
.align 16
f:
	load.q r1, 8(r31)
	load.l r1, (r1)
	sext.l r1, r1
	rts
.size f,.-f
.section .text,"ax"
.global g
.type g,@function
.align 16
g:
	sub.q r31, r31, #24
	store.q r25, 8(r31)
	move.q r25, r31
	lea r7, 0(r25)
	sub.q r31, r31, #16
	move.q r8, r31
	moveq r1, #1
	moveq r2, #2
	moveq r3, #3
	moveq r4, #4
	moveq r5, #5
	moveq r6, #6
	move.l r30, #lo32(s+0)
	movt r30, #hi32(s+0)
	load.l r9, (r30)
	store.l r9, (r7)
	store.q r7, (r8)
	jsr f
	add.q r31, r31, #16
	sext.l r1, r1
	move.q r31, r25
	load.q r25, 8(r31)
	add.q r31, r31, #24
	rts
.size g,.-g
.section .data,"aw"
.local s
.type s,@object
.align 4
s:
	dc.l 7
.size s,4
