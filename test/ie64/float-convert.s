.section .text,"ax"
.global toint
.type toint,@function
.align 16
toint:
	fcvtfi r1, f0
	sext.l r1, r1
	rts
.size toint,.-toint
.section .text,"ax"
.global tofloat
.type tofloat,@function
.align 16
tofloat:
	fcvtif f0, r1
	rts
.size tofloat,.-tofloat
