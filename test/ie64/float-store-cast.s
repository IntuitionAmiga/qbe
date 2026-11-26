.section .text,"ax"
.global store_single_cast
.type store_single_cast,@function
.align 16
store_single_cast:
	fstore f0, (r1)
	fstore f1, 4(r1)
	rts
.size store_single_cast,.-store_single_cast
.section .text,"ax"
.global store_double_cast
.type store_double_cast,@function
.align 16
store_double_cast:
	dstore f0, (r1)
	dstore f2, -8(r1)
	rts
.size store_double_cast,.-store_double_cast
.section .text,"ax"
.global store_narrow_cast
.type store_narrow_cast,@function
.align 16
store_narrow_cast:
	move.q r2, r1
	fmovo r1, f0
	store.w r1, (r2)
	sext.l r1, r1
	rts
.size store_narrow_cast,.-store_narrow_cast
