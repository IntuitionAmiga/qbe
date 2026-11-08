.section .text,"ax"
.global w_eq_R_n
.type w_eq_R_n,@function
.align 16
w_eq_R_n:
	move.l r1, r1
	moveq r2, #7
	beq r1, r2, .L2
.L1:
	moveq r1, #2
	rts
.L2:
	moveq r1, #1
	rts
.size w_eq_R_n,.-w_eq_R_n
.section .text,"ax"
.global w_eq_R_y
.type w_eq_R_y,@function
.align 16
w_eq_R_y:
	moveq r3, #7
.L4:
	sub.l r1, r1, #1
	bne r1, r3, .L4
.L5:
	moveq r1, #1
	rts
.size w_eq_R_y,.-w_eq_R_y
.section .text,"ax"
.global w_eq_L_n
.type w_eq_L_n,@function
.align 16
w_eq_L_n:
	move.l r1, r1
	moveq r2, #7
	beq r1, r2, .L8
.L7:
	moveq r1, #2
	rts
.L8:
	moveq r1, #1
	rts
.size w_eq_L_n,.-w_eq_L_n
.section .text,"ax"
.global w_eq_L_y
.type w_eq_L_y,@function
.align 16
w_eq_L_y:
	moveq r3, #7
.L10:
	sub.l r1, r1, #1
	bne r1, r3, .L10
.L11:
	moveq r1, #1
	rts
.size w_eq_L_y,.-w_eq_L_y
.section .text,"ax"
.global w_ne_R_n
.type w_ne_R_n,@function
.align 16
w_ne_R_n:
	move.l r1, r1
	moveq r2, #7
	bne r1, r2, .L14
.L13:
	moveq r1, #2
	rts
.L14:
	moveq r1, #1
	rts
.size w_ne_R_n,.-w_ne_R_n
.section .text,"ax"
.global w_ne_R_y
.type w_ne_R_y,@function
.align 16
w_ne_R_y:
	moveq r3, #7
.L16:
	sub.l r1, r1, #1
	beq r1, r3, .L16
.L17:
	moveq r1, #1
	rts
.size w_ne_R_y,.-w_ne_R_y
.section .text,"ax"
.global w_ne_L_n
.type w_ne_L_n,@function
.align 16
w_ne_L_n:
	move.l r1, r1
	moveq r2, #7
	bne r1, r2, .L20
.L19:
	moveq r1, #2
	rts
.L20:
	moveq r1, #1
	rts
.size w_ne_L_n,.-w_ne_L_n
.section .text,"ax"
.global w_ne_L_y
.type w_ne_L_y,@function
.align 16
w_ne_L_y:
	moveq r3, #7
.L22:
	sub.l r1, r1, #1
	beq r1, r3, .L22
.L23:
	moveq r1, #1
	rts
.size w_ne_L_y,.-w_ne_L_y
.section .text,"ax"
.global w_slt_R_n
.type w_slt_R_n,@function
.align 16
w_slt_R_n:
	sext.l r1, r1
	moveq r2, #7
	blt r1, r2, .L26
.L25:
	moveq r1, #2
	rts
.L26:
	moveq r1, #1
	rts
.size w_slt_R_n,.-w_slt_R_n
.section .text,"ax"
.global w_slt_R_y
.type w_slt_R_y,@function
.align 16
w_slt_R_y:
	moveq r4, #7
.L28:
	sub.l r1, r1, #1
	sext.l r2, r1
	bge r2, r4, .L28
.L29:
	moveq r1, #1
	rts
.size w_slt_R_y,.-w_slt_R_y
.section .text,"ax"
.global w_slt_L_n
.type w_slt_L_n,@function
.align 16
w_slt_L_n:
	move.l r2, r1
	moveq r1, #7
	sext.l r2, r2
	blt r1, r2, .L32
.L31:
	moveq r1, #2
	rts
.L32:
	moveq r1, #1
	rts
.size w_slt_L_n,.-w_slt_L_n
.section .text,"ax"
.global w_slt_L_y
.type w_slt_L_y,@function
.align 16
w_slt_L_y:
	moveq r4, #7
