.section ".fasttext","ax"
.global sectioned
.type sectioned,@function
.align 16
sectioned:
	li r1, #0
	sext.l r1, r1
	rts
.size sectioned,.-sectioned
