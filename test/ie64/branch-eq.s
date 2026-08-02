align 16
f:
	and.q r1, r1, #0xffffffff
	li r2, #3
	and.q r2, r2, #0xffffffff
	beq r1, r2, .L2
.L1:
	li r1, #0
	sext.l r1, r1
	rts
.L2:
	li r1, #1
	sext.l r1, r1
	rts
