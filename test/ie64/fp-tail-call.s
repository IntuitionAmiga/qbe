.section .text,"ax"
.global tail_f32
.type tail_f32,@function
.align 16
tail_f32:
	bra callee_f32
.size tail_f32,.-tail_f32
.section .text,"ax"
.global tail_f64
.type tail_f64,@function
.align 16
tail_f64:
	bra callee_f64
.size tail_f64,.-tail_f64
.section .text,"ax"
.global tail_f32_second
.type tail_f32_second,@function
.align 16
tail_f32_second:
	fmov f0, f1
	bra callee_f32
.size tail_f32_second,.-tail_f32_second
.section .text,"ax"
.global convert_f64
.type convert_f64,@function
.align 16
convert_f64:
	sub.q r31, r31, #8
	jsr callee_f64
	fcvtds f0, f0
	add.q r31, r31, #8
	rts
.size convert_f64,.-convert_f64
.section .text,"ax"
.global change_result
.type change_result,@function
.align 16
change_result:
	sub.q r31, r31, #8
	jsr callee_f64
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f2, (r30)
	dadd f0, f0, f2
	add.q r31, r31, #8
	rts
.size change_result,.-change_result
.section .text,"ax"
.global stack_arg
.type stack_arg,@function
.align 16
stack_arg:
	sub.q r31, r31, #24
	store.q r25, 0(r31)
	fstore f8, 8(r31)
	fstore f9, 16(r31)
	move.q r25, r31
	dmov f8, f0
	sub.q r31, r31, #16
	move.q r1, r31
	dmov f0, f8
	dstore f8, (r1)
	jsr callee_stack
	add.q r31, r31, #16
	move.q r31, r25
	load.q r25, 0(r31)
	fload f8, 8(r31)
	fload f9, 16(r31)
	add.q r31, r31, #24
	rts
.size stack_arg,.-stack_arg
.section .text,"ax"
.global indirect_call
.type indirect_call,@function
.align 16
indirect_call:
	sub.q r31, r31, #8
	jsr (r1)
	add.q r31, r31, #8
	rts
.size indirect_call,.-indirect_call
.section .text,"ax"
.global sincos_struct
.type sincos_struct,@function
.align 16
sincos_struct:
	sub.q r31, r31, #24
	store.q r18, 8(r31)
	store.q r19, 16(r31)
	lea r19, 0(r31)
	move.q r18, r1
	move.q r1, r19
	jsr callee_sincos
	load.q r1, (r19)
	store.q r1, (r18)
	load.q r18, 8(r31)
	load.q r19, 16(r31)
	add.q r31, r31, #24
	rts
.size sincos_struct,.-sincos_struct
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 8
.L__ie64_fconst_0:
	dc.q 0x3ff0000000000000