.L34:
	sub.l r1, r1, #1
	sext.l r3, r1
	bge r4, r3, .L34
.L35:
	moveq r1, #1
	rts
.size w_slt_L_y,.-w_slt_L_y
.section .text,"ax"
.global w_sle_R_n
.type w_sle_R_n,@function
.align 16
w_sle_R_n:
	sext.l r1, r1
	moveq r2, #7
	ble r1, r2, .L38
.L37:
	moveq r1, #2
	rts
.L38:
	moveq r1, #1
	rts
.size w_sle_R_n,.-w_sle_R_n
.section .text,"ax"
.global w_sle_R_y
.type w_sle_R_y,@function
.align 16
w_sle_R_y:
	moveq r4, #7
.L40:
	sub.l r1, r1, #1
	sext.l r2, r1
	bgt r2, r4, .L40
.L41:
	moveq r1, #1
	rts
.size w_sle_R_y,.-w_sle_R_y
.section .text,"ax"
.global w_sle_L_n
.type w_sle_L_n,@function
.align 16
w_sle_L_n:
	move.l r2, r1
	moveq r1, #7
	sext.l r2, r2
	ble r1, r2, .L44
.L43:
	moveq r1, #2
	rts
.L44:
	moveq r1, #1
	rts
.size w_sle_L_n,.-w_sle_L_n
.section .text,"ax"
.global w_sle_L_y
.type w_sle_L_y,@function
.align 16
w_sle_L_y:
	moveq r4, #7
.L46:
	sub.l r1, r1, #1
	sext.l r3, r1
	bgt r4, r3, .L46
.L47:
	moveq r1, #1
	rts
.size w_sle_L_y,.-w_sle_L_y
.section .text,"ax"
.global w_sgt_R_n
.type w_sgt_R_n,@function
.align 16
w_sgt_R_n:
	sext.l r1, r1
	moveq r2, #7
	bgt r1, r2, .L50
.L49:
	moveq r1, #2
	rts
.L50:
	moveq r1, #1
	rts
.size w_sgt_R_n,.-w_sgt_R_n
.section .text,"ax"
.global w_sgt_R_y
.type w_sgt_R_y,@function
.align 16
w_sgt_R_y:
	moveq r4, #7
.L52:
	sub.l r1, r1, #1
	sext.l r2, r1
	ble r2, r4, .L52
.L53:
	moveq r1, #1
	rts
.size w_sgt_R_y,.-w_sgt_R_y
.section .text,"ax"
.global w_sgt_L_n
.type w_sgt_L_n,@function
.align 16
w_sgt_L_n:
	move.l r2, r1
	moveq r1, #7
	sext.l r2, r2
	bgt r1, r2, .L56
.L55:
	moveq r1, #2
	rts
.L56:
	moveq r1, #1
	rts
.size w_sgt_L_n,.-w_sgt_L_n
.section .text,"ax"
.global w_sgt_L_y
.type w_sgt_L_y,@function
.align 16
w_sgt_L_y:
	moveq r4, #7
.L58:
	sub.l r1, r1, #1
	sext.l r3, r1
	ble r4, r3, .L58
.L59:
	moveq r1, #1
	rts
.size w_sgt_L_y,.-w_sgt_L_y
.section .text,"ax"
.global w_sge_R_n
.type w_sge_R_n,@function
.align 16
w_sge_R_n:
	sext.l r1, r1
	moveq r2, #7
	bge r1, r2, .L62
.L61:
	moveq r1, #2
	rts
.L62:
	moveq r1, #1
	rts
.size w_sge_R_n,.-w_sge_R_n
.section .text,"ax"
.global w_sge_R_y
.type w_sge_R_y,@function
.align 16
w_sge_R_y:
	moveq r4, #7
.L64:
	sub.l r1, r1, #1
	sext.l r2, r1
	blt r2, r4, .L64
.L65:
	moveq r1, #1
	rts
