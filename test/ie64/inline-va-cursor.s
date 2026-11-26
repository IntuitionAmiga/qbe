.section .text,"ax"
.global sum_variadic
.type sum_variadic,@function
.align 16
sum_variadic:
	lea r3, 8(r31)
	beq r1, r0, .L4
.L1:
	moveq r2, #0
.L2:
	load.q r4, (r3)
	add.q r3, r3, #8
	add.q r2, r2, r4
	sub.l r1, r1, #1
	bne r1, r0, .L2
.L3:
	move.q r1, r2
	rts
.L4:
	moveq r1, #0
	rts
.size sum_variadic,.-sum_variadic
.section .text,"ax"
.global escaped_cursor
.type escaped_cursor,@function
.align 16
escaped_cursor:
	sub.q r31, r31, #24
	store.q r18, 8(r31)
	lea r18, 0(r31)
	move.q r2, r1
	lea r1, 32(r31)
	store.q r1, (r18)
	move.q r1, r18
	jsr advance
	load.q r2, (r18)
	load.q r1, (r2)
	add.q r2, r2, #8
	store.q r2, (r18)
	load.q r18, 8(r31)
	add.q r31, r31, #24
	rts
.size escaped_cursor,.-escaped_cursor
.section .text,"ax"
.global stack_cursor
.type stack_cursor,@function
.align 16
stack_cursor:
	load.q r1, 8(r31)
	load.q r2, 16(r31)
	add.q r1, r1, r2
	rts
.size stack_cursor,.-stack_cursor
