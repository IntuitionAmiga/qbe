align 16
call:
	sub.q r31, r31, #8
	move.q r2, r1
	li r1, #2
	jsr (r2)
	move.l r1, r1
	sext.l r1, r1
	add.q r31, r31, #8
	rts
