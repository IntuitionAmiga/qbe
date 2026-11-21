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
	load.q r4, (r1)
	moveq r3, #0
	moveq r15, #1
	moveq r16, #44
	moveq r10, #4
	moveq r17, #9
	moveq r11, #6
	moveq r26, #7
	moveq r27, #69
	moveq r28, #101
	moveq r12, #2
	moveq r13, #43
	moveq r14, #45
	moveq r9, #46
.L4:
	load.b r6, (r4)
	beq r6, r0, .L60
.L5:
	beq r3, r15, .L60
.L6:
	add.q r4, r4, #1
	beq r6, r16, .L60
.L7:
	sub.l r5, r6, #48
	bhi r5, r17, .Lcmp1
	moveq r5, #1
	bra .Lcmpe1
.Lcmp1:
	moveq r5, #0
.Lcmpe1:
	beq r3, r10, .L54
.L8:
	bhi r10, r3, .L27
.L9:
	beq r3, r11, .L22
.L10:
	bhi r11, r3, .L15
.L11:
	bne r3, r26, .L4
.L12:
	bne r5, r0, .L4
.L13:
	load.l r3, 4(r2)
	add.l r3, r3, #1
	store.l r3, 4(r2)
	moveq r3, #1
	bra .L4
.L15:
	beq r6, r27, .L20
.L16:
	beq r6, r28, .L20
.L17:
	bne r5, r0, .L4
.L18:
	load.l r3, 20(r2)
	add.l r3, r3, #1
	store.l r3, 20(r2)
	moveq r3, #1
	bra .L4
.L20:
	load.l r3, 20(r2)
	add.l r3, r3, #1
	store.l r3, 20(r2)
	moveq r3, #3
	bra .L4
.L22:
	bne r5, r0, .L25
.L23:
	load.l r3, 24(r2)
	add.l r3, r3, #1
	store.l r3, 24(r2)
	moveq r3, #1
	bra .L4
.L25:
	load.l r3, 24(r2)
	add.l r3, r3, #1
	store.l r3, 24(r2)
	moveq r3, #7
	bra .L4
.L27:
	beq r3, r12, .L46
.L28:
	bhi r12, r3, .L35
.L29:
	beq r6, r13, .L33
.L30:
	beq r6, r14, .L33
.L31:
	load.l r3, 12(r2)
	add.l r3, r3, #1
	store.l r3, 12(r2)
	moveq r3, #1
	bra .L4
.L33:
	load.l r3, 12(r2)
	add.l r3, r3, #1
	store.l r3, 12(r2)
	moveq r3, #6
	bra .L4
.L35:
	bne r3, r0, .L4
.L36:
	bne r5, r0, .L44
.L37:
	beq r6, r13, .L43
.L38:
	beq r6, r14, .L43
.L39:
	beq r6, r9, .L42
.L40:
	load.l r3, 4(r2)
	add.l r3, r3, #1
	store.l r3, 4(r2)
	moveq r3, #1
	bra .L45
.L42:
	moveq r3, #5
	bra .L45
.L43:
	moveq r3, #2
	bra .L45
.L44:
	moveq r3, #4
.L45:
	load.l r5, (r2)
	add.l r5, r5, #1
	store.l r5, (r2)
	bra .L4
.L46:
	bne r5, r0, .L52
.L47:
	beq r6, r9, .L50
.L48:
	load.l r3, 8(r2)
	add.l r3, r3, #1
	store.l r3, 8(r2)
	moveq r3, #1
	bra .L4
.L50:
	load.l r3, 8(r2)
	add.l r3, r3, #1
	store.l r3, 8(r2)
	moveq r3, #5
	bra .L4
.L52:
	load.l r3, 8(r2)
	add.l r3, r3, #1
	store.l r3, 8(r2)
	moveq r3, #4
	bra .L4
.L54:
	beq r6, r9, .L58
.L55:
	bne r5, r0, .L4
.L56:
	load.l r3, 16(r2)
	add.l r3, r3, #1
	store.l r3, 16(r2)
	moveq r3, #1
	bra .L4
.L58:
	load.l r3, 16(r2)
	add.l r3, r3, #1
	store.l r3, 16(r2)
	moveq r3, #5
	load.b r6, (r4)
	bne r6, r0, .L5
.L60:
	store.q r4, (r1)
	move.q r1, r3
	rts
.size core_state_transition,.-core_state_transition
