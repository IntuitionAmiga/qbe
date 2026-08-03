.section .text,"ax"
.global roundtrip
.type roundtrip,@function
.align 16
roundtrip:
	move.l r30, #lo32(g+0)
	movt r30, #hi32(g+0)
	fstore f0, (r30)
.L1:
	rts
.size roundtrip,.-roundtrip
.section .data,"aw"
.global g
.type g,@object
.align 4
g:
	dc.l 0
.size g,4
