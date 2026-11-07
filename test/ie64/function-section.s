.section ".fasttext","ax"
.global sectioned
.type sectioned,@function
.align 16
sectioned:
	moveq r1, #0
	rts
.size sectioned,.-sectioned
