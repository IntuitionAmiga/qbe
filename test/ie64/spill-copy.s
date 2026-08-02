align 16
spill_copy:
	sub.q r31, r31, #88
	store.q r18, 32(r31)
	store.q r19, 40(r31)
	store.q r20, 48(r31)
	store.q r21, 56(r31)
	store.q r22, 64(r31)
	store.q r23, 72(r31)
	store.q r24, 80(r31)
	move.q r22, r1
	move.q r23, r2
	move.q r24, r3
	move.q r21, r4
	move.q r20, r5
	move.q r19, r6
	load.q r18, 96(r31)
	load.q r4, 104(r31)
	lea r30, 0(r31)
	store.q r4, (r30)
	load.q r1, 112(r31)
	lea r30, 8(r31)
	store.q r1, (r30)
	load.q r6, 120(r31)
	lea r30, 16(r31)
	store.q r6, (r30)
	jsr clobber
	move.q r3, r24
	move.q r2, r23
	load.q r6, 16(r31)
	load.q r1, 8(r31)
	load.q r4, 0(r31)
	add.q r2, r22, r2
	add.q r5, r3, r21
	add.q r3, r20, r19
	add.q r4, r18, r4
	add.q r1, r1, r6
	add.q r2, r2, r5
	add.q r3, r3, r4
	add.q r2, r2, r3
	add.q r1, r1, r2
	move.q r1, r1
	load.q r18, 32(r31)
	load.q r19, 40(r31)
	load.q r20, 48(r31)
	load.q r21, 56(r31)
	load.q r22, 64(r31)
	load.q r23, 72(r31)
	load.q r24, 80(r31)
	add.q r31, r31, #88
	rts
align 16
clobber:
.L2:
	rts
