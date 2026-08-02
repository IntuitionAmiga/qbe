align 16
greater_than_zero:
	sext.l r1, r1
	li r2, #0
	sext.l r2, r2
	li r30, #0
	ble r1, r2, .Lcmp0
	li r30, #1
.Lcmp0:
	move.q r1, r30
	move.l r1, r1
	sext.l r1, r1
	rts
