.section .text,"ax"
.global w_eq_R_32766
.type w_eq_R_32766,@function
.align 16
w_eq_R_32766:
	move.l r1, r1
	moveq r2, #32766
	beq r1, r2, .L2
.L1:
	moveq r1, #2
	rts
.L2:
	moveq r1, #1
	rts
.size w_eq_R_32766,.-w_eq_R_32766
.section .text,"ax"
.global w_eq_R_32767
.type w_eq_R_32767,@function
.align 16
w_eq_R_32767:
	move.l r1, r1
	moveq r2, #32767
	beq r1, r2, .L5
.L4:
	moveq r1, #2
	rts
.L5:
	moveq r1, #1
	rts
.size w_eq_R_32767,.-w_eq_R_32767
.section .text,"ax"
.global w_eq_R_32768
.type w_eq_R_32768,@function
.align 16
w_eq_R_32768:
	move.l r1, r1
	moveq r2, #32768
	beq r1, r2, .L8
.L7:
	moveq r1, #2
	rts
.L8:
	moveq r1, #1
	rts
.size w_eq_R_32768,.-w_eq_R_32768
.section .text,"ax"
.global w_eq_R_32769
.type w_eq_R_32769,@function
.align 16
w_eq_R_32769:
	move.l r1, r1
	moveq r2, #32769
	beq r1, r2, .L11
.L10:
	moveq r1, #2
	rts
.L11:
	moveq r1, #1
	rts
.size w_eq_R_32769,.-w_eq_R_32769
.section .text,"ax"
.global w_eq_R_m32767
.type w_eq_R_m32767,@function
.align 16
w_eq_R_m32767:
	move.l r1, r1
	move.l r2, #0xffff8001
	beq r1, r2, .L14
.L13:
	moveq r1, #2
	rts
.L14:
	moveq r1, #1
	rts
.size w_eq_R_m32767,.-w_eq_R_m32767
.section .text,"ax"
.global w_eq_R_m32768
.type w_eq_R_m32768,@function
.align 16
w_eq_R_m32768:
	move.l r1, r1
	move.l r2, #0xffff8000
	beq r1, r2, .L17
.L16:
	moveq r1, #2
	rts
.L17:
	moveq r1, #1
	rts
.size w_eq_R_m32768,.-w_eq_R_m32768
.section .text,"ax"
.global w_eq_R_m32769
.type w_eq_R_m32769,@function
.align 16
w_eq_R_m32769:
	move.l r1, r1
	move.l r2, #0xffff7fff
	beq r1, r2, .L20
.L19:
	moveq r1, #2
	rts
.L20:
	moveq r1, #1
	rts
.size w_eq_R_m32769,.-w_eq_R_m32769
.section .text,"ax"
.global w_eq_R_0
.type w_eq_R_0,@function
.align 16
w_eq_R_0:
	move.l r1, r1
	beq r1, r0, .L23
.L22:
	moveq r1, #2
	rts
.L23:
	moveq r1, #1
	rts
.size w_eq_R_0,.-w_eq_R_0
.section .text,"ax"
.global w_eq_R_1
.type w_eq_R_1,@function
.align 16
w_eq_R_1:
	move.l r1, r1
	moveq r2, #1
	beq r1, r2, .L26
.L25:
	moveq r1, #2
	rts
.L26:
	moveq r1, #1
	rts
.size w_eq_R_1,.-w_eq_R_1
.section .text,"ax"
.global w_eq_R_m1
.type w_eq_R_m1,@function
.align 16
w_eq_R_m1:
	move.l r1, r1
	move.l r2, #0xffffffff
	beq r1, r2, .L29
.L28:
	moveq r1, #2
	rts
.L29:
	moveq r1, #1
	rts
.size w_eq_R_m1,.-w_eq_R_m1
.section .text,"ax"
.global w_eq_R_4294967295
.type w_eq_R_4294967295,@function
.align 16
w_eq_R_4294967295:
	move.l r1, r1
	move.l r2, #0xffffffff
	beq r1, r2, .L32
.L31:
	moveq r1, #2
	rts
.L32:
	moveq r1, #1
	rts
.size w_eq_R_4294967295,.-w_eq_R_4294967295
.section .text,"ax"
.global w_eq_R_2147483648
.type w_eq_R_2147483648,@function
.align 16
w_eq_R_2147483648:
	move.l r1, r1
	move.l r2, #0x80000000
	beq r1, r2, .L35
.L34:
	moveq r1, #2
	rts
.L35:
	moveq r1, #1
	rts
.size w_eq_R_2147483648,.-w_eq_R_2147483648
.section .text,"ax"
.global w_slt_R_32766
.type w_slt_R_32766,@function
.align 16
w_slt_R_32766:
	sext.l r1, r1
	moveq r2, #32766
	blt r1, r2, .L38
.L37:
	moveq r1, #2
	rts
.L38:
	moveq r1, #1
	rts
.size w_slt_R_32766,.-w_slt_R_32766
.section .text,"ax"
.global w_slt_R_32767
.type w_slt_R_32767,@function
.align 16
w_slt_R_32767:
	sext.l r1, r1
	moveq r2, #32767
	blt r1, r2, .L41
.L40:
	moveq r1, #2
	rts
.L41:
	moveq r1, #1
	rts
.size w_slt_R_32767,.-w_slt_R_32767
.section .text,"ax"
.global w_slt_R_32768
.type w_slt_R_32768,@function
.align 16
w_slt_R_32768:
	sext.l r1, r1
	moveq r2, #32768
	blt r1, r2, .L44
.L43:
	moveq r1, #2
	rts
.L44:
	moveq r1, #1
	rts
.size w_slt_R_32768,.-w_slt_R_32768
.section .text,"ax"
.global w_slt_R_32769
.type w_slt_R_32769,@function
.align 16
w_slt_R_32769:
	sext.l r1, r1
	moveq r2, #32769
	blt r1, r2, .L47
.L46:
	moveq r1, #2
	rts
.L47:
	moveq r1, #1
	rts
.size w_slt_R_32769,.-w_slt_R_32769
.section .text,"ax"
.global w_slt_R_m32767
.type w_slt_R_m32767,@function
.align 16
w_slt_R_m32767:
	sext.l r1, r1
	moveq r2, #-32767
	blt r1, r2, .L50
.L49:
	moveq r1, #2
	rts
.L50:
	moveq r1, #1
	rts
.size w_slt_R_m32767,.-w_slt_R_m32767
.section .text,"ax"
.global w_slt_R_m32768
.type w_slt_R_m32768,@function
.align 16
w_slt_R_m32768:
	sext.l r1, r1
	moveq r2, #-32768
	blt r1, r2, .L53
.L52:
	moveq r1, #2
	rts
.L53:
	moveq r1, #1
	rts
.size w_slt_R_m32768,.-w_slt_R_m32768
.section .text,"ax"
.global w_slt_R_m32769
.type w_slt_R_m32769,@function
.align 16
w_slt_R_m32769:
	sext.l r1, r1
	moveq r2, #-32769
	blt r1, r2, .L56
.L55:
	moveq r1, #2
	rts
.L56:
	moveq r1, #1
	rts
.size w_slt_R_m32769,.-w_slt_R_m32769
.section .text,"ax"
.global w_slt_R_0
.type w_slt_R_0,@function
.align 16
w_slt_R_0:
	sext.l r1, r1
	blt r1, r0, .L59
.L58:
	moveq r1, #2
	rts
.L59:
	moveq r1, #1
	rts
.size w_slt_R_0,.-w_slt_R_0
.section .text,"ax"
.global w_slt_R_1
.type w_slt_R_1,@function
.align 16
w_slt_R_1:
	sext.l r1, r1
	moveq r2, #1
	blt r1, r2, .L62
