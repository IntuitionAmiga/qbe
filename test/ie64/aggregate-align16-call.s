.section .text,"ax"
.local take
.type take,@function
.align 16
take:
.L1:
	rts
.size take,.-take
.section .text,"ax"
.global call_aligned
.type call_aligned,@function
.align 16
call_aligned:
	sub.q r31, r31, #24
	lea r30, 0(r31)
	store.q r1, (r30)
	move.l r30, #lo32(value+0)
	movt r30, #hi32(value+0)
	load.q r1, (r30)
	lea r2, 0(r31)
	store.q r1, (r2)
	move.l r1, #lo32(value+0)
	movt r1, #hi32(value+0)
	add.q r1, r1, #8
	lea r2, 0(r31)
	add.q r2, r2, #8
	load.q r1, (r1)
	store.q r1, (r2)
	lea r1, 0(r31)
	jsr take
.L3:
	add.q r31, r31, #24
	rts
.size call_aligned,.-call_aligned
.section .data,"aw"
.local value
.type value,@object
.align 16
value:
	dc.q 1
	dc.q 2
.size value,16
