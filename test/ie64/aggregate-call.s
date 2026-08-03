.section .text,"ax"
.local f
.type f,@function
.align 16
f:
.L1:
	rts
.size f,.-f
.section .text,"ax"
.global g
.type g,@function
.align 16
g:
	sub.q r31, r31, #8
	lea r30, 0(r31)
	store.q r1, (r30)
	move.l r30, #lo32(s+0)
	movt r30, #hi32(s+0)
	load.l r1, (r30)
	lea r2, 0(r31)
	store.l r1, (r2)
	lea r1, 0(r31)
	jsr f
.L3:
	add.q r31, r31, #8
	rts
.size g,.-g
.section .bss,"aw"
.local s
.type s,@object
.align 4
s:
	ds.b 4
.size s,4
