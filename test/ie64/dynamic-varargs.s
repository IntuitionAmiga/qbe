.section .text,"ax"
.global dynamic_varargs
.type dynamic_varargs,@function
.align 16
dynamic_varargs:
	sub.q r31, r31, #24
	store.q r18, 0(r31)
	store.q r25, 8(r31)
	move.q r25, r31
	add.q r1, r1, #15
	and.q r1, r1, #-16
	sub.q r31, r31, r1
	move.q r1, r31
	load.q r18, 40(r25)
	store.q r18, (r1)
	jsr use
	move.q r1, r18
	move.q r31, r25
	load.q r18, 0(r31)
	load.q r25, 8(r31)
	add.q r31, r31, #24
	rts
.size dynamic_varargs,.-dynamic_varargs
.section .text,"ax"
.local use
.type use,@function
.align 16
use:
	rts
.size use,.-use
