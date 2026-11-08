.section .text,"ax"
.global load_off
.type load_off,@function
.align 16
load_off:
	load.q r1, 24(r1)
	rts
.size load_off,.-load_off
.section .text,"ax"
.global load_neg
.type load_neg,@function
.align 16
load_neg:
	load.l r1, -12(r1)
	sext.l r1, r1
	rts
.size load_neg,.-load_neg
.section .text,"ax"
.global load_byte
.type load_byte,@function
.align 16
load_byte:
	load.b r1, 3(r1)
	rts
.size load_byte,.-load_byte
.section .text,"ax"
.global store_off
.type store_off,@function
.align 16
store_off:
	store.l r2, 40(r1)
.L4:
	rts
.size store_off,.-store_off
.section .text,"ax"
.global load_double
.type load_double,@function
.align 16
load_double:
	dload f0, 16(r1)
.L6:
	rts
.size load_double,.-load_double
.section .text,"ax"
.global store_single
.type store_single,@function
.align 16
store_single:
	fstore f0, 8(r1)
.L8:
	rts
.size store_single,.-store_single
.section .text,"ax"
.global copy_off
.type copy_off,@function
.align 16
copy_off:
	load.q r2, 64(r1)
	add.q r2, r1, r2
	store.q r2, 64(r1)
	rts
.size copy_off,.-copy_off
.section .text,"ax"
.global base_overwritten
.type base_overwritten,@function
.align 16
base_overwritten:
	add.q r2, r1, #64
	load.q r1, (r2)
	add.q r1, r1, #1
	store.q r1, (r2)
.L11:
	rts
.size base_overwritten,.-base_overwritten
.section .text,"ax"
.global addr_escapes
.type addr_escapes,@function
.align 16
addr_escapes:
	add.q r1, r1, #24
	load.q r2, (r1)
	add.q r1, r1, r2
	rts
.size addr_escapes,.-addr_escapes
.section .text,"ax"
.global addr_stored
.type addr_stored,@function
.align 16
addr_stored:
	add.q r1, r1, #24
	store.q r1, (r2)
.L14:
	rts
.size addr_stored,.-addr_stored
.section .text,"ax"
.global far_off
.type far_off,@function
.align 16
far_off:
	move.l r2, #0x00000000
	movt r2, #0x00000001
	add.q r1, r1, r2
	load.q r1, (r1)
	rts
.size far_off,.-far_off
.section .text,"ax"
.global addr_live_out
.type addr_live_out,@function
.align 16
addr_live_out:
	sub.q r31, r31, #8
	store.q r18, 0(r31)
	add.q r1, r1, #8
	load.q r18, (r1)
	bne r2, r0, .L18
.L17:
	move.q r1, r18
	load.q r18, 0(r31)
	add.q r31, r31, #8
	rts
.L18:
	jsr ext2
	move.q r1, r18
	load.q r18, 0(r31)
	add.q r31, r31, #8
	rts
.size addr_live_out,.-addr_live_out
.section .text,"ax"
.global across_call
.type across_call,@function
.align 16
across_call:
	sub.q r31, r31, #8
	store.q r18, 0(r31)
	add.q r18, r1, #8
	jsr ext
	load.q r1, (r18)
	load.q r18, 0(r31)
	add.q r31, r31, #8
	rts
.size across_call,.-across_call
.section .text,"ax"
.global base_reused
.type base_reused,@function
.align 16
base_reused:
	move.q r30, r1
	move.q r1, r2
	move.q r2, r30
	mulu.q r1, r1, r1
	load.q r2, 8(r2)
	add.q r1, r1, r2
	rts
.size base_reused,.-base_reused
.section .text,"ax"
.global slot_field
.type slot_field,@function
.align 16
slot_field:
	sub.q r31, r31, #40
	store.q r18, 32(r31)
	lea r18, 0(r31)
	move.q r1, r18
	jsr ext2
	add.q r1, r18, #16
	load.q r18, (r1)
	jsr ext2
	move.q r1, r18
	load.q r18, 32(r31)
	add.q r31, r31, #40
	rts
.size slot_field,.-slot_field
.section .text,"ax"
.global load_negative_add
.type load_negative_add,@function
.align 16
load_negative_add:
	load.q r1, -24(r1)
	rts
.size load_negative_add,.-load_negative_add
.section .text,"ax"
.global load_far_negative
.type load_far_negative,@function
.align 16
load_far_negative:
	sub.q r1, r1, #4294967295
	load.q r1, (r1)
	rts
.size load_far_negative,.-load_far_negative
