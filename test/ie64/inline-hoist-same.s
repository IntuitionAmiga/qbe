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
	load.q r10, (r1)
	add.q r4, r2, #8
	add.q r5, r2, #16
	add.q r6, r2, #20
	add.q r7, r2, #12
	add.q r8, r2, #24
	add.q r9, r2, #4
	moveq r3, #0
	moveq r28, #1
	moveq r17, #4
	moveq r26, #6
	moveq r27, #2
	moveq r16, #45
.L4:
	load.b r14, (r10)
	beq r14, r0, .L62
.L5:
	beq r3, r28, .L62
.L6:
	moveq r11, #44
	add.q r10, r10, #1
	beq r14, r11, .L62
.L7:
	moveq r11, #46
	moveq r12, #0
	bne r14, r11, .Lcmp1
	moveq r12, #1
.Lcmp1:
	sub.l r11, r14, #48
	moveq r15, #9
	bhi r11, r15, .Lcmp2
	moveq r11, #1
	bra .Lcmpe2
.Lcmp2:
	moveq r11, #0
.Lcmpe2:
	beq r3, r17, .L56
.L8:
	bhi r17, r3, .L27
.L9:
	beq r3, r26, .L22
.L10:
	bhi r26, r3, .L15
.L11:
	moveq r12, #7
	bne r3, r12, .L4
.L12:
	bne r11, r0, .L4
.L13:
	load.l r3, (r9)
	add.l r3, r3, #1
	store.l r3, (r9)
	moveq r3, #1
	bra .L4
.L15:
	moveq r12, #69
	beq r14, r12, .L20
.L16:
	moveq r12, #101
	beq r14, r12, .L20
.L17:
	bne r11, r0, .L4
.L18:
	load.l r3, (r6)
	add.l r3, r3, #1
	store.l r3, (r6)
	moveq r3, #1
	bra .L4
.L20:
	load.l r3, (r6)
	add.l r3, r3, #1
	store.l r3, (r6)
	moveq r3, #3
	bra .L4
.L22:
	bne r11, r0, .L25
.L23:
	load.l r3, (r8)
	add.l r3, r3, #1
	store.l r3, (r8)
	moveq r3, #1
	bra .L4
.L25:
	load.l r3, (r8)
	add.l r3, r3, #1
	store.l r3, (r8)
	moveq r3, #7
	bra .L4
.L27:
	beq r3, r27, .L48
.L28:
	moveq r13, #43
	bne r14, r13, .Lcmp3
	moveq r13, #1
	bra .Lcmpe3
.Lcmp3:
	moveq r13, #0
.Lcmpe3:
	bne r14, r16, .Lcmp4
	moveq r14, #1
	bra .Lcmpe4
.Lcmp4:
	moveq r14, #0
.Lcmpe4:
	bhi r27, r3, .L35
.L29:
	bne r13, r0, .L33
.L30:
	bne r14, r0, .L33
.L31:
	load.l r3, (r7)
	add.l r3, r3, #1
	store.l r3, (r7)
	moveq r3, #1
	bra .L4
.L33:
	load.l r3, (r7)
	add.l r3, r3, #1
	store.l r3, (r7)
	moveq r3, #6
	bra .L4
.L35:
	bne r3, r0, .L4
.L36:
	move.l r3, r14
	bne r11, r0, .L46
.L38:
	move.l r11, r13
	bne r11, r0, .L45
.L40:
	bne r3, r0, .L45
.L41:
	bne r12, r0, .L44
.L42:
	load.l r3, (r9)
	add.l r3, r3, #1
	store.l r3, (r9)
	moveq r3, #1
	bra .L47
.L44:
	moveq r3, #5
	bra .L47
.L45:
	moveq r3, #2
	bra .L47
.L46:
	moveq r3, #4
.L47:
	load.l r11, (r2)
	add.l r11, r11, #1
	store.l r11, (r2)
	bra .L4
.L48:
	bne r11, r0, .L54
.L49:
	bne r12, r0, .L52
.L50:
	load.l r3, (r4)
	add.l r3, r3, #1
	store.l r3, (r4)
	moveq r3, #1
	bra .L4
.L52:
	load.l r3, (r4)
	add.l r3, r3, #1
	store.l r3, (r4)
	moveq r3, #5
	bra .L4
.L54:
	load.l r3, (r4)
	add.l r3, r3, #1
	store.l r3, (r4)
	moveq r3, #4
	bra .L4
.L56:
	bne r12, r0, .L60
.L57:
	bne r11, r0, .L4
.L58:
	load.l r3, (r5)
	add.l r3, r3, #1
	store.l r3, (r5)
	moveq r3, #1
	bra .L4
.L60:
	load.l r3, (r5)
	add.l r3, r3, #1
	store.l r3, (r5)
	moveq r3, #5
	load.b r14, (r10)
	bne r14, r0, .L5
.L62:
	store.q r10, (r1)
	move.q r1, r3
	rts
.size core_state_transition,.-core_state_transition
