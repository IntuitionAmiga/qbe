.section .text,"ax"
.global f
.type f,@function
.align 16
f:
	move.l r1, r1
	bne r2, r0, .L2
.L1:
	moveq r2, #46
	beq r1, r2, .L3
	bra .L4
.L2:
	moveq r2, #46
	beq r1, r2, .L4
.L3:
	moveq r1, #2
	rts
.L4:
	moveq r1, #1
	rts
.size f,.-f
.section .text,"ax"
.global g
.type g,@function
.align 16
g:
	move.l r1, r1
	moveq r4, #46
	bne r1, r4, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	bne r2, r0, .L7
.L6:
	store.l r1, (r3)
	bne r1, r0, .L8
	bra .L9
.L7:
	bne r1, r0, .L9
.L8:
	moveq r1, #2
	rts
.L9:
	moveq r1, #1
	rts
.size g,.-g
