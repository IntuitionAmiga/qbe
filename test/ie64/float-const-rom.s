.section .text,"ax"
.global rom_pi
.type rom_pi,@function
.align 16
rom_pi:
	fmovecr f0, #0
	rts
.size rom_pi,.-rom_pi
.section .text,"ax"
.global rom_e
.type rom_e,@function
.align 16
rom_e:
	fmovecr f0, #1
	rts
.size rom_e,.-rom_e
.section .text,"ax"
.global rom_log2e
.type rom_log2e,@function
.align 16
rom_log2e:
	fmovecr f0, #2
	rts
.size rom_log2e,.-rom_log2e
.section .text,"ax"
.global rom_log10e
.type rom_log10e,@function
.align 16
rom_log10e:
	fmovecr f0, #3
	rts
.size rom_log10e,.-rom_log10e
.section .text,"ax"
.global rom_ln2
.type rom_ln2,@function
.align 16
rom_ln2:
	fmovecr f0, #4
	rts
.size rom_ln2,.-rom_ln2
.section .text,"ax"
.global rom_ln10
.type rom_ln10,@function
.align 16
rom_ln10:
	fmovecr f0, #5
	rts
.size rom_ln10,.-rom_ln10
.section .text,"ax"
.global rom_log10_2
.type rom_log10_2,@function
.align 16
rom_log10_2:
	fmovecr f0, #6
	rts
.size rom_log10_2,.-rom_log10_2
.section .text,"ax"
.global rom_zero
.type rom_zero,@function
.align 16
rom_zero:
	fmovecr f0, #7
	rts
.size rom_zero,.-rom_zero
.section .text,"ax"
.global rom_one
.type rom_one,@function
.align 16
rom_one:
	fmovecr f0, #8
	rts
.size rom_one,.-rom_one
.section .text,"ax"
.global rom_two
.type rom_two,@function
.align 16
rom_two:
	fmovecr f0, #9
	rts
.size rom_two,.-rom_two
.section .text,"ax"
.global rom_ten
.type rom_ten,@function
.align 16
rom_ten:
	fmovecr f0, #10
	rts
.size rom_ten,.-rom_ten
.section .text,"ax"
.global rom_hundred
.type rom_hundred,@function
.align 16
rom_hundred:
	fmovecr f0, #11
	rts
.size rom_hundred,.-rom_hundred
.section .text,"ax"
.global rom_thousand
.type rom_thousand,@function
.align 16
rom_thousand:
	fmovecr f0, #12
	rts
.size rom_thousand,.-rom_thousand
.section .text,"ax"
.global rom_half
.type rom_half,@function
.align 16
rom_half:
	fmovecr f0, #13
	rts
.size rom_half,.-rom_half
.section .text,"ax"
.global rom_tiny
.type rom_tiny,@function
.align 16
rom_tiny:
	fmovecr f0, #14
	rts
.size rom_tiny,.-rom_tiny
.section .text,"ax"
.global rom_max
.type rom_max,@function
.align 16
rom_max:
	fmovecr f0, #15
	rts
.size rom_max,.-rom_max
.section .text,"ax"
.global between
.type between,@function
.align 16
between:
	fmovecr f2, #8
	fmul f0, f0, f2
	fmovecr f2, #13
	fadd f0, f0, f2
	fmul f0, f0, f1
	rts
.size between,.-between
.section .text,"ax"
.global twice
.type twice,@function
.align 16
twice:
	fmovecr f1, #10
	fadd f0, f0, f1
	fmovecr f1, #10
	fmul f0, f0, f1
	rts
.size twice,.-twice
