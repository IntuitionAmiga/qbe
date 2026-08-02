align 16
signed_byte:
	sext.b r1, r1
	move.q r1, r1
	rts
align 16
unsigned_half:
	and.q r1, r1, #0xffff
	move.q r1, r1
	rts
align 16
call_narrow:
	sub.q r31, r31, #8
	store.q r18, 0(r31)
	move.l r18, r1
	sext.b r1, r1
	jsr signed_byte
	move.l r30, r1
	move.l r1, r18
	move.l r18, r30
	and.q r1, r1, #0xffff
	jsr unsigned_half
	add.l r1, r18, r1
	move.l r1, r1
	sext.l r1, r1
	load.q r18, 0(r31)
	add.q r31, r31, #8
	rts
