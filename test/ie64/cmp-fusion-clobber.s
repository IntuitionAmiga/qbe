.section .text,"ax"
.global aftercall
.type aftercall,@function
.align 16
aftercall:
	sub.q r31, r31, #8
	store.q r18, 0(r31)
	sext.l r1, r1
	sext.l r2, r2
	li r30, #0
	bge r1, r2, .Lcmp0
	li r30, #1
.Lcmp0:
	move.q r18, r30
	jsr sink
	bne r18, r0, .L2
.L1:
	li r1, #0
	sext.l r1, r1
	load.q r18, 0(r31)
	add.q r31, r31, #8
	rts
.L2:
	li r1, #1
	sext.l r1, r1
	load.q r18, 0(r31)
	add.q r31, r31, #8
	rts
.size aftercall,.-aftercall
.section .text,"ax"
.global afterdef
.type afterdef,@function
.align 16
afterdef:
	sext.l r1, r1
	sext.l r2, r2
	li r30, #0
	bge r1, r2, .Lcmp1
	li r30, #1
.Lcmp1:
	move.q r1, r30
	add.q r2, r3, #8
	move.l r30, #lo32(slot+0)
	movt r30, #hi32(slot+0)
	store.q r2, (r30)
	bne r1, r0, .L5
.L4:
	li r1, #0
	sext.l r1, r1
	rts
.L5:
	li r1, #1
	sext.l r1, r1
	rts
.size afterdef,.-afterdef
.section .text,"ax"
.global stillfused
.type stillfused,@function
.align 16
stillfused:
	sext.l r1, r1
	sext.l r2, r2
	move.l r30, #lo32(slot+0)
	movt r30, #hi32(slot+0)
	store.q r3, (r30)
	blt r1, r2, .L8
.L7:
	li r1, #0
	sext.l r1, r1
	rts
.L8:
	li r1, #1
	sext.l r1, r1
	rts
.size stillfused,.-stillfused
.section .bss,"aw"
.local slot
.type slot,@object
.align 8
slot:
	ds.b 8
.size slot,8
