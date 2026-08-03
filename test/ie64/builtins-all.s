.section .text,"ax"
.global facilities
.type facilities,@function
.align 16
facilities:
	sub.q r31, r31, #56
	store.q r18, 0(r31)
	store.q r19, 8(r31)
	store.q r20, 16(r31)
	fstore f8, 24(r31)
	fstore f10, 32(r31)
	fstore f11, 40(r31)
	move.q r20, r3
	fmov f8, f0
	dmov f10, f2
	nop
	sei
	cli
	tlbflush
	tlbinval r2
	suaen
	suadis
	wait #5
	mtcr cr6, r2
	move.q r19, r2
	mfcr r2, cr6
	move.q r18, r1
	smode r1
	syscall #7
	dmov f2, f10
	fmov f0, f8
	move.q r3, r20
	move.q r2, r19
	move.q r1, r18
	xchg r4, (r1), r3
	faa r4, (r1), r3
	fand r4, (r1), r3
	for r4, (r1), r3
	fxor r4, (r1), r3
	move.q r29, r3
	move.q r30, r1
	move.q r1, r2
	cas r1, (r30), r29
	fmovecr f1, #0
	fmovecr f4, #1
	fcvtsd f4, f4
	fmod f1, f0, f0
	dmod f4, f2, f2
	fabs f1, f0
	dabs f4, f2
	fint f1, f0
	dint f4, f2
	fcvtsd f4, f0
	fcvtds f1, f2
	fsin f1, f0
	fcos f1, f0
	ftan f1, f0
	fatan f1, f0
	flog f1, f0
	fexp f1, f0
	fpow f1, f0, f1
	fsqrt f0, f0
	dsin f0, f2
	dcos f0, f2
	dtan f0, f2
	datan f0, f2
	dlog f0, f2
	dexp f0, f2
	dpow f0, f2, f0
	dsqrt f0, f2
	fmovsr r1
	fmovcr r1
	li r1, #1
	fmovsc r1
	li r1, #2
	fmovcc r1
.L1:
	load.q r18, 0(r31)
	load.q r19, 8(r31)
	load.q r20, 16(r31)
	fload f8, 24(r31)
	fload f10, 32(r31)
	fload f11, 40(r31)
	add.q r31, r31, #56
	rts
.size facilities,.-facilities
.section .text,"ax"
.global stop
.type stop,@function
.align 16
stop:
	halt
	halt
.size stop,.-stop
.section .text,"ax"
.global rettrap
.type rettrap,@function
.align 16
rettrap:
	eret
	halt
.size rettrap,.-rettrap
.section .text,"ax"
.global retirq
.type retirq,@function
.align 16
retirq:
	rti
	halt
.size retirq,.-retirq
