.section .text,"ax"
.global word
.type word,@function
.align 16
word:
	neg.l r3, r2
	or.l r2, r2, r3
	asr.l r2, r2, #31
	and.l r2, r2, #137
	and.q r1, r1, #0xff
	asr.l r1, r1, #1
	eor.l r1, r1, r2
	rts
.size word,.-word
.section .text,"ax"
.global long_bits
.type long_bits,@function
.align 16
long_bits:
	neg.l r3, r2
	or.l r2, r2, r3
	asr.l r2, r2, #31
	sext.l r2, r2
	move.l r3, #0x00000009
	movt r3, #0x80000000
	and.q r2, r2, r3
	lsr.q r1, r1, #1
	eor.q r1, r1, r2
	rts
.size long_bits,.-long_bits
.section .text,"ax"
.global overlap
.type overlap,@function
.align 16
overlap:
	beq r2, r0, .L6
.L5:
	eor.l r1, r1, #18
.L6:
	move.l r1, r1
	lsr.l r1, r1, #1
	beq r2, r0, .L8
.L7:
	or.l r1, r1, #128
.L8:
	rts
.size overlap,.-overlap
.section .text,"ax"
.global escape
.type escape,@function
.align 16
escape:
	beq r2, r0, .L11
.L10:
	eor.l r1, r1, #18
.L11:
	and.q r1, r1, #0xff
	lsr.l r1, r1, #1
	bne r2, r0, .L13
.L12:
	move.l r2, r1
	bra .L14
.L13:
	move.l r2, r1
	or.l r1, r1, #128
.L14:
	store.l r2, (r3)
	rts
.size escape,.-escape
