.section .text,"ax"
.global half_span
.type half_span,@function
.align 16
half_span:
	lsr.q r3, r2, #1
	lsl.q r2, r3, #3
	add.q r2, r1, r2
	moveq r1, #0
	bls r3, r1, .L4
.L3:
	load.l r4, (r2)
	store.l r4, 4(r2)
	add.q r1, r1, #1
	add.q r2, r2, #8
	bhi r3, r1, .L3
.L4:
	rts
.size half_span,.-half_span
.section .text,"ax"
.global word_half
.type word_half,@function
.align 16
word_half:
	move.l r1, r1
	lsr.l r1, r1, #31
	rts
.size word_half,.-word_half
.section .text,"ax"
.global long_half
.type long_half,@function
.align 16
long_half:
	lsr.q r1, r1, #63
	rts
.size long_half,.-long_half
