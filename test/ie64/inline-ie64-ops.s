.section .text,"ax"
.local clear
.type clear,@function
.align 16
clear:
	moveq r1, #0
	fmovsc r1
	rts
.size clear,.-clear
.section .text,"ax"
.local root
.type root,@function
.align 16
root:
	fsqrt f0, f0
	rts
.size root,.-root
.section .text,"ax"
.local status
.type status,@function
.align 16
status:
	fmovsr r1
	rts
.size status,.-status
.section .text,"ax"
.global flags
.type flags,@function
.align 16
flags:
	sub.q r31, r31, #24
	store.q r18, 0(r31)
	fstore f8, 8(r31)
	fmov f8, f0
	move.q r18, r1
	jsr clear
	fmov f0, f8
	move.q r1, r18
	fsqrt f0, f0
	fstore f0, (r1)
	jsr status
	load.q r18, 0(r31)
	fload f8, 8(r31)
	add.q r31, r31, #24
	rts
.size flags,.-flags
