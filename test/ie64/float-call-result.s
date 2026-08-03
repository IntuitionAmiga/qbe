.section .text,"ax"
.global call_float
.type call_float,@function
.align 16
call_float:
	sub.q r31, r31, #8
	jsr returns_float
.L1:
	add.q r31, r31, #8
	rts
.size call_float,.-call_float
.section .text,"ax"
.global call_double
.type call_double,@function
.align 16
call_double:
	sub.q r31, r31, #8
	jsr returns_double
.L3:
	add.q r31, r31, #8
	rts
.size call_double,.-call_double