.L61:
	moveq r1, #2
	rts
.L62:
	moveq r1, #1
	rts
.size w_slt_R_1,.-w_slt_R_1
.section .text,"ax"
.global w_slt_R_m1
.type w_slt_R_m1,@function
.align 16
w_slt_R_m1:
	sext.l r1, r1
	moveq r2, #-1
	blt r1, r2, .L65
.L64:
	moveq r1, #2
	rts
.L65:
	moveq r1, #1
	rts
.size w_slt_R_m1,.-w_slt_R_m1
.section .text,"ax"
.global w_slt_R_4294967295
.type w_slt_R_4294967295,@function
.align 16
w_slt_R_4294967295:
	sext.l r1, r1
	moveq r2, #-1
	blt r1, r2, .L68
.L67:
	moveq r1, #2
	rts
.L68:
	moveq r1, #1
	rts
.size w_slt_R_4294967295,.-w_slt_R_4294967295
.section .text,"ax"
.global w_slt_R_2147483648
.type w_slt_R_2147483648,@function
.align 16
w_slt_R_2147483648:
	sext.l r1, r1
	moveq r2, #-2147483648
	blt r1, r2, .L71
.L70:
	moveq r1, #2
	rts
.L71:
	moveq r1, #1
	rts
.size w_slt_R_2147483648,.-w_slt_R_2147483648
.section .text,"ax"
.global w_ult_R_32766
.type w_ult_R_32766,@function
.align 16
w_ult_R_32766:
	move.l r1, r1
	moveq r2, #32766
	bhi r2, r1, .L74
.L73:
	moveq r1, #2
	rts
.L74:
	moveq r1, #1
	rts
.size w_ult_R_32766,.-w_ult_R_32766
.section .text,"ax"
.global w_ult_R_32767
.type w_ult_R_32767,@function
.align 16
w_ult_R_32767:
	move.l r1, r1
	moveq r2, #32767
	bhi r2, r1, .L77
.L76:
	moveq r1, #2
	rts
.L77:
	moveq r1, #1
	rts
.size w_ult_R_32767,.-w_ult_R_32767
.section .text,"ax"
.global w_ult_R_32768
.type w_ult_R_32768,@function
.align 16
w_ult_R_32768:
	move.l r1, r1
	moveq r2, #32768
	bhi r2, r1, .L80
.L79:
	moveq r1, #2
	rts
.L80:
	moveq r1, #1
	rts
.size w_ult_R_32768,.-w_ult_R_32768
.section .text,"ax"
.global w_ult_R_32769
.type w_ult_R_32769,@function
.align 16
w_ult_R_32769:
	move.l r1, r1
	moveq r2, #32769
	bhi r2, r1, .L83
.L82:
	moveq r1, #2
	rts
.L83:
	moveq r1, #1
	rts
.size w_ult_R_32769,.-w_ult_R_32769
.section .text,"ax"
.global w_ult_R_m32767
.type w_ult_R_m32767,@function
.align 16
w_ult_R_m32767:
	move.l r1, r1
	move.l r2, #0xffff8001
	bhi r2, r1, .L86
.L85:
	moveq r1, #2
	rts
.L86:
	moveq r1, #1
	rts
.size w_ult_R_m32767,.-w_ult_R_m32767
.section .text,"ax"
.global w_ult_R_m32768
.type w_ult_R_m32768,@function
.align 16
w_ult_R_m32768:
	move.l r1, r1
	move.l r2, #0xffff8000
	bhi r2, r1, .L89
.L88:
	moveq r1, #2
	rts
.L89:
	moveq r1, #1
	rts
.size w_ult_R_m32768,.-w_ult_R_m32768
.section .text,"ax"
.global w_ult_R_m32769
.type w_ult_R_m32769,@function
.align 16
w_ult_R_m32769:
	move.l r1, r1
	move.l r2, #0xffff7fff
	bhi r2, r1, .L92
.L91:
	moveq r1, #2
	rts
.L92:
	moveq r1, #1
	rts
.size w_ult_R_m32769,.-w_ult_R_m32769
.section .text,"ax"
.global w_ult_R_0
.type w_ult_R_0,@function
.align 16
w_ult_R_0:
	move.l r1, r1
	bhi r0, r1, .L95
.L94:
	moveq r1, #2
	rts
.L95:
	moveq r1, #1
	rts
.size w_ult_R_0,.-w_ult_R_0
.section .text,"ax"
.global w_ult_R_1
.type w_ult_R_1,@function
.align 16
w_ult_R_1:
	move.l r1, r1
	moveq r2, #1
	bhi r2, r1, .L98
.L97:
	moveq r1, #2
	rts
.L98:
	moveq r1, #1
	rts
.size w_ult_R_1,.-w_ult_R_1
.section .text,"ax"
.global w_ult_R_m1
.type w_ult_R_m1,@function
.align 16
w_ult_R_m1:
	move.l r1, r1
	move.l r2, #0xffffffff
	bhi r2, r1, .L101
.L100:
	moveq r1, #2
	rts
.L101:
	moveq r1, #1
	rts
.size w_ult_R_m1,.-w_ult_R_m1
.section .text,"ax"
.global w_ult_R_4294967295
.type w_ult_R_4294967295,@function
.align 16
w_ult_R_4294967295:
	move.l r1, r1
	move.l r2, #0xffffffff
	bhi r2, r1, .L104
.L103:
	moveq r1, #2
	rts
.L104:
	moveq r1, #1
	rts
.size w_ult_R_4294967295,.-w_ult_R_4294967295
.section .text,"ax"
.global w_ult_R_2147483648
.type w_ult_R_2147483648,@function
.align 16
w_ult_R_2147483648:
	move.l r1, r1
	move.l r2, #0x80000000
	bhi r2, r1, .L107
.L106:
	moveq r1, #2
	rts
.L107:
	moveq r1, #1
	rts
.size w_ult_R_2147483648,.-w_ult_R_2147483648
.section .text,"ax"
.global w_uge_R_32766
.type w_uge_R_32766,@function
.align 16
w_uge_R_32766:
	move.l r1, r1
	moveq r2, #32766
	bls r2, r1, .L110
.L109:
	moveq r1, #2
	rts
.L110:
	moveq r1, #1
	rts
.size w_uge_R_32766,.-w_uge_R_32766
.section .text,"ax"
.global w_uge_R_32767
.type w_uge_R_32767,@function
.align 16
w_uge_R_32767:
	move.l r1, r1
	moveq r2, #32767
	bls r2, r1, .L113
.L112:
	moveq r1, #2
	rts
.L113:
	moveq r1, #1
	rts
.size w_uge_R_32767,.-w_uge_R_32767
.section .text,"ax"
.global w_uge_R_32768
.type w_uge_R_32768,@function
.align 16
w_uge_R_32768:
	move.l r1, r1
	moveq r2, #32768
	bls r2, r1, .L116
.L115:
	moveq r1, #2
	rts
.L116:
	moveq r1, #1
	rts
.size w_uge_R_32768,.-w_uge_R_32768
.section .text,"ax"
.global w_uge_R_32769
.type w_uge_R_32769,@function
.align 16
w_uge_R_32769:
	move.l r1, r1
	moveq r2, #32769
	bls r2, r1, .L119
.L118:
	moveq r1, #2
	rts
.L119:
	moveq r1, #1
	rts
.size w_uge_R_32769,.-w_uge_R_32769
.section .text,"ax"
.global w_uge_R_m32767
.type w_uge_R_m32767,@function
.align 16
w_uge_R_m32767:
	move.l r1, r1
	move.l r2, #0xffff8001
	bls r2, r1, .L122
