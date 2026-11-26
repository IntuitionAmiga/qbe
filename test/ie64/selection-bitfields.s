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
.section .text,"ax"
.global packed_stores
.type packed_stores,@function
.align 16
packed_stores:
	move.l r4, r3
	move.l r3, r2
	move.q r2, r1
	load.l r1, (r2)
	and.l r5, r1, #4294967264
	or.l r3, r3, r5
	lsl.l r4, r4, #16
	eor.l r3, r3, r4
	store.l r3, (r2)
	sext.l r1, r1
	rts
.size packed_stores,.-packed_stores
.section .text,"ax"
.global volatile_observation
.type volatile_observation,@function
.align 16
volatile_observation:
	move.q r30, r1
	move.q r1, r2
	move.q r2, r30
	store.l r1, (r2)
	load.l r1, (r2)
	store.l r3, (r2)
	sext.l r1, r1
	rts
.size volatile_observation,.-volatile_observation
