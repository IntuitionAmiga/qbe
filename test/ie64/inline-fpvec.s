.section .text,"ax"
.local dot4
.type dot4,@function
.align 16
dot4:
	fipr f0, (r1), (r2)
	rts
.size dot4,.-dot4
.section .text,"ax"
.local mulv
.type mulv,@function
.align 16
mulv:
	sub.q r31, r31, #24
	lea r4, 0(r31)
	move.q r30, r1
	move.q r1, r2
	move.q r2, r30
	load.q r5, (r3)
	add.q r6, r4, #0
	store.q r5, (r6)
	load.q r3, 8(r3)
	add.q r5, r4, #8
	store.q r3, (r5)
	ftrv (r1), (r4)
	load.q r1, (r4)
	store.q r1, (r2)
	load.q r1, 8(r4)
	store.q r1, 8(r2)
	add.q r31, r31, #24
	rts
.size mulv,.-mulv
.section .text,"ax"
.global local
.type local,@function
.align 16
local:
	sub.q r31, r31, #56
	store.q r18, 32(r31)
	store.q r19, 40(r31)
	lea r19, 16(r31)
	lea r18, 0(r31)
	move.q r1, r18
	jsr fill
	move.q r1, r19
	jsr fill
	fipr f0, (r18), (r19)
	load.q r18, 32(r31)
	load.q r19, 40(r31)
	add.q r31, r31, #56
	rts
.size local,.-local
.section .text,"ax"
.global apply
.type apply,@function
.align 16
apply:
	sub.q r31, r31, #56
	store.q r18, 32(r31)
	store.q r19, 40(r31)
	store.q r20, 48(r31)
	lea r18, 0(r31)
	lea r20, 16(r31)
	move.q r19, r1
	move.q r1, r20
	jsr fill
	load.q r1, (r20)
	store.q r1, (r18)
	load.q r1, 8(r20)
	add.q r2, r18, #8
	store.q r1, (r2)
	ftrv (r19), (r18)
	fload f0, (r18)
	load.q r18, 32(r31)
	load.q r19, 40(r31)
	load.q r20, 48(r31)
	add.q r31, r31, #56
	rts
.size apply,.-apply