.size w_sge_R_y,.-w_sge_R_y
.section .text,"ax"
.global w_sge_L_n
.type w_sge_L_n,@function
.align 16
w_sge_L_n:
	move.l r2, r1
	moveq r1, #7
	sext.l r2, r2
	bge r1, r2, .L68
.L67:
	moveq r1, #2
	rts
.L68:
	moveq r1, #1
	rts
.size w_sge_L_n,.-w_sge_L_n
.section .text,"ax"
.global w_sge_L_y
.type w_sge_L_y,@function
.align 16
w_sge_L_y:
	moveq r4, #7
.L70:
	sub.l r1, r1, #1
	sext.l r3, r1
	blt r4, r3, .L70
.L71:
	moveq r1, #1
	rts
.size w_sge_L_y,.-w_sge_L_y
.section .text,"ax"
.global w_ult_R_n
.type w_ult_R_n,@function
.align 16
w_ult_R_n:
	move.l r1, r1
	moveq r2, #7
	bhi r2, r1, .L74
.L73:
	moveq r1, #2
	rts
.L74:
	moveq r1, #1
	rts
.size w_ult_R_n,.-w_ult_R_n
.section .text,"ax"
.global w_ult_R_y
.type w_ult_R_y,@function
.align 16
w_ult_R_y:
	moveq r3, #7
.L76:
	sub.l r1, r1, #1
	bls r3, r1, .L76
.L77:
	moveq r1, #1
	rts
.size w_ult_R_y,.-w_ult_R_y
.section .text,"ax"
.global w_ult_L_n
.type w_ult_L_n,@function
.align 16
w_ult_L_n:
	move.l r2, r1
	moveq r1, #7
	move.l r2, r2
	bhi r2, r1, .L80
.L79:
	moveq r1, #2
	rts
.L80:
	moveq r1, #1
	rts
.size w_ult_L_n,.-w_ult_L_n
.section .text,"ax"
.global w_ult_L_y
.type w_ult_L_y,@function
.align 16
w_ult_L_y:
	moveq r3, #7
.L82:
	sub.l r1, r1, #1
	bls r1, r3, .L82
.L83:
	moveq r1, #1
	rts
.size w_ult_L_y,.-w_ult_L_y
.section .text,"ax"
.global w_ule_R_n
.type w_ule_R_n,@function
.align 16
w_ule_R_n:
	move.l r1, r1
	moveq r2, #7
	bls r1, r2, .L86
.L85:
	moveq r1, #2
	rts
.L86:
	moveq r1, #1
	rts
.size w_ule_R_n,.-w_ule_R_n
.section .text,"ax"
.global w_ule_R_y
.type w_ule_R_y,@function
.align 16
w_ule_R_y:
	moveq r3, #7
.L88:
	sub.l r1, r1, #1
	bhi r1, r3, .L88
.L89:
	moveq r1, #1
	rts
.size w_ule_R_y,.-w_ule_R_y
.section .text,"ax"
.global w_ule_L_n
.type w_ule_L_n,@function
.align 16
w_ule_L_n:
	move.l r2, r1
	moveq r1, #7
	move.l r2, r2
	bls r1, r2, .L92
.L91:
	moveq r1, #2
	rts
.L92:
	moveq r1, #1
	rts
.size w_ule_L_n,.-w_ule_L_n
.section .text,"ax"
.global w_ule_L_y
.type w_ule_L_y,@function
.align 16
w_ule_L_y:
	moveq r3, #7
.L94:
	sub.l r1, r1, #1
	bhi r3, r1, .L94
.L95:
	moveq r1, #1
	rts
.size w_ule_L_y,.-w_ule_L_y
.section .text,"ax"
.global w_ugt_R_n
.type w_ugt_R_n,@function
.align 16
w_ugt_R_n:
	move.l r1, r1
	moveq r2, #7
	bhi r1, r2, .L98
.L97:
	moveq r1, #2
	rts
.L98:
	moveq r1, #1
	rts
