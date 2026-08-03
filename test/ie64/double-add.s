.section .text,"ax"
.global add
.type add,@function
.align 16
add:
	dadd f0, f0, f2
.L1:
	rts
.size add,.-add
.section .text,"ax"
.global call
.type call,@function
.align 16
call:
	sub.q r31, r31, #8
	jsr add
.L3:
	add.q r31, r31, #8
	rts
.size call,.-call
