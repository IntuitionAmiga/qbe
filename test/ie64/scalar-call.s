align 16
add:
	add.l r1, r1, r2
	move.l r1, r1
	sext.l r1, r1
	rts
align 16
main:
	sub.q r31, r31, #8
	li r1, #2
	li r2, #3
	jsr add
	move.l r1, r1
	sext.l r1, r1
	add.q r31, r31, #8
	rts
