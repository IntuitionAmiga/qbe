.section .text,"ax"
.global cmp_loaded
.type cmp_loaded,@function
.align 16
cmp_loaded:
	load.l r1, (r1)
	load.l r2, (r2)
	bls r2, r1, .Lcmp0
	moveq r1, #1
	bra .Lcmpe0
.Lcmp0:
	moveq r1, #0
.Lcmpe0:
	rts
.size cmp_loaded,.-cmp_loaded
.section .text,"ax"
.global cmp_masked
.type cmp_masked,@function
.align 16
cmp_masked:
	and.l r1, r1, #255
	moveq r2, #100
	bge r1, r2, .Lcmp1
	moveq r1, #1
	bra .Lcmpe1
.Lcmp1:
	moveq r1, #0
.Lcmpe1:
	rts
.size cmp_masked,.-cmp_masked
.section .text,"ax"
.global cmp_param
.type cmp_param,@function
.align 16
cmp_param:
	move.l r1, r1
	move.l r2, r2
	bls r2, r1, .Lcmp2
	moveq r1, #1
	bra .Lcmpe2
.Lcmp2:
	moveq r1, #0
.Lcmpe2:
	rts
.size cmp_param,.-cmp_param
.section .text,"ax"
.global zext_after_add
.type zext_after_add,@function
.align 16
zext_after_add:
	add.l r1, r1, r2
	rts
.size zext_after_add,.-zext_after_add
.section .text,"ax"
.global zext_param
.type zext_param,@function
.align 16
zext_param:
	move.l r1, r1
	rts
.size zext_param,.-zext_param
.section .text,"ax"
.global sext_masked
.type sext_masked,@function
.align 16
sext_masked:
	and.l r1, r1, #127
	rts
.size sext_masked,.-sext_masked
.section .text,"ax"
.global sext_param
.type sext_param,@function
.align 16
sext_param:
	sext.l r1, r1
	rts
.size sext_param,.-sext_param
.section .text,"ax"
.global byte_twice
.type byte_twice,@function
.align 16
byte_twice:
	and.q r1, r1, #0xff
	rts
.size byte_twice,.-byte_twice
.section .text,"ax"
.global switch_chain
.type switch_chain,@function
.align 16
switch_chain:
	move.l r1, r1
	moveq r2, #1
	beq r1, r2, .L14
.L9:
	moveq r2, #2
	beq r1, r2, .L13
.L10:
	moveq r2, #3
	beq r1, r2, .L12
.L11:
	moveq r1, #0
	rts
.L12:
	moveq r1, #30
	rts
.L13:
	moveq r1, #20
	rts
.L14:
	moveq r1, #10
	rts
.size switch_chain,.-switch_chain
