.section .text,"ax"
.global dot
.type dot,@function
.align 16
dot:
	fipr f0, (r1), (r2)
	rts
.size dot,.-dot
.section .text,"ax"
.global transform
.type transform,@function
.align 16
transform:
	ftrv (r1), (r2)
	rts
.size transform,.-transform
.section .text,"ax"
.global reload
.type reload,@function
.align 16
reload:
	fload f0, (r2)
	ftrv (r1), (r2)
	fload f1, (r2)
	fadd f0, f0, f1
	rts
.size reload,.-reload
.section .text,"ax"
.global reload_store
.type reload_store,@function
.align 16
reload_store:
	add.q r3, r2, #8
	fmovecr f0, #9
	fstore f0, (r3)
	ftrv (r1), (r2)
	fload f0, (r3)
	rts
.size reload_store,.-reload_store
.section .text,"ax"
.global reload_global
.type reload_global,@function
.align 16
reload_global:
	move.l r30, #lo32(gvec+0)
	movt r30, #hi32(gvec+0)
	fload f0, (r30)
	move.l r2, #lo32(gvec+0)
	movt r2, #hi32(gvec+0)
	ftrv (r1), (r2)
	move.l r30, #lo32(gvec+0)
	movt r30, #hi32(gvec+0)
	fload f1, (r30)
	fadd f0, f0, f1
	rts
.size reload_global,.-reload_global
.section .text,"ax"
.global local_vec
.type local_vec,@function
.align 16
local_vec:
	sub.q r31, r31, #24
	lea r3, 0(r31)
	move.q r2, r1
	add.q r5, r3, #4
	add.q r1, r3, #8
	add.q r4, r3, #12
	fmovecr f0, #8
	fstore f0, (r3)
	fmovecr f0, #9
	fstore f0, (r5)
	moveq r30, #1077936128
	fmovi f0, r30
	fstore f0, (r1)
	moveq r30, #1082130432
	fmovi f0, r30
	fstore f0, (r4)
	ftrv (r2), (r3)
	fload f0, (r1)
	add.q r31, r31, #24
	rts
.size local_vec,.-local_vec
.section .text,"ax"
.global dot_unused
.type dot_unused,@function
.align 16
dot_unused:
	fmovecr f0, #8
	fstore f0, (r1)
	fipr f0, (r1), (r2)
	rts
.size dot_unused,.-dot_unused
.section .text,"ax"
.global dot_flags
.type dot_flags,@function
.align 16
dot_flags:
	fipr f0, (r1), (r1)
	fmovsr r1
	sext.l r1, r1
	rts
.size dot_flags,.-dot_flags
.section .text,"ax"
.global read_only
.type read_only,@function
.align 16
read_only:
	fload f0, (r1)
	fipr f1, (r1), (r2)
	fadd f0, f0, f0
	fadd f0, f0, f1
	rts
.size read_only,.-read_only
.section .text,"ax"
.global beyond_vector
.type beyond_vector,@function
.align 16
beyond_vector:
	add.q r3, r2, #16
	fload f0, (r3)
	ftrv (r1), (r2)
	fadd f0, f0, f0
	rts
.size beyond_vector,.-beyond_vector
.section .text,"ax"
.global last_word
.type last_word,@function
.align 16
last_word:
	add.q r3, r2, #12
	fload f0, (r3)
	ftrv (r1), (r2)
	fload f1, (r3)
	fadd f0, f0, f1
	rts
.size last_word,.-last_word
.section .text,"ax"
.global matrix_read
.type matrix_read,@function
.align 16
matrix_read:
	sub.q r31, r31, #88
	lea r1, 64(r31)
	lea r2, 0(r31)
	fmovecr f0, #8
	fstore f0, (r2)
	fmovecr f0, #9
	fstore f0, (r1)
	ftrv (r2), (r1)
	fload f1, (r1)
	fmovecr f0, #8
	fadd f0, f0, f1
	add.q r31, r31, #88
	rts
.size matrix_read,.-matrix_read
.section .text,"ax"
.global untouched_local
.type untouched_local,@function
.align 16
untouched_local:
	ftrv (r1), (r2)
	moveq r30, #1084227584
	fmovi f0, r30
	rts
.size untouched_local,.-untouched_local
.section .text,"ax"
.global dot_forward
.type dot_forward,@function
.align 16
dot_forward:
	fload f2, (r1)
	fload f0, (r2)
	fipr f1, (r1), (r2)
	fadd f0, f0, f0
	fadd f0, f0, f2
	fadd f0, f0, f1
	rts
.size dot_forward,.-dot_forward
