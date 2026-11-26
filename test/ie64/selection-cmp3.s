.section .text,"ax"
.global signed_word
.type signed_word,@function
.align 16
signed_word:
	load.l r3, (r1)
	load.l r4, (r2)
	sext.l r3, r3
	sext.l r4, r4
	blt r3, r4, .Lcmp3l0
	bne r3, r4, .Lcmp3g0
	moveq r1, #0
	rts
.Lcmp3g0:
	moveq r1, #1
	rts
.Lcmp3l0:
	moveq r1, #-1
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
	rts
.Lcmp3g1:
	moveq r1, #1
	rts
.Lcmp3l1:
	moveq r1, #-1
	rts
.size unsigned_long,.-unsigned_long
.section .text,"ax"
.global different_pairs
.type different_pairs,@function
.align 16
different_pairs:
	sext.l r4, r1
	sext.l r5, r2
	ble r4, r5, .Lcmp0
	moveq r4, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r4, #0
.Lcmpe0:
	sext.l r5, r1
	sext.l r6, r3
	bge r5, r6, .Lcmp1
	moveq r5, #1
	bra .Lcmpe1
.Lcmp1:
	moveq r5, #0
.Lcmpe1:
	sub.l r1, r4, r5
	sext.l r1, r1
	rts
.size different_pairs,.-different_pairs
.section .text,"ax"
.global framed_return
.type framed_return,@function
.align 16
framed_return:
	sub.q r31, r31, #24
	store.q r18, 8(r31)
	store.q r19, 16(r31)
	lea r3, 0(r31)
	move.q r18, r1
	move.q r1, r3
	move.q r19, r2
	jsr opaque
	blt r18, r19, .Lcmp3l2
	bne r18, r19, .Lcmp3g2
	moveq r1, #0
	bra .Lcmp3e2
.Lcmp3g2:
	moveq r1, #1
	bra .Lcmp3e2
.Lcmp3l2:
	moveq r1, #-1
.Lcmp3e2:
	load.q r18, 8(r31)
	load.q r19, 16(r31)
	add.q r31, r31, #24
	rts
.size framed_return,.-framed_return
