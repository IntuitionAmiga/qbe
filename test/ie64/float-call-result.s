.section .text,"ax"
.global call_float
.type call_float,@function
.align 16
call_float:
	bra returns_float
.size call_float,.-call_float
.section .text,"ax"
.global call_double
.type call_double,@function
.align 16
call_double:
	bra returns_double
.size call_double,.-call_double
