.section .text,"ax"
.global signed_word
.type signed_word,@function
.align 16
signed_word:
	load.l r1, (r1)
	load.l r2, (r2)
	sext.l r1, r1
	sext.l r2, r2
	blt r1, r2, .Lcmp3l0
	bne r1, r2, .Lcmp3g0
	moveq r1, #0
	bra .Lcmp3e0
.Lcmp3g0:
	moveq r1, #1
	bra .Lcmp3e0
.Lcmp3l0:
	moveq r1, #-1
.Lcmp3e0:
	rts
.size signed_word,.-signed_word
.section .text,"ax"
.global unsigned_long
.type unsigned_long,@function
.align 16
unsigned_long:
	bhi r2, r1, .Lcmp3l1
	bne r1, r2, .Lcmp3g1
	moveq r1, #0
	bra .Lcmp3e1
.Lcmp3g1:
	moveq r1, #1
	bra .Lcmp3e1
.Lcmp3l1:
	moveq r1, #-1
.Lcmp3e1:
	rts
.size unsigned_long,.-unsigned_long
.section .text,"ax"
.global different_pairs
.type different_pairs,@function
.align 16
different_pairs:
	move.l r4, r2
	move.l r2, r1
	sext.l r1, r2
	sext.l r4, r4
	ble r1, r4, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	sext.l r2, r2
	sext.l r3, r3
	bge r2, r3, .Lcmp1
	moveq r2, #1
	bra .Lcmpe1
.Lcmp1:
	moveq r2, #0
.Lcmpe1:
	sub.l r1, r1, r2
	sext.l r1, r1
	rts
.size different_pairs,.-different_pairs
