.section .text,"ax"
.global convert
.type convert,@function
.align 16
convert:
	and.q r1, r1, #0xffff
	rts
.size convert,.-convert
.section .text,"ax"
.global preserve
.type preserve,@function
.align 16
preserve:
	sext.b r1, r1
	and.q r1, r1, #0xffff
	rts
.size preserve,.-preserve
