.section .text,"ax"
.local f
.type f,@function
.align 16
f:
	sub.q r31, r31, #8
	lea r3, 0(r31)
	sub.q r3, r1, r3
	add.q r3, r2, r3
	moveq r2, #7
	store.l r2, (r3)
	load.l r1, (r1)
	sext.l r1, r1
	add.q r31, r31, #8
	rts
.size f,.-f
.section .text,"ax"
.local g
.type g,@function
.align 16
g:
	sub.q r31, r31, #8
	lea r3, 0(r31)
	move.q r30, r1
	move.q r1, r2
	move.q r2, r30
	moveq r4, #1
	store.l r4, (r3)
	sub.q r1, r1, r3
	add.q r1, r3, r1
	load.l r1, (r1)
	store.l r1, (r2)
	add.q r31, r31, #8
	rts
.size g,.-g
.section .text,"ax"
.global h
.type h,@function
.align 16
h:
	sub.q r31, r31, #24
	lea r5, 12(r31)
	lea r6, 8(r31)
	lea r7, 4(r31)
	lea r2, 0(r31)
	lea r4, 20(r31)
	moveq r1, #5
	store.l r1, (r4)
	load.l r1, (r4)
	store.l r1, (r2)
	sub.q r1, r2, r7
	add.q r7, r4, r1
	moveq r1, #7
	store.l r1, (r7)
	load.l r2, (r2)
	load.l r1, (r4)
	store.l r1, (r6)
	moveq r1, #1
	store.l r1, (r5)
	sub.q r1, r6, r5
	add.q r1, r5, r1
	load.l r1, (r1)
	store.l r1, 16(r31)
	moveq r1, #9
	store.l r1, (r4)
	load.l r1, 16(r31)
	add.l r1, r1, r2
	sext.l r1, r1
	add.q r31, r31, #24
	rts
.size h,.-h
.section .text,"ax"
.local read
.type read,@function
.align 16
read:
	sub.q r31, r31, #8
	lea r3, 0(r31)
	sub.q r2, r2, r3
	add.q r3, r3, r2
	moveq r2, #9
	store.q r2, (r3)
	load.q r1, (r1)
	add.q r31, r31, #8
	rts
.size read,.-read
.section .text,"ax"
.global reload
.type reload,@function
.align 16
reload:
	sub.q r31, r31, #24
	lea r1, 0(r31)
	lea r2, 8(r31)
	moveq r3, #5
	store.q r3, (r2)
	sub.q r2, r2, r1
	add.q r2, r1, r2
	moveq r1, #9
	store.q r1, (r2)
	moveq r1, #5
	add.q r31, r31, #24
	rts
.size reload,.-reload
.section .text,"ax"
.local readd
.type readd,@function
.align 16
readd:
	sub.q r31, r31, #8
	lea r3, 0(r31)
	sub.q r2, r2, r3
	store.q r2, (r3)
	sub.q r31, r31, #16
	store.q r2, (r31)
	dload f0, (r31)
	add.q r31, r31, #16
	sub.q r31, r31, #16
	dstore f0, (r31)
	load.q r2, (r31)
	add.q r31, r31, #16
	add.q r3, r3, r2
	moveq r2, #9
	store.l r2, (r3)
	load.l r1, (r1)
	sext.l r1, r1
	add.q r31, r31, #8
	rts
.size readd,.-readd
.section .text,"ax"
.global reloadd
.type reloadd,@function
.align 16
reloadd:
	sub.q r31, r31, #24
	lea r1, 0(r31)
	lea r2, 8(r31)
	moveq r3, #5
	store.l r3, (r2)
	sub.q r2, r2, r1
	store.q r2, (r1)
	sub.q r31, r31, #16
	store.q r2, (r31)
	dload f0, (r31)
	add.q r31, r31, #16
	sub.q r31, r31, #16
	dstore f0, (r31)
	load.q r2, (r31)
	add.q r31, r31, #16
	add.q r2, r1, r2
	moveq r1, #9
	store.l r1, (r2)
	moveq r1, #5
	add.q r31, r31, #24
	rts
.size reloadd,.-reloadd
