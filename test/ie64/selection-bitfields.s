.section .text,"ax"
.global extract_low
.type extract_low,@function
.align 16
extract_low:
	and.l r1, r1, #31
	rts
.size extract_low,.-extract_low
.section .text,"ax"
.global pack_signed
.type pack_signed,@function
.align 16
pack_signed:
	and.l r1, r1, #511
	eor.l r1, r1, #256
	lsl.l r1, r1, #5
	sext.l r1, r1
	rts
.size pack_signed,.-pack_signed
.section .text,"ax"
.global xor_high
.type xor_high,@function
.align 16
xor_high:
	lsl.l r2, r2, #16
	eor.l r1, r1, r2
	sext.l r1, r1
	rts
.size xor_high,.-xor_high
.section .text,"ax"
.global partial_mask
.type partial_mask,@function
.align 16
partial_mask:
	lsl.l r1, r1, #5
	and.l r1, r1, #65504
	rts
.size partial_mask,.-partial_mask