.size w_ugt_R_n,.-w_ugt_R_n
.section .text,"ax"
.global w_ugt_R_y
.type w_ugt_R_y,@function
.align 16
w_ugt_R_y:
	moveq r3, #7
.L100:
	sub.l r1, r1, #1
	bls r1, r3, .L100
.L101:
	moveq r1, #1
	rts
.size w_ugt_R_y,.-w_ugt_R_y
.section .text,"ax"
.global w_ugt_L_n
.type w_ugt_L_n,@function
.align 16
w_ugt_L_n:
	move.l r2, r1
	moveq r1, #7
	move.l r2, r2
	bhi r1, r2, .L104
.L103:
	moveq r1, #2
	rts
.L104:
	moveq r1, #1
	rts
.size w_ugt_L_n,.-w_ugt_L_n
.section .text,"ax"
.global w_ugt_L_y
.type w_ugt_L_y,@function
.align 16
w_ugt_L_y:
	moveq r3, #7
.L106:
	sub.l r1, r1, #1
	bls r3, r1, .L106
.L107:
	moveq r1, #1
	rts
.size w_ugt_L_y,.-w_ugt_L_y
.section .text,"ax"
.global w_uge_R_n
.type w_uge_R_n,@function
.align 16
w_uge_R_n:
	move.l r1, r1
	moveq r2, #7
	bls r2, r1, .L110
.L109:
	moveq r1, #2
	rts
.L110:
	moveq r1, #1
	rts
.size w_uge_R_n,.-w_uge_R_n
.section .text,"ax"
.global w_uge_R_y
.type w_uge_R_y,@function
.align 16
w_uge_R_y:
	moveq r3, #7
.L112:
	sub.l r1, r1, #1
	bhi r3, r1, .L112
.L113:
	moveq r1, #1
	rts
.size w_uge_R_y,.-w_uge_R_y
.section .text,"ax"
.global w_uge_L_n
.type w_uge_L_n,@function
.align 16
w_uge_L_n:
	move.l r2, r1
	moveq r1, #7
	move.l r2, r2
	bls r2, r1, .L116
.L115:
	moveq r1, #2
	rts
.L116:
	moveq r1, #1
	rts
.size w_uge_L_n,.-w_uge_L_n
.section .text,"ax"
.global w_uge_L_y
.type w_uge_L_y,@function
.align 16
w_uge_L_y:
	moveq r3, #7
.L118:
	sub.l r1, r1, #1
	bhi r1, r3, .L118
.L119:
	moveq r1, #1
	rts
.size w_uge_L_y,.-w_uge_L_y
.section .text,"ax"
.global l_eq_R_n
.type l_eq_R_n,@function
.align 16
l_eq_R_n:
	moveq r2, #7
	beq r1, r2, .L122
.L121:
	moveq r1, #2
	rts
.L122:
	moveq r1, #1
	rts
.size l_eq_R_n,.-l_eq_R_n
.section .text,"ax"
.global l_eq_R_y
.type l_eq_R_y,@function
.align 16
l_eq_R_y:
	moveq r3, #7
.L124:
	sub.q r1, r1, #1
	bne r1, r3, .L124
.L125:
	moveq r1, #1
	rts
.size l_eq_R_y,.-l_eq_R_y
.section .text,"ax"
.global l_eq_L_n
.type l_eq_L_n,@function
.align 16
l_eq_L_n:
	moveq r2, #7
	beq r1, r2, .L128
.L127:
	moveq r1, #2
	rts
.L128:
	moveq r1, #1
	rts
.size l_eq_L_n,.-l_eq_L_n
.section .text,"ax"
.global l_eq_L_y
.type l_eq_L_y,@function
.align 16
l_eq_L_y:
	moveq r3, #7
.L130:
	sub.q r1, r1, #1
	bne r1, r3, .L130
.L131:
	moveq r1, #1
	rts
.size l_eq_L_y,.-l_eq_L_y
.section .text,"ax"
.global l_ne_R_n
.type l_ne_R_n,@function
.align 16
l_ne_R_n:
	moveq r2, #7
	bne r1, r2, .L134
