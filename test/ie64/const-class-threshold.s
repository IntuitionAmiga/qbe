.section .text,"ax"
.global uconv
.type uconv,@function
.align 16
uconv:
	sub.q r31, r31, #8
	store.q r18, 0(r31)
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f4, (r30)
	dcmp r29, f0, f4
	fmovsr r30
	and.q r30, r30, #0x1000000
	moveq r1, #0
	bne r30, r0, .Lcmp0
	bge r29, r0, .Lcmptrue0
	bra .Lcmp0
.Lcmptrue0:
	moveq r1, #1
.Lcmp0:
	move.l r2, r1
	dcvtif f2, r2
	dmul f2, f2, f4
	dsub f0, f0, f2
	dcvtfi r1, f0
	lsl.q r2, r2, #63
	or.q r18, r1, r2
	move.l r1, #0x00000000
	movt r1, #0x43e00000
	jsr tagl
	move.q r1, r18
	load.q r18, 0(r31)
	add.q r31, r31, #8
	rts
.size uconv,.-uconv
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 8
.L__ie64_fconst_0:
	dc.q 0x43e0000000000000
