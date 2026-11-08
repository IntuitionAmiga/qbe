.section .text,"ax"
.global isdigit
.type isdigit,@function
.align 16
isdigit:
	and.q r1, r1, #0xff
	sub.l r1, r1, #48
	moveq r2, #9
	bhi r1, r2, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	rts
.size isdigit,.-isdigit
.section .text,"ax"
.global in_signed
.type in_signed,@function
.align 16
in_signed:
	add.l r1, r1, #4
	moveq r2, #103
	bhi r1, r2, .Lcmp1
	moveq r1, #1
	bra .Lcmpe1
.Lcmp1:
	moveq r1, #0
.Lcmpe1:
	add.l r1, r1, #7
	sext.l r1, r1
	rts
.size in_signed,.-in_signed
.section .text,"ax"
.global in_unsigned
.type in_unsigned,@function
.align 16
in_unsigned:
	sub.l r1, r1, #10
	move.l r2, #0xee6b27f6
	bls r1, r2, .L4
.L3:
	moveq r1, #0
	rts
.L4:
	moveq r1, #1
	rts
.size in_unsigned,.-in_unsigned
.section .text,"ax"
.global mixed
.type mixed,@function
.align 16
mixed:
	move.l r2, r1
	sext.l r1, r2
	moveq r3, #48
	blt r1, r3, .Lcmp2
	moveq r1, #1
	bra .Lcmpe2
.Lcmp2:
	moveq r1, #0
.Lcmpe2:
	move.l r2, r2
	moveq r3, #57
	bhi r2, r3, .Lcmp3
	moveq r2, #1
	bra .Lcmpe3
.Lcmp3:
	moveq r2, #0
.Lcmpe3:
	and.l r1, r1, r2
	rts
.size mixed,.-mixed
.section .text,"ax"
.global reused
.type reused,@function
.align 16
reused:
	move.l r2, r1
	sext.l r1, r2
	moveq r3, #48
	blt r1, r3, .Lcmp4
	moveq r1, #1
	bra .Lcmpe4
.Lcmp4:
	moveq r1, #0
.Lcmpe4:
	sext.l r2, r2
	moveq r3, #57
	bgt r2, r3, .Lcmp5
	moveq r2, #1
	bra .Lcmpe5
.Lcmp5:
	moveq r2, #0
.Lcmpe5:
	and.l r2, r1, r2
	add.l r1, r1, r2
	sext.l r1, r1
	rts
.size reused,.-reused
