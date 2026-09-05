#include "../all.h"

enum Ie64Reg {
	/* R0 is architectural zero and is deliberately not allocatable. */
	R1 = RXX + 1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12,
	R13, R14, R15, R16, R17,
	/* callee-save integer registers */
	R18, R19, R20, R21, R22, R23, R24, R25,
	/* caller-save integer registers */
	R26, R27, R28, R29, R30,
	/* SP is globally live and is never allocated. */
	R31,
	/* caller-save floating registers */
	F0, F1, F2, F3, F4, F5, F6, F7,
	/* callee-save floating registers */
	F8, F9, F10, F11, F12, F13, F14, F15,
	NFPR = F15 - F0 + 1,
	NGPR = R31 - R1 + 1,
	NGPS = R30 - R1 + 1,
	NFPS = F7 - F0 + 1,
};
/* The `env` pseudo-argument travels in its own caller-save register, outside
 * the ordinary argument sequence so that it never displaces a real argument. */
#define RENV R28

MAKESURE(reg_not_tmp, F15 < (int)Tmp0);

extern int ie64_rsave[];
bits ie64_retregs(Ref, int[2]);
bits ie64_argregs(Ref, int[2]);
void ie64_extsb(Fn *);
void ie64_abi(Fn *);
void ie64_isel(Fn *);
void ie64_emitfn(Fn *, FILE *);
void ie64_emitdat(Dat *, FILE *);
void ie64_emitfin(FILE *);
