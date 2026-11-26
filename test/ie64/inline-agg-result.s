.section .text,"ax"
.local mk
.type mk,@function
.align 16
mk:
	sub.q r31, r31, #24
	move.q r2, r1
	fstore f0, (r31)
	fstore f0, 4(r31)
	fstore f0, 8(r31)
	fstore f0, 12(r31)
	load.q r1, (r31)
	store.q r1, (r2)
	load.q r1, 8(r31)
	store.q r1, 8(r2)
	add.q r31, r31, #24
	rts
.size mk,.-mk
.section .text,"ax"
.local glob
.type glob,@function
.align 16
glob:
	move.q r2, r1
	move.l r30, #lo32(gv+0)
	movt r30, #hi32(gv+0)
	load.q r1, (r30)
	store.q r1, (r2)
	move.l r1, #lo32(gv+0)
	movt r1, #hi32(gv+0)
	load.q r1, 8(r1)
	store.q r1, 8(r2)
	rts
.size glob,.-glob
.section .text,"ax"
.local id
.type id,@function
.align 16
id:
	load.q r3, (r2)
	store.q r3, (r1)
	load.q r3, 8(r2)
	store.q r3, 8(r1)
	rts
.size id,.-id
.section .text,"ax"
.local rev
.type rev,@function
.align 16
rev:
	sub.q r31, r31, #24
	move.q r6, r2
	move.q r2, r1
	fload f0, (r6)
	fload f1, 4(r6)
	fload f2, 8(r6)
	fload f3, 12(r6)
	fstore f3, (r31)
	fstore f2, 4(r31)
	fstore f1, 8(r31)
	fstore f0, 12(r31)
	load.q r1, (r31)
	store.q r1, (r2)
	load.q r1, 8(r31)
	store.q r1, 8(r2)
	add.q r31, r31, #24
	rts
.size rev,.-rev
.section .text,"ax"
.global own
.type own,@function
.align 16
own:
	rts
.size own,.-own
.section .text,"ax"
.global global
.type global,@function
.align 16
global:
	sub.q r31, r31, #24
	move.l r2, #lo32(gv+0)
	movt r2, #hi32(gv+0)
	load.q r2, (r2)
	store.q r2, (r31)
	move.l r2, #lo32(gv+0)
	movt r2, #hi32(gv+0)
	load.q r2, 8(r2)
	store.q r2, 8(r31)
	fload f0, 8(r31)
	add.q r31, r31, #24
	rts
.size global,.-global
.section .text,"ax"
.global ident
.type ident,@function
.align 16
ident:
	sub.q r31, r31, #24
	load.q r3, (r1)
	store.q r3, (r31)
	load.q r1, 8(r1)
	store.q r1, 8(r31)
	fload f0, 8(r31)
	add.q r31, r31, #24
	rts
.size ident,.-ident
.section .text,"ax"
.global inplace
.type inplace,@function
.align 16
inplace:
	fload f0, 12(r1)
	fstore f0, (r1)
	rts
.size inplace,.-inplace
.section .data,"aw"
.local gv
.type gv,@object
.align 8
gv:
	dc.l 1065353216
	dc.l 1073741824
	dc.l 1077936128
	dc.l 1082130432
.size gv,16
