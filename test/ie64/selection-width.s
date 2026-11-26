.section .text,"ax"
.global signed_use
.type signed_use,@function
.align 16
signed_use:
	load.l r4, (r1)
	sext.l r4, r4
	load.l r5, (r2)
	sext.l r5, r5
	moveq r3, #0
	ble r4, r5, .Lcmp0
	moveq r3, #1
.Lcmp0:
	bge r4, r5, .Lcmp1
	moveq r4, #1
	bra .Lcmpe1
.Lcmp1:
	moveq r4, #0
.Lcmpe1:
	add.l r1, r3, r4
	sext.l r1, r1
	rts
.size signed_use,.-signed_use
.section .text,"ax"
.global lower_byte
.type lower_byte,@function
.align 16
lower_byte:
	load.b r1, (r1)
	rts
.size lower_byte,.-lower_byte
.section .text,"ax"
.global volatile_half
.type volatile_half,@function
.align 16
volatile_half:
	load.w r1, (r1)
	and.q r1, r1, #0xff
	rts
.size volatile_half,.-volatile_half
.section .text,"ax"
.global used_half
.type used_half,@function
.align 16
used_half:
	load.w r2, (r1)
	and.q r1, r2, #0xff
	add.q r1, r1, r2
	rts
.size used_half,.-used_half
.section .text,"ax"
.global logical_word
.type logical_word,@function
.align 16
logical_word:
	load.l r1, (r1)
	lsr.l r1, r1, #16
	rts
.size logical_word,.-logical_word
.section .text,"ax"
.global literal_half
.type literal_half,@function
.align 16
literal_half:
	moveq r30, #983040
	load.w r1, (r30)
	and.q r1, r1, #0xff
	rts
.size literal_half,.-literal_half
