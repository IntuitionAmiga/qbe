.section .text,"ax"
.global sar_narrow
.type sar_narrow,@function
.align 16
sar_narrow:
	and.q r1, r1, #0xff
	asr.l r1, r1, #1
	asr.l r1, r1, #2
	rts
.size sar_narrow,.-sar_narrow
