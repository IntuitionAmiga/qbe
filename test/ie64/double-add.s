.section .text,"ax"
.global add
.type add,@function
.align 16
add:
	dadd f0, f0, f2
	rts
.size add,.-add
.section .text,"ax"
.global call
.type call,@function
.align 16
call:
	bra add
.size call,.-call
