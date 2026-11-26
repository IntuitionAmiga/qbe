.section .text,"ax"
.global load_bits
.type load_bits,@function
.align 16
load_bits:
	fload f0, (r1)
	moveq r3, #0
	store.l r3, (r2)
	rts
.size load_bits,.-load_bits
.section .text,"ax"
.global load_double_bits
.type load_double_bits,@function
.align 16
load_double_bits:
	dload f0, (r1)
	rts
.size load_double_bits,.-load_double_bits
.section .text,"ax"
.global shared_bits
.type shared_bits,@function
.align 16
shared_bits:
	load.l r3, (r1)
	store.l r3, (r2)
	fmovi f0, r3
	rts
.size shared_bits,.-shared_bits
.section .text,"ax"
.global volatile_bits
.type volatile_bits,@function
.align 16
volatile_bits:
	load.l r1, (r1)
	fmovi f0, r1
	rts
.size volatile_bits,.-volatile_bits
.section .text,"ax"
.global status_between
.type status_between,@function
.align 16
status_between:
	load.l r3, (r1)
	fmovsr r4
	store.q r4, (r2)
	fmovi f0, r3
	rts
.size status_between,.-status_between
.section .text,"ax"
.global overwritten_status
.type overwritten_status,@function
.align 16
overwritten_status:
	fmov f2, f0
	fmov f0, f1
	fload f1, (r1)
	fadd f0, f2, f0
	fadd f0, f0, f1
	rts
.size overwritten_status,.-overwritten_status
.section .text,"ax"
.global live_status
.type live_status,@function
.align 16
live_status:
	load.l r3, (r1)
	fadd f0, f0, f1
	fstore f0, (r2)
	fmovi f0, r3
	rts
.size live_status,.-live_status
