align 16
make_double:
	move.q r30, #__ie64_fconst_0
	dload f0, (r30)
.L1:
	rts
align 16
call_with_live_doubles:
	sub.q r31, r31, #120
	fstore f8, 48(r31)
	fstore f9, 56(r31)
	fstore f10, 64(r31)
	fstore f11, 72(r31)
	fstore f12, 80(r31)
	fstore f13, 88(r31)
	fstore f14, 96(r31)
	fstore f15, 104(r31)
	move.q r30, #a+0
	dload f0, (r30)
	move.q r30, #b+0
	dload f2, (r30)
	move.q r30, #c+0
	dload f8, (r30)
	move.q r30, #d+0
	dload f14, (r30)
	move.q r30, #e+0
	dload f4, (r30)
	lea r30, 8(r31)
	dstore f4, (r30)
	dmov f12, f2
	move.q r30, #f+0
	dload f2, (r30)
	lea r30, 16(r31)
	dstore f2, (r30)
	move.q r30, #g+0
	dload f6, (r30)
	lea r30, 24(r31)
	dstore f6, (r30)
	dmov f10, f0
	move.q r30, #h+0
	dload f0, (r30)
	lea r30, 32(r31)
	dstore f0, (r30)
	jsr make_double
	dmov f2, f12
	dstore f0, 0(r31)
	dmov f0, f10
	dload f10, 32(r31)
	dload f6, 24(r31)
	dload f12, 16(r31)
	dload f4, 8(r31)
	dadd f2, f0, f2
	dload f0, 0(r31)
	dadd f8, f8, f14
	dadd f4, f4, f12
	dadd f6, f6, f10
	dadd f2, f2, f8
	dadd f4, f4, f6
	dadd f2, f2, f4
	dadd f0, f0, f2
.L3:
	fload f8, 48(r31)
	fload f9, 56(r31)
	fload f10, 64(r31)
	fload f11, 72(r31)
	fload f12, 80(r31)
	fload f13, 88(r31)
	fload f14, 96(r31)
	fload f15, 104(r31)
	add.q r31, r31, #120
	rts
align 8
a:
	dc.q 4607182418800017408
align 8
b:
	dc.q 4611686018427387904
align 8
c:
	dc.q 4613937818241073152
align 8
d:
	dc.q 4616189618054758400
align 8
e:
	dc.q 4617315517961601024
align 8
f:
	dc.q 4618441417868443648
align 8
g:
	dc.q 4619567317775286272
align 8
h:
	dc.q 4620693217682128896
align 8
__ie64_fconst_0:
	dc.q 0x3ff8000000000000
