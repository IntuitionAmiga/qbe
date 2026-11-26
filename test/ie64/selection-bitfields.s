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
	lsl.l r3, r2, #16
	eor.l r1, r1, r3
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
	move.q r4, r1
	load.l r1, (r4)
	and.l r5, r1, #4294967264
	or.l r5, r2, r5
	lsl.l r6, r3, #16
	eor.l r5, r5, r6
	store.l r5, (r4)
	sext.l r1, r1
	rts
.size packed_stores,.-packed_stores
.section .text,"ax"
.global volatile_observation
.type volatile_observation,@function
.align 16
volatile_observation:
	move.q r4, r1
	store.l r2, (r4)
	load.l r1, (r4)
	store.l r3, (r4)
	sext.l r1, r1
	rts
.size volatile_observation,.-volatile_observation
.section .text,"ax"
.global literal_stores
.type literal_stores,@function
.align 16
literal_stores:
	moveq r30, #983040
	store.l r1, (r30)
	eor.l r3, r1, r2
	moveq r30, #983040
	store.l r3, (r30)
	rts
.size literal_stores,.-literal_stores
