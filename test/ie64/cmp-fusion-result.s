.section .text,"ax"
.global storeresult
.type storeresult,@function
.align 16
storeresult:
	sext.l r1, r1
	sext.l r2, r2
	li r30, #0
	bge r1, r2, .Lcmp0
	li r30, #1
.Lcmp0:
	move.q r1, r30
	store.b r1, (r3)
	bne r1, r0, .L2
.L1:
	li r1, #0
	sext.l r1, r1
	rts
.L2:
	li r1, #1
	sext.l r1, r1
	rts
.size storeresult,.-storeresult
.section .text,"ax"
.global liveout
.type liveout,@function
.align 16
liveout:
	sext.l r1, r1
	sext.l r2, r2
	li r30, #0
	bge r1, r2, .Lcmp1
	li r30, #1
.Lcmp1:
	move.q r1, r30
	bne r1, r0, .L5
.L4:
	li r1, #7
	sext.l r1, r1
	rts
.L5:
	load.l r2, (r3)
	sext.l r2, r2
	add.l r1, r1, r2
	move.l r1, r1
	sext.l r1, r1
	rts
.size liveout,.-liveout
.section .text,"ax"
.global phiout
.type phiout,@function
.align 16
phiout:
	sext.l r1, r1
	sext.l r2, r2
	li r30, #0
	bge r1, r2, .Lcmp2
	li r30, #1
.Lcmp2:
	move.q r1, r30
	bne r1, r0, .L8
.L7:
	li r1, #9
	bra .L9
.L8:
	move.l r30, #lo32(slot+0)
	movt r30, #hi32(slot+0)
	store.q r3, (r30)
.L9:
	move.l r1, r1
	sext.l r1, r1
	rts
.size phiout,.-phiout
.section .text,"ax"
.global phifalse
.type phifalse,@function
.align 16
phifalse:
	sext.l r1, r1
	sext.l r2, r2
	li r30, #0
	bge r1, r2, .Lcmp3
	li r30, #1
.Lcmp3:
	move.q r1, r30
	beq r1, r0, .L13
.L11:
	move.l r30, #lo32(slot+0)
	movt r30, #hi32(slot+0)
	store.q r3, (r30)
.L12:
	li r1, #9
.L13:
	move.l r1, r1
	sext.l r1, r1
	rts
.size phifalse,.-phifalse
.section .text,"ax"
.global loopfalse
.type loopfalse,@function
.align 16
loopfalse:
	sext.l r1, r1
	sext.l r4, r2
	li r30, #0
	bge r1, r4, .Lcmp4
	li r30, #1
.Lcmp4:
	move.q r5, r30
	bne r5, r0, .L18
.L15:
	li r1, #0
.L16:
	load.l r4, (r3)
	sext.l r4, r4
	add.l r1, r1, r4
	sext.l r4, r1
	sext.l r6, r2
	blt r4, r6, .L16
.L17:
	add.l r1, r5, r1
	move.l r1, r1
	sext.l r1, r1
	rts
.L18:
	li r1, #7
	sext.l r1, r1
	rts
.size loopfalse,.-loopfalse
.section .bss,"aw"
.local slot
.type slot,@object
.align 8
slot:
	ds.b 8
.size slot,8
