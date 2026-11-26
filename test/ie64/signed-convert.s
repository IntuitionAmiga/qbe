.section .text,"ax"
.global double_to_long
.type double_to_long,@function
.align 16
double_to_long:
	dcvtfi r1, f0
	rts
.size double_to_long,.-double_to_long
.section .text,"ax"
.global long_to_double
.type long_to_double,@function
.align 16
long_to_double:
	dcvtif f0, r1
	rts
.size long_to_double,.-long_to_double
.section .text,"ax"
.global single_to_long
.type single_to_long,@function
.align 16
single_to_long:
	fcvtsd f0, f0
	dcvtfi r1, f0
	rts
.size single_to_long,.-single_to_long
.section .text,"ax"
.global long_to_single
.type long_to_single,@function
.align 16
long_to_single:
	asr.q r30, r1, #53
	add.q r30, r30, #1
	lsr.q r30, r30, #1
	bne r30, r0, .Lutoh0
	dcvtif f0, r1
	bra .Lutoe0
.Lutoh0:
	asr.q r30, r1, #11
	and.q r29, r1, #2047
	add.q r29, r29, #2047
	lsr.q r29, r29, #11
	or.q r30, r30, r29
	lsl.q r30, r30, #11
	dcvtif f0, r30
.Lutoe0:
	fcvtds f0, f0
	rts
.size long_to_single,.-long_to_single
.section .text,"ax"
.global double_to_long_to_single
.type double_to_long_to_single,@function
.align 16
double_to_long_to_single:
	dcvtfi r1, f0
	dcvtif f0, r1
	fcvtds f0, f0
	rts
.size double_to_long_to_single,.-double_to_long_to_single
