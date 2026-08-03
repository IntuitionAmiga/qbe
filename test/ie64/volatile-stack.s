.section .text,"ax"
.global volatile_stack_float
.type volatile_stack_float,@function
.align 16
volatile_stack_float:
	sub.q r31, r31, #8
	lea r30, 0(r31)
	store.q r1, (r30)
	lea r1, 0(r31)
	fstore f0, (r1)
	lea r1, 0(r31)
	fload f0, (r1)
.L1:
	add.q r31, r31, #8
	rts
.size volatile_stack_float,.-volatile_stack_float
.section .text,"ax"
.global volatile_stack_double
.type volatile_stack_double,@function
.align 16
volatile_stack_double:
	sub.q r31, r31, #8
	lea r30, 0(r31)
	store.q r1, (r30)
	lea r1, 0(r31)
	dstore f0, (r1)
	lea r1, 0(r31)
	dload f0, (r1)
.L3:
	add.q r31, r31, #8
	rts
.size volatile_stack_double,.-volatile_stack_double