.L121:
	moveq r1, #2
	rts
.L122:
	moveq r1, #1
	rts
.size w_uge_R_m32767,.-w_uge_R_m32767
.section .text,"ax"
.global w_uge_R_m32768
.type w_uge_R_m32768,@function
.align 16
w_uge_R_m32768:
	move.l r1, r1
	move.l r2, #0xffff8000
	bls r2, r1, .L125
.L124:
	moveq r1, #2
	rts
.L125:
	moveq r1, #1
	rts
.size w_uge_R_m32768,.-w_uge_R_m32768
.section .text,"ax"
.global w_uge_R_m32769
.type w_uge_R_m32769,@function
.align 16
w_uge_R_m32769:
	move.l r1, r1
	move.l r2, #0xffff7fff
	bls r2, r1, .L128
.L127:
	moveq r1, #2
	rts
.L128:
	moveq r1, #1
	rts
.size w_uge_R_m32769,.-w_uge_R_m32769
.section .text,"ax"
.global w_uge_R_0
.type w_uge_R_0,@function
.align 16
w_uge_R_0:
	move.l r1, r1
	bls r0, r1, .L131
.L130:
	moveq r1, #2
	rts
.L131:
	moveq r1, #1
	rts
.size w_uge_R_0,.-w_uge_R_0
.section .text,"ax"
.global w_uge_R_1
.type w_uge_R_1,@function
.align 16
w_uge_R_1:
	move.l r1, r1
	moveq r2, #1
	bls r2, r1, .L134
.L133:
	moveq r1, #2
	rts
.L134:
	moveq r1, #1
	rts
.size w_uge_R_1,.-w_uge_R_1
.section .text,"ax"
.global w_uge_R_m1
.type w_uge_R_m1,@function
.align 16
w_uge_R_m1:
	move.l r1, r1
	move.l r2, #0xffffffff
	bls r2, r1, .L137
.L136:
	moveq r1, #2
	rts
.L137:
	moveq r1, #1
	rts
.size w_uge_R_m1,.-w_uge_R_m1
.section .text,"ax"
.global w_uge_R_4294967295
.type w_uge_R_4294967295,@function
.align 16
w_uge_R_4294967295:
	move.l r1, r1
	move.l r2, #0xffffffff
	bls r2, r1, .L140
.L139:
	moveq r1, #2
	rts
.L140:
	moveq r1, #1
	rts
.size w_uge_R_4294967295,.-w_uge_R_4294967295
.section .text,"ax"
.global w_uge_R_2147483648
.type w_uge_R_2147483648,@function
.align 16
w_uge_R_2147483648:
	move.l r1, r1
	move.l r2, #0x80000000
	bls r2, r1, .L143
.L142:
	moveq r1, #2
	rts
.L143:
	moveq r1, #1
	rts
.size w_uge_R_2147483648,.-w_uge_R_2147483648
.section .text,"ax"
.global w_ugt_R_32766
.type w_ugt_R_32766,@function
.align 16
w_ugt_R_32766:
	move.l r1, r1
	moveq r2, #32766
	bhi r1, r2, .L146
.L145:
	moveq r1, #2
	rts
.L146:
	moveq r1, #1
	rts
.size w_ugt_R_32766,.-w_ugt_R_32766
.section .text,"ax"
.global w_ugt_R_32767
.type w_ugt_R_32767,@function
.align 16
w_ugt_R_32767:
	move.l r1, r1
	moveq r2, #32767
	bhi r1, r2, .L149
.L148:
	moveq r1, #2
	rts
.L149:
	moveq r1, #1
	rts
.size w_ugt_R_32767,.-w_ugt_R_32767
.section .text,"ax"
.global w_ugt_R_32768
.type w_ugt_R_32768,@function
.align 16
w_ugt_R_32768:
	move.l r1, r1
	moveq r2, #32768
	bhi r1, r2, .L152
.L151:
	moveq r1, #2
	rts
.L152:
	moveq r1, #1
	rts
.size w_ugt_R_32768,.-w_ugt_R_32768
.section .text,"ax"
.global w_ugt_R_32769
.type w_ugt_R_32769,@function
.align 16
w_ugt_R_32769:
	move.l r1, r1
	moveq r2, #32769
	bhi r1, r2, .L155
.L154:
	moveq r1, #2
	rts
.L155:
	moveq r1, #1
	rts
.size w_ugt_R_32769,.-w_ugt_R_32769
.section .text,"ax"
.global w_ugt_R_m32767
.type w_ugt_R_m32767,@function
.align 16
w_ugt_R_m32767:
	move.l r1, r1
	move.l r2, #0xffff8001
	bhi r1, r2, .L158
.L157:
	moveq r1, #2
	rts
.L158:
	moveq r1, #1
	rts
.size w_ugt_R_m32767,.-w_ugt_R_m32767
.section .text,"ax"
.global w_ugt_R_m32768
.type w_ugt_R_m32768,@function
.align 16
w_ugt_R_m32768:
	move.l r1, r1
	move.l r2, #0xffff8000
	bhi r1, r2, .L161
.L160:
	moveq r1, #2
	rts
.L161:
	moveq r1, #1
	rts
.size w_ugt_R_m32768,.-w_ugt_R_m32768
.section .text,"ax"
.global w_ugt_R_m32769
.type w_ugt_R_m32769,@function
.align 16
w_ugt_R_m32769:
	move.l r1, r1
	move.l r2, #0xffff7fff
	bhi r1, r2, .L164
.L163:
	moveq r1, #2
	rts
.L164:
	moveq r1, #1
	rts
.size w_ugt_R_m32769,.-w_ugt_R_m32769
.section .text,"ax"
.global w_ugt_R_0
.type w_ugt_R_0,@function
.align 16
w_ugt_R_0:
	move.l r1, r1
	bhi r1, r0, .L167
.L166:
	moveq r1, #2
	rts
.L167:
	moveq r1, #1
	rts
.size w_ugt_R_0,.-w_ugt_R_0
.section .text,"ax"
.global w_ugt_R_1
.type w_ugt_R_1,@function
.align 16
w_ugt_R_1:
	move.l r1, r1
	moveq r2, #1
	bhi r1, r2, .L170
.L169:
	moveq r1, #2
	rts
.L170:
	moveq r1, #1
	rts
.size w_ugt_R_1,.-w_ugt_R_1
.section .text,"ax"
.global w_ugt_R_m1
.type w_ugt_R_m1,@function
.align 16
w_ugt_R_m1:
	move.l r1, r1
	move.l r2, #0xffffffff
	bhi r1, r2, .L173
.L172:
	moveq r1, #2
	rts
.L173:
	moveq r1, #1
	rts
.size w_ugt_R_m1,.-w_ugt_R_m1
.section .text,"ax"
.global w_ugt_R_4294967295
.type w_ugt_R_4294967295,@function
.align 16
w_ugt_R_4294967295:
	move.l r1, r1
	move.l r2, #0xffffffff
	bhi r1, r2, .L176
.L175:
	moveq r1, #2
	rts
.L176:
	moveq r1, #1
	rts
.size w_ugt_R_4294967295,.-w_ugt_R_4294967295
.section .text,"ax"
.global w_ugt_R_2147483648
.type w_ugt_R_2147483648,@function
.align 16
w_ugt_R_2147483648:
	move.l r1, r1
	move.l r2, #0x80000000
	bhi r1, r2, .L179
.L178:
	moveq r1, #2
	rts
.L179:
	moveq r1, #1
	rts
