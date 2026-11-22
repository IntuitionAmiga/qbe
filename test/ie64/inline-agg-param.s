.section .text,"ax"
.local sum
.type sum,@function
.align 16
sum:
	move.q r2, r1
	fload f0, (r2)
	fload f1, 4(r2)
	fadd f0, f0, f1
	rts
.size sum,.-sum
.section .text,"ax"
.local mix
.type mix,@function
.align 16
mix:
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	fload f0, (r30)
	fstore f0, (r1)
	fload f0, (r2)
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	fload f1, (r30)
	fadd f0, f0, f1
	rts
.size mix,.-mix
.section .text,"ax"
.local thru
.type thru,@function
.align 16
thru:
	move.l r30, #lo32(.L__ie64_fconst_1+0)
	movt r30, #hi32(.L__ie64_fconst_1+0)
	fload f0, (r30)
	fstore f0, (r2)
	fload f0, (r1)
	rts
.size thru,.-thru
.section .text,"ax"
.local addr
.type addr,@function
.align 16
addr:
	bne r1, r2, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	rts
.size addr,.-addr
.section .text,"ax"
.global local
.type local,@function
.align 16
local:
	sub.q r31, r31, #24
	store.q r18, 16(r31)
	lea r18, 0(r31)
	move.q r1, r18
	jsr fill
	fload f0, (r18)
	fload f1, 4(r18)
	fadd f0, f0, f1
	load.q r18, 16(r31)
	add.q r31, r31, #24
	rts
.size local,.-local
.section .text,"ax"
.global viaptr
.type viaptr,@function
.align 16
viaptr:
	sub.q r31, r31, #24
	load.q r3, (r1)
	store.q r3, (r31)
	load.q r1, 8(r1)
	store.q r1, 8(r31)
	fload f0, (r31)
	fload f1, 4(r31)
	fadd f0, f0, f1
	add.q r31, r31, #24
	rts
.size viaptr,.-viaptr
.section .text,"ax"
.global mixed
.type mixed,@function
.align 16
mixed:
	sub.q r31, r31, #40
	store.q r18, 32(r31)
	lea r18, 16(r31)
	lea r1, 0(r31)
	jsr fill
	move.q r1, r18
	jsr fill
	fload f0, (r18)
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	fload f1, (r30)
	fadd f0, f0, f1
	load.q r18, 32(r31)
	add.q r31, r31, #40
	rts
.size mixed,.-mixed
.section .text,"ax"
.global through
.type through,@function
.align 16
through:
	sub.q r31, r31, #56
	store.q r18, 32(r31)
	store.q r19, 40(r31)
	lea r19, 16(r31)
	move.q r1, r19
	jsr fill
	load.q r2, (r19)
	store.q r2, (r31)
	load.q r2, 8(r19)
	store.q r2, 8(r31)
	move.l r30, #lo32(.L__ie64_fconst_1+0)
	movt r30, #hi32(.L__ie64_fconst_1+0)
	fload f0, (r30)
	fstore f0, (r1)
	fload f0, (r31)
	load.q r18, 32(r31)
	load.q r19, 40(r31)
	add.q r31, r31, #56
	rts
.size through,.-through
.section .text,"ax"
.global compare
.type compare,@function
.align 16
compare:
	sub.q r31, r31, #56
	store.q r18, 32(r31)
	store.q r19, 40(r31)
	lea r19, 0(r31)
	lea r18, 16(r31)
	move.q r1, r18
	jsr fill
	load.q r1, (r18)
	store.q r1, (r19)
	load.q r1, 8(r18)
	store.q r1, 8(r19)
	moveq r1, #0
	bne r18, r19, .Lcmp1
	moveq r1, #1
.Lcmp1:
	load.q r18, 32(r31)
	load.q r19, 40(r31)
	add.q r31, r31, #56
	rts
.size compare,.-compare
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 4
.L__ie64_fconst_0:
	dc.l 0x3f800000
.section .rodata,"a"
.local .L__ie64_fconst_1
.align 4
.L__ie64_fconst_1:
	dc.l 0x40a00000
