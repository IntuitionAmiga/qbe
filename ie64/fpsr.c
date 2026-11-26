/* SPDX-FileCopyrightText: 2026 Zayn Otley
 * SPDX-License-Identifier: LicenseRef-IntuitionEngine-Proprietary
 */
#include "all.h"

/* Value transport does not define the IL's FP status. These are the
 * transports that nevertheless classify a value on the machine. */
int
ie64_fptransport(Ins *i)
{
	if (KBASE(i->cls) != 1)
		return 0;
	if (isload(i->op) || i->op == Ocast || i->op == Oswap)
		return 1;
	return i->op == Ocopy && rtype(i->to) != RSlot
		&& (rtype(i->arg[0]) == RSlot || rtype(i->arg[0]) == RCon);
}

static int
writes(Ins *i)
{
	int k, cc;

	switch (i->op) {
	case Oadd: case Osub: case Omul: case Odiv:
		return KBASE(i->cls) == 1 ? FpCC | FpSticky : 0;
	case Oneg:
		return KBASE(i->cls) == 1 ? FpCC : 0;
	case Ostosi: case Ostoui:
		/* Wide signed and both unsigned forms first widen the input. */
		return FpSticky | (i->op == Ostoui || i->cls == Kl ? FpCC : 0);
	case Odtosi:
		return FpSticky;
	case Odtoui:
		/* The unsigned 64-bit expansion ends with a floating subtraction. */
		return FpSticky | (i->cls == Kl ? FpCC : 0);
	case Oexts: case Otruncd:
	case Oswtof: case Ouwtof: case Osltof: case Oultof:
	case Oie64sltofprep: case Oie64ultofprep:
	case Oie64fmovecr: case Oie64dmovecr: case Oie64fcvtsd: case Oie64fcvtds:
	case Oie64fsincos: case Oie64dsincos:
	case Oie64fabs: case Oie64dabs: case Oie64fint: case Oie64dint:
	case Oie64fround: case Oie64ftrunc: case Oie64ffloor: case Oie64fceil:
	case Oie64dround: case Oie64dtrunc: case Oie64dfloor: case Oie64dceil:
		return FpCC;
	case Oie64ftrv:
		return FpSticky;
	case Oie64fipr:
		return FpCC | FpSticky;
	}
	if (INRANGE(i->op, Oie64fmod, Oie64dsqrt)
	|| INRANGE(i->op, Oie64frsqrt, Oie64dfma)
	|| (iscmp(i->op, &k, &cc) && KBASE(k) == 1))
		return FpCC | FpSticky;
	return 0;
}

/* CAS uses r29. FP operations and calls must see the real FPSR. */
int
ie64_fpbarrier(Ins *i)
{
	return iscallop(i->op) || i->op == Oie64fmovsr || i->op == Oie64fmovsc
		|| i->op == Oie64cas || writes(i);
}

int
ie64_fpsrstep(Ins *i, int live)
{
	if (iscallop(i->op) || i->op == Oie64fmovsr)
		return FpCC | FpSticky;
	if (i->op == Oie64fmovsc)
		return 0;
	/* Exception flags accumulate until an explicit full write. */
	return live & ~(writes(i) & FpCC);
}

int
ie64_fpsrout(Blk *b, unsigned char *in)
{
	Blk *s;
	uint edge;
	int live;

	/* The ABI does not specify FPSR as a return value. */
	live = 0;
	for (edge=0; (s=succ(b, edge)); edge++)
		live |= in[s->id];
	return live;
}

unsigned char *
ie64_fpsrlive(Fn *fn)
{
	unsigned char *in;
	Blk *b;
	Ins *i;
	uint n;
	int chg, live;

	in = alloc(fn->nblk * sizeof in[0]);
	do {
		chg = 0;
		for (n=fn->nblk; n--;) {
			b = fn->rpo[n];
			live = ie64_fpsrout(b, in);
			for (i=b->ins+b->nins; i!=b->ins;)
				live = ie64_fpsrstep(--i, live);
			if (live != in[b->id]) {
				in[b->id] = live;
				chg = 1;
			}
		}
	} while (chg);
	return in;
}

/* Run before folding and again after selection, which can split one
 * conversion into several arithmetic instructions. */
void
ie64_fpsrmark(Fn *fn)
{
	unsigned char *in;
	Blk *b;
	Ins *i;
	int live;

	in = ie64_fpsrlive(fn);
	for (b=fn->start; b; b=b->link) {
		live = ie64_fpsrout(b, in);
		for (i=b->ins+b->nins; i!=b->ins;) {
			--i;
			i->fpkeep = (writes(i) & live) != 0;
			live = ie64_fpsrstep(i, live);
		}
	}
}
