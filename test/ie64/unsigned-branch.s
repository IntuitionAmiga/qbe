align 16
unsigned_less:
	bhi r2, r1, .L2
.L1:
	li r1, #0
	sext.l r1, r1
	rts
.L2:
	li r1, #1
	sext.l r1, r1
	rts
align 16
unsigned_greater:
	bhi r1, r2, .L5
.L4:
	li r1, #0
	sext.l r1, r1
	rts
.L5:
	li r1, #1
	sext.l r1, r1
	rts
