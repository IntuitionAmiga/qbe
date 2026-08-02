align 16
f:
	fadd f0, f0, f1
.L1:
	rts
align 16
g:
	sub.q r31, r31, #8
	jsr f
.L3:
	add.q r31, r31, #8
	rts
