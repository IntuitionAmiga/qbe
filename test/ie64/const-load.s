.section .text,"ax"
.global w_small
.type w_small,@function
.align 16
w_small:
	moveq r1, #7
	rts
.size w_small,.-w_small
.section .text,"ax"
.global w_negative
.type w_negative,@function
.align 16
w_negative:
	moveq r1, #-7
	rts
.size w_negative,.-w_negative
.section .text,"ax"
.global w_int_min
.type w_int_min,@function
.align 16
w_int_min:
	moveq r1, #-2147483648
	rts
.size w_int_min,.-w_int_min
.section .text,"ax"
.global l_small
.type l_small,@function
.align 16
l_small:
	moveq r1, #7
	rts
.size l_small,.-l_small
.section .text,"ax"
.global l_negative
.type l_negative,@function
.align 16
l_negative:
	moveq r1, #-2
	rts
.size l_negative,.-l_negative
.section .text,"ax"
.global l_int_min
.type l_int_min,@function
.align 16
l_int_min:
	moveq r1, #-2147483648
	rts
.size l_int_min,.-l_int_min
.section .text,"ax"
.global l_int_max
.type l_int_max,@function
.align 16
l_int_max:
	moveq r1, #2147483647
	rts
.size l_int_max,.-l_int_max
.section .text,"ax"
.global l_uint_low
.type l_uint_low,@function
.align 16
l_uint_low:
	move.l r1, #0x80000000
	rts
.size l_uint_low,.-l_uint_low
.section .text,"ax"
.global l_uint_max
.type l_uint_max,@function
.align 16
l_uint_max:
	move.l r1, #0xffffffff
	rts
.size l_uint_max,.-l_uint_max
.section .text,"ax"
.global l_wide
.type l_wide,@function
.align 16
l_wide:
	move.l r1, #0x00000000
	movt r1, #0x00000001
	rts
.size l_wide,.-l_wide
.section .text,"ax"
.global l_below_int_min
.type l_below_int_min,@function
.align 16
l_below_int_min:
	move.l r1, #0x7fffffff
	movt r1, #0xffffffff
	rts
.size l_below_int_min,.-l_below_int_min
.section .text,"ax"
.global l_min
.type l_min,@function
.align 16
l_min:
	move.l r1, #0x00000000
	movt r1, #0x80000000
	rts
.size l_min,.-l_min
.section .text,"ax"
.global l_use
.type l_use,@function
.align 16
l_use:
	add.q r1, r1, #4294967295
	rts
.size l_use,.-l_use
