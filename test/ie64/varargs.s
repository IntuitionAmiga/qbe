.section .text,"ax"
.local f
.type f,@function
.align 16
f:
	sub.q r31, r31, #8
	lea r3, 0(r31)
	lea r1, 16(r31)
	store.q r1, (r3)
	load.q r2, (r3)
	load.l r1, (r2)
	add.q r2, r2, #8
	store.q r2, (r3)
	load.q r2, (r3)
	add.q r2, r2, #8
	store.q r2, (r3)
	move.l r1, r1
	sext.l r1, r1
	add.q r31, r31, #8
	rts
.size f,.-f
.section .text,"ax"
.global g
.type g,@function
.align 16
g:
	sub.q r31, r31, #8
	store.q r25, 0(r31)
	move.q r25, r31
	sub.q r31, r31, #16
	move.q r2, r31
	li r1, #1
	li r3, #2
	store.q r3, (r2)
	add.q r2, r2, #8
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f0, (r30)
	dstore f0, (r2)
	jsr f
	add.q r31, r31, #16
	move.l r1, r1
	sext.l r1, r1
	move.q r31, r25
	load.q r25, 0(r31)
	add.q r31, r31, #8
	rts
.size g,.-g
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 8
.L__ie64_fconst_0:
	dc.q 0x4008000000000000
