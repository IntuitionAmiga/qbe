.section .text,"ax"
.global hidden_function
.hidden hidden_function
.type hidden_function,@function
.align 16
hidden_function:
	li r1, #7
	rts
.size hidden_function,.-hidden_function
.section .data,"aw"
.global hidden_object
.hidden hidden_object
.type hidden_object,@object
.align 8
hidden_object:
	dc.q 9
.size hidden_object,8
