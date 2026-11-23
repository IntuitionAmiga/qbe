.section .text,"ax"
.global xchg_reload
.type xchg_reload,@function
.align 16
xchg_reload:
	move.q r3, r1
	move.q r1, r2
	load.q r2, (r3)
	xchg r1, (r3), r1
	load.q r3, (r3)
	add.q r2, r2, r3
	add.q r1, r1, r2
	rts
.size xchg_reload,.-xchg_reload
.section .text,"ax"
.global xchg_store
.type xchg_store,@function
.align 16
xchg_store:
	move.q r30, r1
	move.q r1, r2
	move.q r2, r30
	moveq r3, #3
	store.q r3, (r2)
	xchg r1, (r2), r1
	load.q r2, (r2)
	add.q r1, r1, r2
	rts
.size xchg_store,.-xchg_store
.section .text,"ax"
.global xchg_other
.type xchg_other,@function
.align 16
xchg_other:
	move.q r4, r1
	move.q r1, r3
	move.q r3, r2
	load.q r2, (r3)
	xchg r1, (r4), r1
	load.q r3, (r3)
	add.q r2, r2, r3
	add.q r1, r1, r2
	rts
.size xchg_other,.-xchg_other
.section .text,"ax"
.global acquire
.type acquire,@function
.align 16
acquire:
	move.q r3, r2
	load.q r2, (r3)
	moveq r4, #1
	xchg r1, (r1), r4
	load.q r3, (r3)
	add.q r2, r2, r3
	add.q r1, r1, r2
	rts
.size acquire,.-acquire
.section .text,"ax"
.global join
.type join,@function
.align 16
join:
	move.q r4, r2
	move.q r30, r1
	move.q r1, r3
	move.q r3, r30
	load.q r2, (r3)
	move.l r4, r4
	bne r4, r0, .L6
.L5:
	fxor r1, (r3), r1
	bra .L7
.L6:
	faa r1, (r3), r1
.L7:
	load.q r3, (r3)
	add.q r2, r2, r3
	add.q r1, r1, r2
	rts
.size join,.-join
.section .text,"ax"
.global loop
.type loop,@function
.align 16
loop:
	moveq r4, #0
	moveq r3, #0
	bge r3, r2, .L12
.L11:
	load.q r6, (r1)
	move.q r5, r4
	moveq r4, #1
	faa r4, (r1), r4
	add.q r5, r5, r6
	add.q r4, r4, r5
	add.q r3, r3, #1
	blt r3, r2, .L11
.L12:
	move.q r1, r4
	rts
.size loop,.-loop
.section .text,"ax"
.global unused
.type unused,@function
.align 16
unused:
	move.q r3, r2
	move.q r2, r1
	load.q r1, (r2)
	fand r3, (r2), r3
	load.q r2, (r2)
	add.q r1, r1, r2
	rts
.size unused,.-unused
.section .text,"ax"
.global or_reload
.type or_reload,@function
.align 16
or_reload:
	move.q r3, r1
	move.q r1, r2
	load.q r2, (r3)
	for r1, (r3), r1
	load.q r3, (r3)
	add.q r2, r2, r3
	add.q r1, r1, r2
	rts
.size or_reload,.-or_reload
.section .text,"ax"
.global cas_reload
.type cas_reload,@function
.align 16
cas_reload:
	move.q r4, r2
	move.q r30, r1
	move.q r1, r3
	move.q r3, r30
	load.q r2, (r3)
	move.q r29, r4
	move.q r30, r3
	move.q r1, r1
	cas r1, (r30), r29
	load.q r3, (r3)
	add.q r2, r2, r3
	add.q r1, r1, r2
	rts
.size cas_reload,.-cas_reload
.section .text,"ax"
.global private
.type private,@function
.align 16
private:
	moveq r3, #1
	xchg r1, (r1), r3
	move.l r2, r2
	add.q r1, r1, r2
	rts
.size private,.-private
.section .text,"ax"
.global barrier
.type barrier,@function
.align 16
barrier:
	sub.q r31, r31, #24
	store.q r18, 8(r31)
	store.q r19, 16(r31)
	lea r18, 0(r31)
	move.q r19, r1
	moveq r1, #0
	store.q r1, (r18)
	move.q r1, r18
	jsr publish
	move.l r30, #lo32(word+0)
	movt r30, #hi32(word+0)
	load.q r2, (r30)
	xchg r1, (r18), r19
	move.l r30, #lo32(word+0)
	movt r30, #hi32(word+0)
	load.q r3, (r30)
	add.q r2, r2, r3
	add.q r1, r1, r2
	load.q r18, 8(r31)
	load.q r19, 16(r31)
	add.q r31, r31, #24
	rts
.size barrier,.-barrier
.section .data,"aw"
.local word
.type word,@object
.align 8
word:
	dc.q 7
.size word,8
