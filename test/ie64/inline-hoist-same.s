.section .text,"ax"
.local ee_isdigit
.type ee_isdigit,@function
.align 16
ee_isdigit:
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
.size ee_isdigit,.-ee_isdigit
.section .text,"ax"
.local core_state_transition
.type core_state_transition,@function
.align 16
core_state_transition:
	move.q r3, r2
	load.q r4, (r1)
	load.b r2, (r4)
	bne r2, r0, .L4
.L3:
	moveq r3, #0
	bra .L67
.L4:
	moveq r5, #44
	moveq r6, #0
	bne r2, r5, .Lcmp1
	moveq r6, #1
.Lcmp1:
	move.q r5, r4
	add.q r4, r4, #1
	bne r6, r0, .L66
.L5:
	sub.l r6, r2, #48
	moveq r7, #9
	bls r6, r7, .L21
.L6:
	moveq r6, #43
	beq r2, r6, .L12
.L7:
	moveq r6, #45
	beq r2, r6, .L12
.L8:
	moveq r5, #46
	beq r2, r5, .L10
.L9:
	load.l r2, 4(r3)
	add.l r2, r2, #1
	store.l r2, 4(r3)
	load.l r2, (r3)
	add.l r2, r2, #1
	store.l r2, (r3)
	bra .L61
.L10:
	load.l r2, (r3)
	add.l r2, r2, #1
	store.l r2, (r3)
	move.q r2, r3
	moveq r3, #5
	bra .L32
.L12:
	load.l r2, (r3)
	add.l r2, r2, #1
	store.l r2, (r3)
	load.b r2, (r4)
	bne r2, r0, .L14
.L13:
	moveq r3, #2
	bra .L67
.L14:
	move.q r4, r5
	moveq r5, #44
	add.q r4, r4, #2
	beq r2, r5, .L20
.L16:
	sub.l r5, r2, #48
	moveq r6, #9
	bhi r5, r6, .Lcmp2
	moveq r5, #1
	bra .Lcmpe2
.Lcmp2:
	moveq r5, #0
.Lcmpe2:
	load.l r6, 8(r3)
	add.l r6, r6, #1
	store.l r6, 8(r3)
	beq r5, r0, .L18
.L17:
	move.q r2, r3
	moveq r3, #4
	bra .L23
.L18:
	moveq r5, #46
	bne r2, r5, .L61
.L19:
	move.q r2, r3
	moveq r3, #5
	bra .L32
.L20:
	moveq r3, #2
	bra .L67
.L21:
	load.l r2, (r3)
	add.l r2, r2, #1
	store.l r2, (r3)
	move.q r2, r3
	moveq r3, #4
.L23:
	load.b r5, (r4)
	beq r5, r0, .L67
.L24:
	moveq r6, #44
	add.q r4, r4, #1
	beq r5, r6, .L67
.L25:
	moveq r6, #46
	beq r5, r6, .L29
.L26:
	sub.l r5, r5, #48
	moveq r6, #9
	bls r5, r6, .L23
.L27:
	move.q r3, r2
	load.l r2, 16(r3)
	add.l r2, r2, #1
	store.l r2, 16(r3)
	bra .L61
.L29:
	move.q r3, r2
	load.l r2, 16(r3)
	add.l r2, r2, #1
	store.l r2, 16(r3)
	move.q r2, r3
	moveq r3, #5
.L32:
	load.b r5, (r4)
	beq r5, r0, .L67
.L33:
	moveq r6, #44
	moveq r7, #0
	bne r5, r6, .Lcmp3
	moveq r7, #1
.Lcmp3:
	move.q r6, r4
	add.q r4, r4, #1
	bne r7, r0, .L67
.L34:
	moveq r7, #69
	beq r5, r7, .L41
.L35:
	moveq r7, #101
	beq r5, r7, .L40
.L36:
	sub.l r5, r5, #48
	moveq r6, #9
	bls r5, r6, .L32
.L37:
	move.q r3, r2
	load.l r2, 20(r3)
	add.l r2, r2, #1
	store.l r2, 20(r3)
	bra .L61
.L40:
	move.q r5, r4
	bra .L42
.L41:
	move.q r5, r4
.L42:
	move.q r3, r2
	move.q r4, r6
	load.l r2, 20(r3)
	add.l r2, r2, #1
	store.l r2, 20(r3)
	load.b r2, (r5)
	bne r2, r0, .L45
.L43:
	move.q r4, r5
	moveq r3, #3
	bra .L67
.L45:
	moveq r5, #44
	moveq r6, #0
	bne r2, r5, .Lcmp4
	moveq r6, #1
.Lcmp4:
	add.q r5, r4, #2
	bne r6, r0, .L65
.L46:
	moveq r6, #43
	beq r2, r6, .L50
.L47:
	moveq r6, #45
	beq r2, r6, .L50
.L48:
	load.l r2, 12(r3)
	add.l r2, r2, #1
	store.l r2, 12(r3)
	move.q r4, r5
	bra .L61
.L50:
	load.l r2, 12(r3)
	add.l r2, r2, #1
	store.l r2, 12(r3)
	load.b r2, (r5)
	bne r2, r0, .L53
.L51:
	move.q r4, r5
	moveq r3, #6
	bra .L67
.L53:
	moveq r5, #44
	add.q r4, r4, #3
	beq r2, r5, .L64
.L54:
	sub.l r2, r2, #48
	moveq r5, #9
	bhi r2, r5, .Lcmp5
	moveq r2, #1
	bra .Lcmpe5
.Lcmp5:
	moveq r2, #0
.Lcmpe5:
	load.l r5, 24(r3)
	add.l r5, r5, #1
	store.l r5, 24(r3)
	beq r2, r0, .L61
.L55:
	move.q r2, r3
	moveq r3, #7
	moveq r7, #44
	moveq r8, #9
.L56:
	load.b r5, (r4)
	beq r5, r0, .L67
.L57:
	add.q r4, r4, #1
	beq r5, r7, .L67
.L58:
	sub.l r5, r5, #48
	bls r5, r8, .L56
.L59:
	move.q r3, r2
	load.l r2, 4(r3)
	add.l r2, r2, #1
	store.l r2, 4(r3)
.L61:
	load.b r2, (r4)
	bne r2, r0, .L63
.L62:
	moveq r3, #1
	bra .L67
.L63:
	moveq r3, #1
	bra .L67
.L64:
	moveq r3, #6
	bra .L67
.L65:
	move.q r4, r5
	moveq r3, #3
	bra .L67
.L66:
	moveq r3, #0
.L67:
	store.q r4, (r1)
	move.q r1, r3
	rts
.size core_state_transition,.-core_state_transition
