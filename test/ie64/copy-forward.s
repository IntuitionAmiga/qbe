.section .text,"ax"
.global crc_step
.type crc_step,@function
.align 16
crc_step:
	moveq r3, #0
	and.q r3, r3, #0xff
	moveq r4, #8
	moveq r6, #8
	bge r3, r4, .L8
.L3:
	and.l r4, r1, #1
	and.l r5, r2, #1
	eor.l r4, r4, r5
	beq r4, r0, .L5
.L4:
	and.q r2, r2, #0xffff
	eor.l r2, r2, #16386
.L5:
	and.q r2, r2, #0xffff
	asr.l r2, r2, #1
	beq r4, r0, .L7
.L6:
	or.l r2, r2, #32768
.L7:
	add.l r3, r3, #1
	and.q r1, r1, #0xff
	asr.l r1, r1, #1
	and.q r3, r3, #0xff
	blt r3, r6, .L3
.L8:
	and.q r1, r2, #0xffff
	rts
.size crc_step,.-crc_step
