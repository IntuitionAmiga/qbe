.section .text,"ax"
.global main
.type main,@function
.align 16
main:
	move.l r30, #lo32(g+0)
	movt r30, #hi32(g+0)
	load.l r1, (r30)
	sext.l r1, r1
	add.l r1, r1, #4
	move.l r30, #lo32(g+0)
	movt r30, #hi32(g+0)
	store.l r1, (r30)
	move.l r1, r1
	sext.l r1, r1
	rts
.size main,.-main
.section .data,"aw"
.global g
.type g,@object
.align 4
g:
	dc.l 3
.size g,4
