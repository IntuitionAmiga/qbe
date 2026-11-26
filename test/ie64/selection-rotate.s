.section .text,"ax"
.global left_word
.type left_word,@function
.align 16
left_word:
	rol.l r1, r1, r2
	rts
.size left_word,.-left_word
.section .text,"ax"
.global right_long
.type right_long,@function
.align 16
right_long:
	ror.q r1, r1, r2
	rts
.size right_long,.-right_long
.section .text,"ax"
.global constant_long
.type constant_long,@function
.align 16
constant_long:
	rol.q r1, r1, #2
	rts
.size constant_long,.-constant_long
.section .text,"ax"
.global wrong_mask
.type wrong_mask,@function
.align 16
wrong_mask:
	and.l r3, r2, #31
	neg.l r4, r2
	and.l r5, r4, #15
	lsl.l r3, r1, r3
	move.l r4, r1
	lsr.l r4, r4, r5
	or.l r1, r3, r4
	sext.l r1, r1
	rts
.size wrong_mask,.-wrong_mask
