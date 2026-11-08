.section .text,"ax"
.global wide_return
.type wide_return,@function
.align 16
wide_return:
	move.l r1, #0x00000000
	movt r1, #0x12345678
	rts
.size wide_return,.-wide_return
.section .text,"ax"
.global wide_add
.type wide_add,@function
.align 16
wide_add:
	move.l r2, #0x00000000
	movt r2, #0x12345678
	add.q r1, r1, r2
	rts
.size wide_add,.-wide_add
.section .text,"ax"
.global negative_add
.type negative_add,@function
.align 16
negative_add:
	sub.q r1, r1, #1
	rts
.size negative_add,.-negative_add