.size w_ugt_R_2147483648,.-w_ugt_R_2147483648
.section .text,"ax"
.global w_uge_L_0
.type w_uge_L_0,@function
.align 16
w_uge_L_0:
.L181:
	sub.l r1, r1, #1
	bhi r1, r0, .L181
.L182:
	moveq r1, #1
	rts
.size w_uge_L_0,.-w_uge_L_0
.section .text,"ax"
.global w_uge_L_1
.type w_uge_L_1,@function
.align 16
w_uge_L_1:
	moveq r3, #1
.L184:
	sub.l r1, r1, #1
	bhi r1, r3, .L184
.L185:
	moveq r1, #1
	rts
.size w_uge_L_1,.-w_uge_L_1
.section .text,"ax"
.global w_uge_L_32768
.type w_uge_L_32768,@function
.align 16
w_uge_L_32768:
	moveq r3, #32768
.L187:
	sub.l r1, r1, #1
	bhi r1, r3, .L187
.L188:
	moveq r1, #1
	rts
.size w_uge_L_32768,.-w_uge_L_32768
.section .text,"ax"
.global w_uge_L_32769
.type w_uge_L_32769,@function
.align 16
w_uge_L_32769:
	moveq r3, #32769
.L190:
	sub.l r1, r1, #1
	bhi r1, r3, .L190
.L191:
	moveq r1, #1
	rts
.size w_uge_L_32769,.-w_uge_L_32769
.section .text,"ax"
.global w_uge_L_m1
.type w_uge_L_m1,@function
.align 16
w_uge_L_m1:
	move.l r3, #0xffffffff
.L193:
	sub.l r1, r1, #1
	bhi r1, r3, .L193
.L194:
	moveq r1, #1
	rts
.size w_uge_L_m1,.-w_uge_L_m1
.section .text,"ax"
.global w_uge_L_m32768
.type w_uge_L_m32768,@function
.align 16
w_uge_L_m32768:
	move.l r3, #0xffff8000
.L196:
	sub.l r1, r1, #1
	bhi r1, r3, .L196
.L197:
	moveq r1, #1
	rts
.size w_uge_L_m32768,.-w_uge_L_m32768
.section .text,"ax"
.global w_uge_L_m32769
.type w_uge_L_m32769,@function
.align 16
w_uge_L_m32769:
	move.l r3, #0xffff7fff
.L199:
	sub.l r1, r1, #1
	bhi r1, r3, .L199
.L200:
	moveq r1, #1
	rts
.size w_uge_L_m32769,.-w_uge_L_m32769
.section .text,"ax"
.global w_ult_L_0
.type w_ult_L_0,@function
.align 16
w_ult_L_0:
.L202:
	sub.l r1, r1, #1
	bls r1, r0, .L202
.L203:
	moveq r1, #1
	rts
.size w_ult_L_0,.-w_ult_L_0
.section .text,"ax"
.global w_ult_L_1
.type w_ult_L_1,@function
.align 16
w_ult_L_1:
	moveq r3, #1
.L205:
	sub.l r1, r1, #1
	bls r1, r3, .L205
.L206:
	moveq r1, #1
	rts
.size w_ult_L_1,.-w_ult_L_1
.section .text,"ax"
.global w_ult_L_32768
.type w_ult_L_32768,@function
.align 16
w_ult_L_32768:
	moveq r3, #32768
.L208:
	sub.l r1, r1, #1
	bls r1, r3, .L208
.L209:
	moveq r1, #1
	rts
.size w_ult_L_32768,.-w_ult_L_32768
.section .text,"ax"
.global w_ult_L_32769
.type w_ult_L_32769,@function
.align 16
w_ult_L_32769:
	moveq r3, #32769
.L211:
	sub.l r1, r1, #1
	bls r1, r3, .L211
.L212:
	moveq r1, #1
	rts
.size w_ult_L_32769,.-w_ult_L_32769
.section .text,"ax"
.global w_ult_L_m1
.type w_ult_L_m1,@function
.align 16
w_ult_L_m1:
	move.l r3, #0xffffffff
.L214:
	sub.l r1, r1, #1
	bls r1, r3, .L214
.L215:
	moveq r1, #1
	rts
.size w_ult_L_m1,.-w_ult_L_m1
.section .text,"ax"
.global w_ult_L_m32768
.type w_ult_L_m32768,@function
.align 16
w_ult_L_m32768:
	move.l r3, #0xffff8000
.L217:
	sub.l r1, r1, #1
	bls r1, r3, .L217
.L218:
	moveq r1, #1
	rts
.size w_ult_L_m32768,.-w_ult_L_m32768
.section .text,"ax"
.global w_ult_L_m32769
.type w_ult_L_m32769,@function
.align 16
w_ult_L_m32769:
	move.l r3, #0xffff7fff
.L220:
	sub.l r1, r1, #1
	bls r1, r3, .L220
.L221:
	moveq r1, #1
	rts
.size w_ult_L_m32769,.-w_ult_L_m32769
.section .text,"ax"
.global w_sgt_L_0
.type w_sgt_L_0,@function
.align 16
w_sgt_L_0:
.L223:
	sub.l r1, r1, #1
	sext.l r2, r1
	ble r0, r2, .L223
.L224:
	moveq r1, #1
	rts
.size w_sgt_L_0,.-w_sgt_L_0
.section .text,"ax"
.global w_sgt_L_1
.type w_sgt_L_1,@function
.align 16
w_sgt_L_1:
	moveq r4, #1
.L226:
	sub.l r1, r1, #1
	sext.l r3, r1
	ble r4, r3, .L226
.L227:
	moveq r1, #1
	rts
.size w_sgt_L_1,.-w_sgt_L_1
.section .text,"ax"
.global w_sgt_L_32768
.type w_sgt_L_32768,@function
.align 16
w_sgt_L_32768:
	moveq r4, #32768
.L229:
	sub.l r1, r1, #1
	sext.l r3, r1
	ble r4, r3, .L229
.L230:
	moveq r1, #1
	rts
.size w_sgt_L_32768,.-w_sgt_L_32768
.section .text,"ax"
.global w_sgt_L_32769
.type w_sgt_L_32769,@function
.align 16
w_sgt_L_32769:
	moveq r4, #32769
.L232:
	sub.l r1, r1, #1
	sext.l r3, r1
	ble r4, r3, .L232
.L233:
	moveq r1, #1
	rts
.size w_sgt_L_32769,.-w_sgt_L_32769
.section .text,"ax"
.global w_sgt_L_m1
.type w_sgt_L_m1,@function
.align 16
w_sgt_L_m1:
	moveq r4, #-1
.L235:
	sub.l r1, r1, #1
	sext.l r3, r1
	ble r4, r3, .L235
.L236:
	moveq r1, #1
	rts
.size w_sgt_L_m1,.-w_sgt_L_m1
.section .text,"ax"
.global w_sgt_L_m32768
.type w_sgt_L_m32768,@function
.align 16
w_sgt_L_m32768:
	moveq r4, #-32768
.L238:
	sub.l r1, r1, #1
	sext.l r3, r1
	ble r4, r3, .L238
.L239:
	moveq r1, #1
	rts
.size w_sgt_L_m32768,.-w_sgt_L_m32768
.section .text,"ax"
.global w_sgt_L_m32769
.type w_sgt_L_m32769,@function
.align 16
w_sgt_L_m32769:
	moveq r4, #-32769
.L241:
	sub.l r1, r1, #1
	sext.l r3, r1
	ble r4, r3, .L241
.L242:
	moveq r1, #1
	rts
.size w_sgt_L_m32769,.-w_sgt_L_m32769
.section .text,"ax"
.global l_eq_R_32766
.type l_eq_R_32766,@function
.align 16
l_eq_R_32766:
	moveq r2, #32766
	beq r1, r2, .L245
