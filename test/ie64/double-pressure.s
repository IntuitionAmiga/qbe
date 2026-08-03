.section .text,"ax"
.global sum_nine_doubles
.type sum_nine_doubles,@function
.align 16
sum_nine_doubles:
	sub.q r31, r31, #88
	fstore f8, 16(r31)
	fstore f9, 24(r31)
	fstore f10, 32(r31)
	fstore f11, 40(r31)
	fstore f12, 48(r31)
	fstore f13, 56(r31)
	fstore f14, 64(r31)
	fstore f15, 72(r31)
	dmov f14, f4
	dmov f8, f6
	dload f4, 96(r31)
	dload f12, 104(r31)
	dload f6, 112(r31)
	dload f10, 120(r31)
	lea r30, 8(r31)
	dstore f10, (r30)
	dload f10, 128(r31)
	lea r30, 0(r31)
	dstore f10, (r30)
	dload f10, 8(r31)
	dadd f2, f0, f2
	dload f0, 0(r31)
	dadd f8, f14, f8
	dadd f4, f4, f12
	dadd f6, f6, f10
	dadd f2, f2, f8
	dadd f4, f4, f6
	dadd f2, f2, f4
	dadd f0, f0, f2
.L1:
	fload f8, 16(r31)
	fload f9, 24(r31)
	fload f10, 32(r31)
	fload f11, 40(r31)
	fload f12, 48(r31)
	fload f13, 56(r31)
	fload f14, 64(r31)
	fload f15, 72(r31)
	add.q r31, r31, #88
	rts
.size sum_nine_doubles,.-sum_nine_doubles
