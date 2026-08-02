align 16
add:
	dadd f0, f0, f2
.L1:
	rts
align 16
call:
	sub.q r31, r31, #8
	jsr add
.L3:
	add.q r31, r31, #8
	rts