.L244:
	moveq r1, #2
	rts
.L245:
	moveq r1, #1
	rts
.size l_eq_R_32766,.-l_eq_R_32766
.section .text,"ax"
.global l_eq_R_32767
.type l_eq_R_32767,@function
.align 16
l_eq_R_32767:
	moveq r2, #32767
	beq r1, r2, .L248
.L247:
	moveq r1, #2
	rts
.L248:
	moveq r1, #1
	rts
.size l_eq_R_32767,.-l_eq_R_32767
.section .text,"ax"
.global l_eq_R_32768
.type l_eq_R_32768,@function
.align 16
l_eq_R_32768:
	moveq r2, #32768
	beq r1, r2, .L251
.L250:
	moveq r1, #2
	rts
.L251:
	moveq r1, #1
	rts
.size l_eq_R_32768,.-l_eq_R_32768
.section .text,"ax"
.global l_eq_R_32769
.type l_eq_R_32769,@function
.align 16
l_eq_R_32769:
	moveq r2, #32769
	beq r1, r2, .L254
.L253:
	moveq r1, #2
	rts
.L254:
	moveq r1, #1
	rts
.size l_eq_R_32769,.-l_eq_R_32769
.section .text,"ax"
.global l_eq_R_m32767
.type l_eq_R_m32767,@function
.align 16
l_eq_R_m32767:
	moveq r2, #-32767
	beq r1, r2, .L257
.L256:
	moveq r1, #2
	rts
.L257:
	moveq r1, #1
	rts
.size l_eq_R_m32767,.-l_eq_R_m32767
.section .text,"ax"
.global l_eq_R_m32768
.type l_eq_R_m32768,@function
.align 16
l_eq_R_m32768:
	moveq r2, #-32768
	beq r1, r2, .L260
.L259:
	moveq r1, #2
	rts
.L260:
	moveq r1, #1
	rts
.size l_eq_R_m32768,.-l_eq_R_m32768
.section .text,"ax"
.global l_eq_R_m32769
.type l_eq_R_m32769,@function
.align 16
l_eq_R_m32769:
	moveq r2, #-32769
	beq r1, r2, .L263
.L262:
	moveq r1, #2
	rts
.L263:
	moveq r1, #1
	rts
.size l_eq_R_m32769,.-l_eq_R_m32769
.section .text,"ax"
.global l_eq_R_0
.type l_eq_R_0,@function
.align 16
l_eq_R_0:
	beq r1, r0, .L266
.L265:
	moveq r1, #2
	rts
.L266:
	moveq r1, #1
	rts
.size l_eq_R_0,.-l_eq_R_0
.section .text,"ax"
.global l_eq_R_1
.type l_eq_R_1,@function
.align 16
l_eq_R_1:
	moveq r2, #1
	beq r1, r2, .L269
.L268:
	moveq r1, #2
	rts
.L269:
	moveq r1, #1
	rts
.size l_eq_R_1,.-l_eq_R_1
.section .text,"ax"
.global l_eq_R_m1
.type l_eq_R_m1,@function
.align 16
l_eq_R_m1:
	moveq r2, #-1
	beq r1, r2, .L272
.L271:
	moveq r1, #2
	rts
.L272:
	moveq r1, #1
	rts
.size l_eq_R_m1,.-l_eq_R_m1
.section .text,"ax"
.global l_eq_R_m2
.type l_eq_R_m2,@function
.align 16
l_eq_R_m2:
	moveq r2, #-2
	beq r1, r2, .L275
.L274:
	moveq r1, #2
	rts
.L275:
	moveq r1, #1
	rts
.size l_eq_R_m2,.-l_eq_R_m2
.section .text,"ax"
.global l_eq_R_m32770
.type l_eq_R_m32770,@function
.align 16
l_eq_R_m32770:
	moveq r2, #-32770
	beq r1, r2, .L278
.L277:
	moveq r1, #2
	rts
.L278:
	moveq r1, #1
	rts
.size l_eq_R_m32770,.-l_eq_R_m32770
.section .text,"ax"
.global l_eq_R_9223372036854775807
.type l_eq_R_9223372036854775807,@function
.align 16
l_eq_R_9223372036854775807:
	move.l r2, #0xffffffff
	movt r2, #0x7fffffff
	beq r1, r2, .L281
.L280:
	moveq r1, #2
	rts
.L281:
	moveq r1, #1
	rts
.size l_eq_R_9223372036854775807,.-l_eq_R_9223372036854775807
.section .text,"ax"
.global l_slt_R_32766
.type l_slt_R_32766,@function
.align 16
l_slt_R_32766:
	moveq r2, #32766
	blt r1, r2, .L284
.L283:
	moveq r1, #2
	rts
.L284:
	moveq r1, #1
	rts
.size l_slt_R_32766,.-l_slt_R_32766
.section .text,"ax"
.global l_slt_R_32767
.type l_slt_R_32767,@function
.align 16
l_slt_R_32767:
	moveq r2, #32767
	blt r1, r2, .L287
.L286:
	moveq r1, #2
	rts
.L287:
	moveq r1, #1
	rts
.size l_slt_R_32767,.-l_slt_R_32767
.section .text,"ax"
.global l_slt_R_32768
.type l_slt_R_32768,@function
.align 16
l_slt_R_32768:
	moveq r2, #32768
	blt r1, r2, .L290
.L289:
	moveq r1, #2
	rts
.L290:
	moveq r1, #1
	rts
.size l_slt_R_32768,.-l_slt_R_32768
.section .text,"ax"
.global l_slt_R_32769
.type l_slt_R_32769,@function
.align 16
l_slt_R_32769:
	moveq r2, #32769
	blt r1, r2, .L293
.L292:
	moveq r1, #2
	rts
.L293:
	moveq r1, #1
	rts
.size l_slt_R_32769,.-l_slt_R_32769
.section .text,"ax"
.global l_slt_R_m32767
.type l_slt_R_m32767,@function
.align 16
l_slt_R_m32767:
	moveq r2, #-32767
	blt r1, r2, .L296
.L295:
	moveq r1, #2
	rts
.L296:
	moveq r1, #1
	rts
.size l_slt_R_m32767,.-l_slt_R_m32767
.section .text,"ax"
.global l_slt_R_m32768
.type l_slt_R_m32768,@function
.align 16
l_slt_R_m32768:
	moveq r2, #-32768
	blt r1, r2, .L299
.L298:
	moveq r1, #2
	rts
.L299:
	moveq r1, #1
	rts
.size l_slt_R_m32768,.-l_slt_R_m32768
.section .text,"ax"
.global l_slt_R_m32769
.type l_slt_R_m32769,@function
.align 16
l_slt_R_m32769:
	moveq r2, #-32769
	blt r1, r2, .L302
.L301:
	moveq r1, #2
	rts
.L302:
	moveq r1, #1
	rts
.size l_slt_R_m32769,.-l_slt_R_m32769
.section .text,"ax"
.global l_slt_R_0
.type l_slt_R_0,@function
.align 16
l_slt_R_0:
	blt r1, r0, .L305
.L304:
	moveq r1, #2
	rts
.L305:
	moveq r1, #1
	rts
.size l_slt_R_0,.-l_slt_R_0
.section .text,"ax"
.global l_slt_R_1
.type l_slt_R_1,@function
.align 16
l_slt_R_1:
	moveq r2, #1
	blt r1, r2, .L308
.L307:
	moveq r1, #2
	rts
.L308:
	moveq r1, #1
	rts
