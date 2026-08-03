.section .text,"ax"
.global double_to_ulong
.type double_to_ulong,@function
.align 16
double_to_ulong:
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f4, (r30)
	dcmp r29, f0, f4
	fmovsr r30
	and.q r30, r30, #0x1000000
	li r1, #0
	bne r30, r0, .Lcmp0
	bge r29, r0, .Lcmptrue0
	bra .Lcmp0
.Lcmptrue0:
	li r1, #1
.Lcmp0:
	move.l r2, r1
	dcvtif f2, r2
	dmul f2, f2, f4
	dsub f0, f0, f2
	dcvtfi r1, f0
	lsl.q r2, r2, #63
	or.q r1, r1, r2
	move.q r1, r1
	rts
.size double_to_ulong,.-double_to_ulong
.section .text,"ax"
.global single_to_ulong
.type single_to_ulong,@function
.align 16
single_to_ulong:
	fcvtsd f0, f0
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f4, (r30)
	dcmp r29, f0, f4
	fmovsr r30
	and.q r30, r30, #0x1000000
	li r1, #0
	bne r30, r0, .Lcmp1
	bge r29, r0, .Lcmptrue1
	bra .Lcmp1
.Lcmptrue1:
	li r1, #1
.Lcmp1:
	move.l r2, r1
	dcvtif f2, r2
	dmul f2, f2, f4
	dsub f0, f0, f2
	dcvtfi r1, f0
	lsl.q r2, r2, #63
	or.q r1, r1, r2
	move.q r1, r1
	rts
.size single_to_ulong,.-single_to_ulong
.section .text,"ax"
.global ulong_to_double
.type ulong_to_double,@function
.align 16
ulong_to_double:
	and.q r2, r1, #1
	lsr.q r1, r1, #1
	or.q r1, r1, r2
	dcvtif f0, r1
	dadd f0, f0, f0
.L3:
	rts
.size ulong_to_double,.-ulong_to_double
.section .text,"ax"
.global ulong_to_single
.type ulong_to_single,@function
.align 16
ulong_to_single:
	and.q r2, r1, #1
	lsr.q r1, r1, #1
	or.q r1, r1, r2
	dcvtif f0, r1
	dadd f0, f0, f0
	fcvtds f0, f0
.L5:
	rts
.size ulong_to_single,.-ulong_to_single
.section .text,"ax"
.global uint_to_double
.type uint_to_double,@function
.align 16
uint_to_double:
	move.l r1, r1
	dcvtif f0, r1
.L7:
	rts
.size uint_to_double,.-uint_to_double
.section .text,"ax"
.global uint_to_single
.type uint_to_single,@function
.align 16
uint_to_single:
	move.l r1, r1
	dcvtif f0, r1
	fcvtds f0, f0
.L9:
	rts
.size uint_to_single,.-uint_to_single
.section .rodata,"a"
.local .L__ie64_fconst_0
align 8
.L__ie64_fconst_0:
	dc.q 0x43e0000000000000
