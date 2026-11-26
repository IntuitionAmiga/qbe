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
	move.l r3, r2
	move.l r2, r1
	and.l r1, r3, #31
	neg.l r3, r3
	and.l r3, r3, #15
	lsl.l r1, r2, r1
	move.l r2, r2
	lsr.l r2, r2, r3
	or.l r1, r1, r2
	sext.l r1, r1
	rts
.size wrong_mask,.-wrong_mask