.size l_slt_R_1,.-l_slt_R_1
.section .text,"ax"
.global l_slt_R_m1
.type l_slt_R_m1,@function
.align 16
l_slt_R_m1:
	moveq r2, #-1
	blt r1, r2, .L311
.L310:
	moveq r1, #2
	rts
.L311:
	moveq r1, #1
	rts
.size l_slt_R_m1,.-l_slt_R_m1
.section .text,"ax"
.global l_slt_R_m2
.type l_slt_R_m2,@function
.align 16
l_slt_R_m2:
	moveq r2, #-2
	blt r1, r2, .L314
.L313:
	moveq r1, #2
	rts
.L314:
	moveq r1, #1
	rts
.size l_slt_R_m2,.-l_slt_R_m2
.section .text,"ax"
.global l_slt_R_m32770
.type l_slt_R_m32770,@function
.align 16
l_slt_R_m32770:
	moveq r2, #-32770
	blt r1, r2, .L317
.L316:
	moveq r1, #2
	rts
.L317:
	moveq r1, #1
	rts
.size l_slt_R_m32770,.-l_slt_R_m32770
.section .text,"ax"
.global l_slt_R_9223372036854775807
.type l_slt_R_9223372036854775807,@function
.align 16
l_slt_R_9223372036854775807:
	move.l r2, #0xffffffff
	movt r2, #0x7fffffff
	blt r1, r2, .L320
.L319:
	moveq r1, #2
	rts
.L320:
	moveq r1, #1
	rts
.size l_slt_R_9223372036854775807,.-l_slt_R_9223372036854775807
.section .text,"ax"
.global l_ult_R_32766
.type l_ult_R_32766,@function
.align 16
l_ult_R_32766:
	moveq r2, #32766
	bhi r2, r1, .L323
.L322:
	moveq r1, #2
	rts
.L323:
	moveq r1, #1
	rts
.size l_ult_R_32766,.-l_ult_R_32766
.section .text,"ax"
.global l_ult_R_32767
.type l_ult_R_32767,@function
.align 16
l_ult_R_32767:
	moveq r2, #32767
	bhi r2, r1, .L326
.L325:
	moveq r1, #2
	rts
.L326:
	moveq r1, #1
	rts
.size l_ult_R_32767,.-l_ult_R_32767
.section .text,"ax"
.global l_ult_R_32768
.type l_ult_R_32768,@function
.align 16
l_ult_R_32768:
	moveq r2, #32768
	bhi r2, r1, .L329
.L328:
	moveq r1, #2
	rts
.L329:
	moveq r1, #1
	rts
.size l_ult_R_32768,.-l_ult_R_32768
.section .text,"ax"
.global l_ult_R_32769
.type l_ult_R_32769,@function
.align 16
l_ult_R_32769:
	moveq r2, #32769
	bhi r2, r1, .L332
.L331:
	moveq r1, #2
	rts
.L332:
	moveq r1, #1
	rts
.size l_ult_R_32769,.-l_ult_R_32769
.section .text,"ax"
.global l_ult_R_m32767
.type l_ult_R_m32767,@function
.align 16
l_ult_R_m32767:
	moveq r2, #-32767
	bhi r2, r1, .L335
.L334:
	moveq r1, #2
	rts
.L335:
	moveq r1, #1
	rts
.size l_ult_R_m32767,.-l_ult_R_m32767
.section .text,"ax"
.global l_ult_R_m32768
.type l_ult_R_m32768,@function
.align 16
l_ult_R_m32768:
	moveq r2, #-32768
	bhi r2, r1, .L338
.L337:
	moveq r1, #2
	rts
.L338:
	moveq r1, #1
	rts
.size l_ult_R_m32768,.-l_ult_R_m32768
.section .text,"ax"
.global l_ult_R_m32769
.type l_ult_R_m32769,@function
.align 16
l_ult_R_m32769:
	moveq r2, #-32769
	bhi r2, r1, .L341
.L340:
	moveq r1, #2
	rts
.L341:
	moveq r1, #1
	rts
.size l_ult_R_m32769,.-l_ult_R_m32769
.section .text,"ax"
.global l_ult_R_0
.type l_ult_R_0,@function
.align 16
l_ult_R_0:
	bhi r0, r1, .L344
.L343:
	moveq r1, #2
	rts
.L344:
	moveq r1, #1
	rts
.size l_ult_R_0,.-l_ult_R_0
.section .text,"ax"
.global l_ult_R_1
.type l_ult_R_1,@function
.align 16
l_ult_R_1:
	moveq r2, #1
	bhi r2, r1, .L347
.L346:
	moveq r1, #2
	rts
.L347:
	moveq r1, #1
	rts
.size l_ult_R_1,.-l_ult_R_1
.section .text,"ax"
.global l_ult_R_m1
.type l_ult_R_m1,@function
.align 16
l_ult_R_m1:
	moveq r2, #-1
	bhi r2, r1, .L350
.L349:
	moveq r1, #2
	rts
.L350:
	moveq r1, #1
	rts
.size l_ult_R_m1,.-l_ult_R_m1
.section .text,"ax"
.global l_ult_R_m2
.type l_ult_R_m2,@function
.align 16
l_ult_R_m2:
	moveq r2, #-2
	bhi r2, r1, .L353
.L352:
	moveq r1, #2
	rts
.L353:
	moveq r1, #1
	rts
.size l_ult_R_m2,.-l_ult_R_m2
.section .text,"ax"
.global l_ult_R_m32770
.type l_ult_R_m32770,@function
.align 16
l_ult_R_m32770:
	moveq r2, #-32770
	bhi r2, r1, .L356
.L355:
	moveq r1, #2
	rts
.L356:
	moveq r1, #1
	rts
.size l_ult_R_m32770,.-l_ult_R_m32770
.section .text,"ax"
.global l_ult_R_9223372036854775807
.type l_ult_R_9223372036854775807,@function
.align 16
l_ult_R_9223372036854775807:
	move.l r2, #0xffffffff
	movt r2, #0x7fffffff
	bhi r2, r1, .L359
.L358:
	moveq r1, #2
	rts
.L359:
	moveq r1, #1
	rts
.size l_ult_R_9223372036854775807,.-l_ult_R_9223372036854775807
.section .text,"ax"
.global l_uge_R_32766
.type l_uge_R_32766,@function
.align 16
l_uge_R_32766:
	moveq r2, #32766
	bls r2, r1, .L362
.L361:
	moveq r1, #2
	rts
.L362:
	moveq r1, #1
	rts
.size l_uge_R_32766,.-l_uge_R_32766
.section .text,"ax"
.global l_uge_R_32767
.type l_uge_R_32767,@function
.align 16
l_uge_R_32767:
	moveq r2, #32767
	bls r2, r1, .L365
.L364:
	moveq r1, #2
	rts
.L365:
	moveq r1, #1
	rts
.size l_uge_R_32767,.-l_uge_R_32767
.section .text,"ax"
.global l_uge_R_32768
.type l_uge_R_32768,@function
.align 16
l_uge_R_32768:
	moveq r2, #32768
	bls r2, r1, .L368
.L367:
	moveq r1, #2
	rts
.L368:
	moveq r1, #1
	rts
.size l_uge_R_32768,.-l_uge_R_32768
.section .text,"ax"
.global l_uge_R_32769
.type l_uge_R_32769,@function
.align 16
l_uge_R_32769:
	moveq r2, #32769
	bls r2, r1, .L371
.L370:
	moveq r1, #2
	rts
.L371:
	moveq r1, #1
	rts
.size l_uge_R_32769,.-l_uge_R_32769
.section .text,"ax"
.global l_uge_R_m32767
.type l_uge_R_m32767,@function
.align 16
l_uge_R_m32767:
	moveq r2, #-32767
	bls r2, r1, .L374