.L133:
	moveq r1, #2
	rts
.L134:
	moveq r1, #1
	rts
.size l_ne_R_n,.-l_ne_R_n
.section .text,"ax"
.global l_ne_R_y
.type l_ne_R_y,@function
.align 16
l_ne_R_y:
	moveq r3, #7
.L136:
	sub.q r1, r1, #1
	beq r1, r3, .L136
.L137:
	moveq r1, #1
	rts
.size l_ne_R_y,.-l_ne_R_y
.section .text,"ax"
.global l_ne_L_n
.type l_ne_L_n,@function
.align 16
l_ne_L_n:
	moveq r2, #7
	bne r1, r2, .L140
.L139:
	moveq r1, #2
	rts
.L140:
	moveq r1, #1
	rts
.size l_ne_L_n,.-l_ne_L_n
.section .text,"ax"
.global l_ne_L_y
.type l_ne_L_y,@function
.align 16
l_ne_L_y:
	moveq r3, #7
.L142:
	sub.q r1, r1, #1
	beq r1, r3, .L142
.L143:
	moveq r1, #1
	rts
.size l_ne_L_y,.-l_ne_L_y
.section .text,"ax"
.global l_slt_R_n
.type l_slt_R_n,@function
.align 16
l_slt_R_n:
	moveq r2, #7
	blt r1, r2, .L146
.L145:
	moveq r1, #2
	rts
.L146:
	moveq r1, #1
	rts
.size l_slt_R_n,.-l_slt_R_n
.section .text,"ax"
.global l_slt_R_y
.type l_slt_R_y,@function
.align 16
l_slt_R_y:
	moveq r3, #7
.L148:
	sub.q r1, r1, #1
	bge r1, r3, .L148
.L149:
	moveq r1, #1
	rts
.size l_slt_R_y,.-l_slt_R_y
.section .text,"ax"
.global l_slt_L_n
.type l_slt_L_n,@function
.align 16
l_slt_L_n:
	moveq r2, #7
	blt r2, r1, .L152
.L151:
	moveq r1, #2
	rts
.L152:
	moveq r1, #1
	rts
.size l_slt_L_n,.-l_slt_L_n
.section .text,"ax"
.global l_slt_L_y
.type l_slt_L_y,@function
.align 16
l_slt_L_y:
	moveq r3, #7
.L154:
	sub.q r1, r1, #1
	bge r3, r1, .L154
.L155:
	moveq r1, #1
	rts
.size l_slt_L_y,.-l_slt_L_y
.section .text,"ax"
.global l_sle_R_n
.type l_sle_R_n,@function
.align 16
l_sle_R_n:
	moveq r2, #7
	ble r1, r2, .L158
.L157:
	moveq r1, #2
	rts
.L158:
	moveq r1, #1
	rts
.size l_sle_R_n,.-l_sle_R_n
.section .text,"ax"
.global l_sle_R_y
.type l_sle_R_y,@function
.align 16
l_sle_R_y:
	moveq r3, #7
.L160:
	sub.q r1, r1, #1
	bgt r1, r3, .L160
.L161:
	moveq r1, #1
	rts
.size l_sle_R_y,.-l_sle_R_y
.section .text,"ax"
.global l_sle_L_n
.type l_sle_L_n,@function
.align 16
l_sle_L_n:
	moveq r2, #7
	ble r2, r1, .L164
.L163:
	moveq r1, #2
	rts
.L164:
	moveq r1, #1
	rts
.size l_sle_L_n,.-l_sle_L_n
.section .text,"ax"
.global l_sle_L_y
.type l_sle_L_y,@function
.align 16
l_sle_L_y:
	moveq r3, #7
.L166:
	sub.q r1, r1, #1
	bgt r3, r1, .L166
.L167:
	moveq r1, #1
	rts
.size l_sle_L_y,.-l_sle_L_y
.section .text,"ax"
.global l_sgt_R_n
.type l_sgt_R_n,@function
.align 16
l_sgt_R_n:
	moveq r2, #7
	bgt r1, r2, .L170
