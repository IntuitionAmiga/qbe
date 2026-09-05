.section .text,"ax"
.local make_double
.type make_double,@function
.align 16
make_double:
	move.l r30, #lo32(.L__ie64_fconst_0+0)
	movt r30, #hi32(.L__ie64_fconst_0+0)
	dload f0, (r30)
.L1:
	rts
.size make_double,.-make_double
.section .text,"ax"
.global call_with_live_doubles
.type call_with_live_doubles,@function
.align 16
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
	move.l r30, #lo32(a+0)
	movt r30, #hi32(a+0)
	dload f0, (r30)
	move.l r30, #lo32(b+0)
	movt r30, #hi32(b+0)
	dload f2, (r30)
	move.l r30, #lo32(c+0)
	movt r30, #hi32(c+0)
	dload f8, (r30)
	move.l r30, #lo32(d+0)
	movt r30, #hi32(d+0)
	dload f14, (r30)
	move.l r30, #lo32(e+0)
	movt r30, #hi32(e+0)
	dload f4, (r30)
	lea r30, 8(r31)
	dstore f4, (r30)
	dmov f12, f2
	move.l r30, #lo32(f+0)
	movt r30, #hi32(f+0)
	dload f2, (r30)
	lea r30, 16(r31)
	dstore f2, (r30)
	move.l r30, #lo32(g+0)
	movt r30, #hi32(g+0)
	dload f6, (r30)
	lea r30, 24(r31)
	dstore f6, (r30)
	dmov f10, f0
	move.l r30, #lo32(h+0)
	movt r30, #hi32(h+0)
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
.size call_with_live_doubles,.-call_with_live_doubles
.section .data,"aw"
.local a
.type a,@object
.align 8
a:
	dc.q 4607182418800017408
.size a,8
.section .data,"aw"
.local b
.type b,@object
.align 8
b:
	dc.q 4611686018427387904
.size b,8
.section .data,"aw"
.local c
.type c,@object
.align 8
c:
	dc.q 4613937818241073152
.size c,8
.section .data,"aw"
.local d
.type d,@object
.align 8
d:
	dc.q 4616189618054758400
.size d,8
.section .data,"aw"
.local e
.type e,@object
.align 8
e:
	dc.q 4617315517961601024
.size e,8
.section .data,"aw"
.local f
.type f,@object
.align 8
f:
	dc.q 4618441417868443648
.size f,8
.section .data,"aw"
.local g
.type g,@object
.align 8
g:
	dc.q 4619567317775286272
.size g,8
.section .data,"aw"
.local h
.type h,@object
.align 8
h:
	dc.q 4620693217682128896
.size h,8
.section .rodata,"a"
.local .L__ie64_fconst_0
.align 8
.L__ie64_fconst_0:
	dc.q 0x3ff8000000000000