.L373:
	moveq r1, #2
	rts
.L374:
	moveq r1, #1
	rts
.size l_uge_R_m32767,.-l_uge_R_m32767
.section .text,"ax"
.global l_uge_R_m32768
.type l_uge_R_m32768,@function
.align 16
l_uge_R_m32768:
	moveq r2, #-32768
	bls r2, r1, .L377
.L376:
	moveq r1, #2
	rts
.L377:
	moveq r1, #1
	rts
.size l_uge_R_m32768,.-l_uge_R_m32768
.section .text,"ax"
.global l_uge_R_m32769
.type l_uge_R_m32769,@function
.align 16
l_uge_R_m32769:
	moveq r2, #-32769
	bls r2, r1, .L380
.L379:
	moveq r1, #2
	rts
.L380:
	moveq r1, #1
	rts
.size l_uge_R_m32769,.-l_uge_R_m32769
.section .text,"ax"
.global l_uge_R_0
.type l_uge_R_0,@function
.align 16
l_uge_R_0:
	bls r0, r1, .L383
.L382:
	moveq r1, #2
	rts
.L383:
	moveq r1, #1
	rts
.size l_uge_R_0,.-l_uge_R_0
.section .text,"ax"
.global l_uge_R_1
.type l_uge_R_1,@function
.align 16
l_uge_R_1:
	moveq r2, #1
	bls r2, r1, .L386
.L385:
	moveq r1, #2
	rts
.L386:
	moveq r1, #1
	rts
.size l_uge_R_1,.-l_uge_R_1
.section .text,"ax"
.global l_uge_R_m1
.type l_uge_R_m1,@function
.align 16
l_uge_R_m1:
	moveq r2, #-1
	bls r2, r1, .L389
.L388:
	moveq r1, #2
	rts
.L389:
	moveq r1, #1
	rts
.size l_uge_R_m1,.-l_uge_R_m1
.section .text,"ax"
.global l_uge_R_m2
.type l_uge_R_m2,@function
.align 16
l_uge_R_m2:
	moveq r2, #-2
	bls r2, r1, .L392
.L391:
	moveq r1, #2
	rts
.L392:
	moveq r1, #1
	rts
.size l_uge_R_m2,.-l_uge_R_m2
.section .text,"ax"
.global l_uge_R_m32770
.type l_uge_R_m32770,@function
.align 16
l_uge_R_m32770:
	moveq r2, #-32770
	bls r2, r1, .L395
.L394:
	moveq r1, #2
	rts
.L395:
	moveq r1, #1
	rts
.size l_uge_R_m32770,.-l_uge_R_m32770
.section .text,"ax"
.global l_uge_R_9223372036854775807
.type l_uge_R_9223372036854775807,@function
.align 16
l_uge_R_9223372036854775807:
	move.l r2, #0xffffffff
	movt r2, #0x7fffffff
	bls r2, r1, .L398
.L397:
	moveq r1, #2
	rts
.L398:
	moveq r1, #1
	rts
.size l_uge_R_9223372036854775807,.-l_uge_R_9223372036854775807
.section .text,"ax"
.global l_ugt_R_32766
.type l_ugt_R_32766,@function
.align 16
l_ugt_R_32766:
	moveq r2, #32766
	bhi r1, r2, .L401
.L400:
	moveq r1, #2
	rts
.L401:
	moveq r1, #1
	rts
.size l_ugt_R_32766,.-l_ugt_R_32766
.section .text,"ax"
.global l_ugt_R_32767
.type l_ugt_R_32767,@function
.align 16
l_ugt_R_32767:
	moveq r2, #32767
	bhi r1, r2, .L404
.L403:
	moveq r1, #2
	rts
.L404:
	moveq r1, #1
	rts
.size l_ugt_R_32767,.-l_ugt_R_32767
.section .text,"ax"
.global l_ugt_R_32768
.type l_ugt_R_32768,@function
.align 16
l_ugt_R_32768:
	moveq r2, #32768
	bhi r1, r2, .L407
.L406:
	moveq r1, #2
	rts
.L407:
	moveq r1, #1
	rts
.size l_ugt_R_32768,.-l_ugt_R_32768
.section .text,"ax"
.global l_ugt_R_32769
.type l_ugt_R_32769,@function
.align 16
l_ugt_R_32769:
	moveq r2, #32769
	bhi r1, r2, .L410
.L409:
	moveq r1, #2
	rts
.L410:
	moveq r1, #1
	rts
.size l_ugt_R_32769,.-l_ugt_R_32769
.section .text,"ax"
.global l_ugt_R_m32767
.type l_ugt_R_m32767,@function
.align 16
l_ugt_R_m32767:
	moveq r2, #-32767
	bhi r1, r2, .L413
.L412:
	moveq r1, #2
	rts
.L413:
	moveq r1, #1
	rts
.size l_ugt_R_m32767,.-l_ugt_R_m32767
.section .text,"ax"
.global l_ugt_R_m32768
.type l_ugt_R_m32768,@function
.align 16
l_ugt_R_m32768:
	moveq r2, #-32768
	bhi r1, r2, .L416
.L415:
	moveq r1, #2
	rts
.L416:
	moveq r1, #1
	rts
.size l_ugt_R_m32768,.-l_ugt_R_m32768
.section .text,"ax"
.global l_ugt_R_m32769
.type l_ugt_R_m32769,@function
.align 16
l_ugt_R_m32769:
	moveq r2, #-32769
	bhi r1, r2, .L419
.L418:
	moveq r1, #2
	rts
.L419:
	moveq r1, #1
	rts
.size l_ugt_R_m32769,.-l_ugt_R_m32769
.section .text,"ax"
.global l_ugt_R_0
.type l_ugt_R_0,@function
.align 16
l_ugt_R_0:
	bhi r1, r0, .L422
.L421:
	moveq r1, #2
	rts
.L422:
	moveq r1, #1
	rts
.size l_ugt_R_0,.-l_ugt_R_0
.section .text,"ax"
.global l_ugt_R_1
.type l_ugt_R_1,@function
.align 16
l_ugt_R_1:
	moveq r2, #1
	bhi r1, r2, .L425
.L424:
	moveq r1, #2
	rts
.L425:
	moveq r1, #1
	rts
.size l_ugt_R_1,.-l_ugt_R_1
.section .text,"ax"
.global l_ugt_R_m1
.type l_ugt_R_m1,@function
.align 16
l_ugt_R_m1:
	moveq r2, #-1
	bhi r1, r2, .L428
.L427:
	moveq r1, #2
	rts
.L428:
	moveq r1, #1
	rts
.size l_ugt_R_m1,.-l_ugt_R_m1
.section .text,"ax"
.global l_ugt_R_m2
.type l_ugt_R_m2,@function
.align 16
l_ugt_R_m2:
	moveq r2, #-2
	bhi r1, r2, .L431
.L430:
	moveq r1, #2
	rts
.L431:
	moveq r1, #1
	rts
.size l_ugt_R_m2,.-l_ugt_R_m2
.section .text,"ax"
.global l_ugt_R_m32770
.type l_ugt_R_m32770,@function
.align 16
l_ugt_R_m32770:
	moveq r2, #-32770
	bhi r1, r2, .L434
.L433:
	moveq r1, #2
	rts
.L434:
	moveq r1, #1
	rts
.size l_ugt_R_m32770,.-l_ugt_R_m32770
.section .text,"ax"
.global l_ugt_R_9223372036854775807
.type l_ugt_R_9223372036854775807,@function
.align 16
l_ugt_R_9223372036854775807:
	move.l r2, #0xffffffff
	movt r2, #0x7fffffff
	bhi r1, r2, .L437