.L169:
	moveq r1, #2
	rts
.L170:
	moveq r1, #1
	rts
.size l_sgt_R_n,.-l_sgt_R_n
.section .text,"ax"
.global l_sgt_R_y
.type l_sgt_R_y,@function
.align 16
l_sgt_R_y:
	moveq r3, #7
.L172:
	sub.q r1, r1, #1
	ble r1, r3, .L172
.L173:
	moveq r1, #1
	rts
.size l_sgt_R_y,.-l_sgt_R_y
.section .text,"ax"
.global l_sgt_L_n
.type l_sgt_L_n,@function
.align 16
l_sgt_L_n:
	moveq r2, #7
	bgt r2, r1, .L176
.L175:
	moveq r1, #2
	rts
.L176:
	moveq r1, #1
	rts
.size l_sgt_L_n,.-l_sgt_L_n
.section .text,"ax"
.global l_sgt_L_y
.type l_sgt_L_y,@function
.align 16
l_sgt_L_y:
	moveq r3, #7
.L178:
	sub.q r1, r1, #1
	ble r3, r1, .L178
.L179:
	moveq r1, #1
	rts
.size l_sgt_L_y,.-l_sgt_L_y
.section .text,"ax"
.global l_sge_R_n
.type l_sge_R_n,@function
.align 16
l_sge_R_n:
	moveq r2, #7
	bge r1, r2, .L182
.L181:
	moveq r1, #2
	rts
.L182:
	moveq r1, #1
	rts
.size l_sge_R_n,.-l_sge_R_n
.section .text,"ax"
.global l_sge_R_y
.type l_sge_R_y,@function
.align 16
l_sge_R_y:
	moveq r3, #7
.L184:
	sub.q r1, r1, #1
	blt r1, r3, .L184
.L185:
	moveq r1, #1
	rts
.size l_sge_R_y,.-l_sge_R_y
.section .text,"ax"
.global l_sge_L_n
.type l_sge_L_n,@function
.align 16
l_sge_L_n:
	moveq r2, #7
	bge r2, r1, .L188
.L187:
	moveq r1, #2
	rts
.L188:
	moveq r1, #1
	rts
.size l_sge_L_n,.-l_sge_L_n
.section .text,"ax"
.global l_sge_L_y
.type l_sge_L_y,@function
.align 16
l_sge_L_y:
	moveq r3, #7
.L190:
	sub.q r1, r1, #1
	blt r3, r1, .L190
.L191:
	moveq r1, #1
	rts
.size l_sge_L_y,.-l_sge_L_y
.section .text,"ax"
.global l_ult_R_n
.type l_ult_R_n,@function
.align 16
l_ult_R_n:
	moveq r2, #7
	bhi r2, r1, .L194
.L193:
	moveq r1, #2
	rts
.L194:
	moveq r1, #1
	rts
.size l_ult_R_n,.-l_ult_R_n
.section .text,"ax"
.global l_ult_R_y
.type l_ult_R_y,@function
.align 16
l_ult_R_y:
	moveq r3, #7
.L196:
	sub.q r1, r1, #1
	bls r3, r1, .L196
.L197:
	moveq r1, #1
	rts
.size l_ult_R_y,.-l_ult_R_y
.section .text,"ax"
.global l_ult_L_n
.type l_ult_L_n,@function
.align 16
l_ult_L_n:
	moveq r2, #7
	bhi r1, r2, .L200
.L199:
	moveq r1, #2
	rts
.L200:
	moveq r1, #1
	rts
.size l_ult_L_n,.-l_ult_L_n
.section .text,"ax"
.global l_ult_L_y
.type l_ult_L_y,@function
.align 16
l_ult_L_y:
	moveq r3, #7
.L202:
	sub.q r1, r1, #1
	bls r1, r3, .L202
.L203:
	moveq r1, #1
	rts
.size l_ult_L_y,.-l_ult_L_y
.section .text,"ax"
.global l_ule_R_n
.type l_ule_R_n,@function
.align 16
l_ule_R_n:
	moveq r2, #7
	bls r1, r2, .L206
