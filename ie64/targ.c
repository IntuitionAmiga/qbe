#include "all.h"

int ie64_rsave[] = {
	R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
	R16, R17, R26, R27, R28,
	F0, F1, F2, F3, F4, F5, F6, F7,
	-1
};

static int
ie64_regwidth(int cls)
{
	return cls == Kd ? 2 : 1;
}

static int
ie64_memargs(int op)
{
	(void)op;
	return 0;
}

Target T_ie64 = {
	.name = "ie64",
	.gpr0 = R1,
	.ngpr = NGPR,
	.fpr0 = F0,
	.nfpr = NFPR,
	/* R25 holds the fixed frame base for dynamic allocations.  R29 and R30
	 * are instruction-selection scratch registers. */
	.rglob = BIT(R25) | BIT(R29) | BIT(R30) | BIT(R31),
	.nrglob = 4,
	.regwidth = ie64_regwidth,
	.rsave = ie64_rsave,
	.nrsave = {20, NFPS},
	.retregs = ie64_retregs,
	.argregs = ie64_argregs,
	.memargs = ie64_memargs,
	.abi0 = ie64_extsb,
	.abi1 = ie64_abi,
	.isel = ie64_isel,
	.emitfn = ie64_emitfn,
	.emitdat = ie64_emitdat,
	.emitfin = ie64_emitfin,
	.asloc = ".L",
	.cansel = 0,
};