.L436:
	moveq r1, #2
	rts
.L437:
	moveq r1, #1
	rts
.size l_ugt_R_9223372036854775807,.-l_ugt_R_9223372036854775807
.section .text,"ax"
.global l_uge_L_0
.type l_uge_L_0,@function
.align 16
l_uge_L_0:
.L439:
	sub.q r1, r1, #1
	bhi r1, r0, .L439
.L440:
	moveq r1, #1
	rts
.size l_uge_L_0,.-l_uge_L_0
.section .text,"ax"
.global l_uge_L_1
.type l_uge_L_1,@function
.align 16
l_uge_L_1:
	moveq r3, #1
.L442:
	sub.q r1, r1, #1
	bhi r1, r3, .L442
.L443:
	moveq r1, #1
	rts
.size l_uge_L_1,.-l_uge_L_1
.section .text,"ax"
.global l_uge_L_32768
.type l_uge_L_32768,@function
.align 16
l_uge_L_32768:
	moveq r3, #32768
.L445:
	sub.q r1, r1, #1
	bhi r1, r3, .L445
.L446:
	moveq r1, #1
	rts
.size l_uge_L_32768,.-l_uge_L_32768
.section .text,"ax"
.global l_uge_L_32769
.type l_uge_L_32769,@function
.align 16
l_uge_L_32769:
	moveq r3, #32769
.L448:
	sub.q r1, r1, #1
	bhi r1, r3, .L448
.L449:
	moveq r1, #1
	rts
.size l_uge_L_32769,.-l_uge_L_32769
.section .text,"ax"
.global l_uge_L_m1
.type l_uge_L_m1,@function
.align 16
l_uge_L_m1:
	moveq r3, #-1
.L451:
	sub.q r1, r1, #1
	bhi r1, r3, .L451
.L452:
	moveq r1, #1
	rts
.size l_uge_L_m1,.-l_uge_L_m1
.section .text,"ax"
.global l_uge_L_m32768
.type l_uge_L_m32768,@function
.align 16
l_uge_L_m32768:
	moveq r3, #-32768
.L454:
	sub.q r1, r1, #1
	bhi r1, r3, .L454
.L455:
	moveq r1, #1
	rts
.size l_uge_L_m32768,.-l_uge_L_m32768
.section .text,"ax"
.global l_uge_L_m32769
.type l_uge_L_m32769,@function
.align 16
l_uge_L_m32769:
	moveq r3, #-32769
.L457:
	sub.q r1, r1, #1
	bhi r1, r3, .L457
.L458:
	moveq r1, #1
	rts
.size l_uge_L_m32769,.-l_uge_L_m32769
.section .text,"ax"
.global l_ult_L_0
.type l_ult_L_0,@function
.align 16
l_ult_L_0:
.L460:
	sub.q r1, r1, #1
	bls r1, r0, .L460
.L461:
	moveq r1, #1
	rts
.size l_ult_L_0,.-l_ult_L_0
.section .text,"ax"
.global l_ult_L_1
.type l_ult_L_1,@function
.align 16
l_ult_L_1:
	moveq r3, #1
.L463:
	sub.q r1, r1, #1
	bls r1, r3, .L463
.L464:
	moveq r1, #1
	rts
.size l_ult_L_1,.-l_ult_L_1
.section .text,"ax"
.global l_ult_L_32768
.type l_ult_L_32768,@function
.align 16
l_ult_L_32768:
	moveq r3, #32768
.L466:
	sub.q r1, r1, #1
	bls r1, r3, .L466
.L467:
	moveq r1, #1
	rts
.size l_ult_L_32768,.-l_ult_L_32768
.section .text,"ax"
.global l_ult_L_32769
.type l_ult_L_32769,@function
.align 16
l_ult_L_32769:
	moveq r3, #32769
.L469:
	sub.q r1, r1, #1
	bls r1, r3, .L469
.L470:
	moveq r1, #1
	rts
.size l_ult_L_32769,.-l_ult_L_32769
.section .text,"ax"
.global l_ult_L_m1
.type l_ult_L_m1,@function
.align 16
l_ult_L_m1:
	moveq r3, #-1
.L472:
	sub.q r1, r1, #1
	bls r1, r3, .L472
.L473:
	moveq r1, #1
	rts
.size l_ult_L_m1,.-l_ult_L_m1
.section .text,"ax"
.global l_ult_L_m32768
.type l_ult_L_m32768,@function
.align 16
l_ult_L_m32768:
	moveq r3, #-32768
.L475:
	sub.q r1, r1, #1
	bls r1, r3, .L475
.L476:
	moveq r1, #1
	rts
.size l_ult_L_m32768,.-l_ult_L_m32768
.section .text,"ax"
.global l_ult_L_m32769
.type l_ult_L_m32769,@function
.align 16
l_ult_L_m32769:
	moveq r3, #-32769
.L478:
	sub.q r1, r1, #1
	bls r1, r3, .L478
.L479:
	moveq r1, #1
	rts
.size l_ult_L_m32769,.-l_ult_L_m32769
.section .text,"ax"
.global l_sgt_L_0
.type l_sgt_L_0,@function
.align 16
l_sgt_L_0:
.L481:
	sub.q r1, r1, #1
	ble r0, r1, .L481
.L482:
	moveq r1, #1
	rts
.size l_sgt_L_0,.-l_sgt_L_0
.section .text,"ax"
.global l_sgt_L_1
.type l_sgt_L_1,@function
.align 16
l_sgt_L_1:
	moveq r3, #1
.L484:
	sub.q r1, r1, #1
	ble r3, r1, .L484
.L485:
	moveq r1, #1
	rts
.size l_sgt_L_1,.-l_sgt_L_1
.section .text,"ax"
.global l_sgt_L_32768
.type l_sgt_L_32768,@function
.align 16
l_sgt_L_32768:
	moveq r3, #32768
.L487:
	sub.q r1, r1, #1
	ble r3, r1, .L487
.L488:
	moveq r1, #1
	rts
.size l_sgt_L_32768,.-l_sgt_L_32768
.section .text,"ax"
.global l_sgt_L_32769
.type l_sgt_L_32769,@function
.align 16
l_sgt_L_32769:
	moveq r3, #32769
.L490:
	sub.q r1, r1, #1
	ble r3, r1, .L490
.L491:
	moveq r1, #1
	rts
.size l_sgt_L_32769,.-l_sgt_L_32769
.section .text,"ax"
.global l_sgt_L_m1
.type l_sgt_L_m1,@function
.align 16
l_sgt_L_m1:
	moveq r3, #-1
.L493:
	sub.q r1, r1, #1
	ble r3, r1, .L493
.L494:
	moveq r1, #1
	rts
.size l_sgt_L_m1,.-l_sgt_L_m1
.section .text,"ax"
.global l_sgt_L_m32768
.type l_sgt_L_m32768,@function
.align 16
l_sgt_L_m32768:
	moveq r3, #-32768
.L496:
	sub.q r1, r1, #1
	ble r3, r1, .L496
.L497:
	moveq r1, #1
	rts
.size l_sgt_L_m32768,.-l_sgt_L_m32768
.section .text,"ax"
.global l_sgt_L_m32769
.type l_sgt_L_m32769,@function
.align 16
l_sgt_L_m32769:
	moveq r3, #-32769
.L499:
	sub.q r1, r1, #1
	ble r3, r1, .L499
.L500:
	moveq r1, #1
	rts
.size l_sgt_L_m32769,.-l_sgt_L_m32769