.L205:
	moveq r1, #2
	rts
.L206:
	moveq r1, #1
	rts
.size l_ule_R_n,.-l_ule_R_n
.section .text,"ax"
.global l_ule_R_y
.type l_ule_R_y,@function
.align 16
l_ule_R_y:
	moveq r3, #7
.L208:
	sub.q r1, r1, #1
	bhi r1, r3, .L208
.L209:
	moveq r1, #1
	rts
.size l_ule_R_y,.-l_ule_R_y
.section .text,"ax"
.global l_ule_L_n
.type l_ule_L_n,@function
.align 16
l_ule_L_n:
	moveq r2, #7
	bls r2, r1, .L212
.L211:
	moveq r1, #2
	rts
.L212:
	moveq r1, #1
	rts
.size l_ule_L_n,.-l_ule_L_n
.section .text,"ax"
.global l_ule_L_y
.type l_ule_L_y,@function
.align 16
l_ule_L_y:
	moveq r3, #7
.L214:
	sub.q r1, r1, #1
	bhi r3, r1, .L214
.L215:
	moveq r1, #1
	rts
.size l_ule_L_y,.-l_ule_L_y
.section .text,"ax"
.global l_ugt_R_n
.type l_ugt_R_n,@function
.align 16
l_ugt_R_n:
	moveq r2, #7
	bhi r1, r2, .L218
.L217:
	moveq r1, #2
	rts
.L218:
	moveq r1, #1
	rts
.size l_ugt_R_n,.-l_ugt_R_n
.section .text,"ax"
.global l_ugt_R_y
.type l_ugt_R_y,@function
.align 16
l_ugt_R_y:
	moveq r3, #7
.L220:
	sub.q r1, r1, #1
	bls r1, r3, .L220
.L221:
	moveq r1, #1
	rts
.size l_ugt_R_y,.-l_ugt_R_y
.section .text,"ax"
.global l_ugt_L_n
.type l_ugt_L_n,@function
.align 16
l_ugt_L_n:
	moveq r2, #7
	bhi r2, r1, .L224
.L223:
	moveq r1, #2
	rts
.L224:
	moveq r1, #1
	rts
.size l_ugt_L_n,.-l_ugt_L_n
.section .text,"ax"
.global l_ugt_L_y
.type l_ugt_L_y,@function
.align 16
l_ugt_L_y:
	moveq r3, #7
.L226:
	sub.q r1, r1, #1
	bls r3, r1, .L226
.L227:
	moveq r1, #1
	rts
.size l_ugt_L_y,.-l_ugt_L_y
.section .text,"ax"
.global l_uge_R_n
.type l_uge_R_n,@function
.align 16
l_uge_R_n:
	moveq r2, #7
	bls r2, r1, .L230
.L229:
	moveq r1, #2
	rts
.L230:
	moveq r1, #1
	rts
.size l_uge_R_n,.-l_uge_R_n
.section .text,"ax"
.global l_uge_R_y
.type l_uge_R_y,@function
.align 16
l_uge_R_y:
	moveq r3, #7
.L232:
	sub.q r1, r1, #1
	bhi r3, r1, .L232
.L233:
	moveq r1, #1
	rts
.size l_uge_R_y,.-l_uge_R_y
.section .text,"ax"
.global l_uge_L_n
.type l_uge_L_n,@function
.align 16
l_uge_L_n:
	moveq r2, #7
	bls r1, r2, .L236
.L235:
	moveq r1, #2
	rts
.L236:
	moveq r1, #1
	rts
.size l_uge_L_n,.-l_uge_L_n
.section .text,"ax"
.global l_uge_L_y
.type l_uge_L_y,@function
.align 16
l_uge_L_y:
	moveq r3, #7
.L238:
	sub.q r1, r1, #1
	bhi r1, r3, .L238
.L239:
	moveq r1, #1
	rts
.size l_uge_L_y,.-l_uge_L_y
