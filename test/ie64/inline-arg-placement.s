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
.size compare_args,.-compare_args
