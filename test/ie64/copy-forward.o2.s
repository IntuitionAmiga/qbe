.section .text,"ax"
.global crc_step
.type crc_step,@function
.align 16
crc_step:
	and.q r2, r2, #0xffff
	and.q r3, r1, #0xff
	moveq r1, #0
	moveq r4, #8
	moveq r5, #8
	bls r4, r1, .L5
.L3:
	eor.l r4, r3, r2
	and.l r4, r4, #1
	neg.l r4, r4
	and.l r4, r4, #40961
	asr.l r2, r2, #1
	eor.l r2, r2, r4
	add.l r1, r1, #1
	asr.l r3, r3, #1
	bhi r5, r1, .L3
.L5:
	move.q r1, r2
	rts
.size crc_step,.-crc_step
