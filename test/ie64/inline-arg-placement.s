.section .text,"ax"
.global copy_packet
.type copy_packet,@function
.align 16
copy_packet:
	load.q r3, (r2)
	store.q r3, (r1)
	load.q r3, 8(r2)
	store.q r3, 8(r1)
	load.q r3, 16(r2)
	store.q r3, 16(r1)
	load.q r3, 24(r2)
	store.q r3, 24(r1)
	load.q r3, 32(r2)
	store.q r3, 32(r1)
	rts
.size copy_packet,.-copy_packet
.section .text,"ax"
.global compare_args
.type compare_args,@function
.align 16
compare_args:
	load.l r4, (r1)
	load.l r5, (r2)
	sext.l r3, r4
	sext.l r6, r5
	ble r3, r6, .Lcmp0
	moveq r3, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r3, #0
.Lcmpe0:
	sext.l r4, r4
	sext.l r5, r5
	bge r4, r5, .Lcmp1
	moveq r4, #1
	bra .Lcmpe1
.Lcmp1:
	moveq r4, #0
.Lcmpe1:
	sub.l r1, r3, r4
	sext.l r1, r1
	rts
.size compare_args,.-compare_args
