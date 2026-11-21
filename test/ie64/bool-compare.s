.section .text,"ax"
.global phi
.type phi,@function
.align 16
phi:
	move.l r1, r1
	bne r2, r0, .L3
.L2:
	moveq r2, #9
	bhi r1, r2, .L4
	bra .L5
.L3:
	moveq r2, #9
	bhi r1, r2, .L5
.L4:
	moveq r1, #2
	rts
.L5:
	moveq r1, #1
	rts
.size phi,.-phi
.section .text,"ax"
.global ext
.type ext,@function
.align 16
ext:
	move.l r1, r1
	bne r2, r0, .L8
.L7:
	moveq r2, #46
	beq r1, r2, .L9
	bra .L10
.L8:
	moveq r2, #46
	beq r1, r2, .L10
.L9:
	moveq r1, #2
	rts
.L10:
	moveq r1, #1
	rts
.size ext,.-ext
.section .text,"ax"
.global ext64
.type ext64,@function
.align 16
ext64:
	move.l r1, r1
	bne r2, r0, .L13
.L12:
	moveq r2, #46
	bne r1, r2, .L14
	bra .L15
.L13:
	moveq r2, #46
	bne r1, r2, .L15
.L14:
	moveq r1, #2
	rts
.L15:
	moveq r1, #1
	rts
.size ext64,.-ext64
.section .text,"ax"
.global ext64value
.type ext64value,@function
.align 16
ext64value:
	move.l r1, r1
	moveq r3, #46
	beq r1, r3, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	bne r2, r0, .L18
.L17:
	rts
.L18:
	bne r1, r0, .L20
.L19:
	moveq r1, #2
	rts
.L20:
	moveq r1, #1
	rts
.size ext64value,.-ext64value
.section .text,"ax"
.global inverted
.type inverted,@function
.align 16
inverted:
	move.l r1, r1
	moveq r3, #9
	bhi r1, r3, .L23
.L22:
	moveq r1, #1
	bra .L24
.L23:
	moveq r1, #0
.L24:
	bne r2, r0, .L26
.L25:
	bne r1, r0, .L27
	bra .L28
.L26:
	bne r1, r0, .L28
.L27:
	moveq r1, #2
	rts
.L28:
	moveq r1, #1
	rts
.size inverted,.-inverted
.section .text,"ax"
.global range
.type range,@function
.align 16
range:
	move.l r3, r1
	sub.l r1, r3, #48
	bne r2, r0, .L32
.L31:
	moveq r2, #9
	bls r1, r2, .L33
	bra .L34
.L32:
	moveq r2, #9
	bls r1, r2, .L34
.L33:
	moveq r1, #2
	rts
.L34:
	moveq r1, #1
	rts
.size range,.-range
