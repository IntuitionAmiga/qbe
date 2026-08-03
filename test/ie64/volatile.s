.section .text,"ax"
.global f
.type f,@function
.align 16
f:
	move.l r30, #lo32(x+0)
	movt r30, #hi32(x+0)
	load.q r2, (r30)
	move.l r30, #lo32(x+0)
	movt r30, #hi32(x+0)
	load.q r1, (r30)
	add.q r2, r2, r1
	move.l r30, #lo32(x+0)
	movt r30, #hi32(x+0)
	store.q r2, (r30)
	li r2, #3
	move.l r30, #lo32(x+0)
	movt r30, #hi32(x+0)
	store.q r2, (r30)
	move.q r1, r1
	rts
.size f,.-f
.section .text,"ax"
.global discard
.type discard,@function
.align 16
discard:
	move.l r30, #lo32(x+0)
	movt r30, #hi32(x+0)
	load.q r1, (r30)
	move.l r30, #lo32(x+0)
	movt r30, #hi32(x+0)
	load.q r1, (r30)
.L2:
	rts
.size discard,.-discard
.section .bss,"aw"
.global x
.type x,@object
.align 8
x:
	ds.b 8
.size x,8
