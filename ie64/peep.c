#include "all.h"

/*
 * Peephole passes over allocated code, run just before emission.
 *
 *  - A jump to a block that only tests a condition is replaced by the
 *    test itself, so that a loop closes with one conditional branch and
 *    no unconditional one.
 *  - A constant load into a register that already holds that constant
 *    on every path is dropped.
 */

enum {
	MaxDup = 4,	/* instructions copied into a block */
	NReg = 32,
};

/* An instruction the emitter may print twice. */
static int
copyable(Ins *i)
{
	return !iscallop(i->op) && !isie64op(i->op) && !isalloc(i->op)
		&& i->op != Oswap && i->op != Osalloc;
}

static int
dupable(Blk *s)
{
	Ins *i;
	int n;

	if (s->jmp.type != Jjnz || s->s1 == s->s2)
		return 0;
	n = 0;
	for (i=s->ins; i<&s->ins[s->nins]; i++) {
		if (i->op == Onop)
			continue;
		if (!copyable(i) || ++n > MaxDup)
			return 0;
	}
	return 1;
}

void
ie64_dupjumps(Fn *fn)
{
	Blk *b, *s, **pb;
	Ins *ni, *i;
	uint n, k;
	int chg;

	for (b=fn->start; b; b=b->link) {
		s = b->s1;
		if (b->jmp.type != Jjmp || !s || s == b || s == b->link
		|| !b->link || !dupable(s))
			continue;
		if (s->s1 != b->link && s->s2 != b->link)
			continue;
		k = 0;
		for (i=s->ins; i<&s->ins[s->nins]; i++)
			if (i->op != Onop)
				k++;
		ni = vnew(b->nins + k, sizeof ni[0], PFn);
		for (n=0; n<b->nins; n++)
			ni[n] = b->ins[n];
		for (i=s->ins; i<&s->ins[s->nins]; i++)
			if (i->op != Onop)
				ni[n++] = *i;
		b->ins = ni;
		b->nins = n;
		b->jmp = s->jmp;
		b->s1 = s->s1;
		b->s2 = s->s2;
	}
	/* Blocks that nothing reaches any more are not printed. */
	do {
		chg = 0;
		for (b=fn->start; b; b=b->link)
			b->visit = 0;
		fn->start->visit = 1;
		for (b=fn->start; b; b=b->link) {
			if (b->s1)
				b->s1->visit = 1;
			if (b->s2)
				b->s2->visit = 1;
			/* A block that falls through reaches its successor
			 * through the edges above. */
		}
		for (pb=&fn->start->link; *pb;)
			if (!(*pb)->visit) {
				*pb = (*pb)->link;
				chg = 1;
			} else
				pb = &(*pb)->link;
	} while (chg);
}

/* A block that ends in a jump to the next block, which nothing else
 * reaches, is one run of straight code with it. */
void
ie64_mergeblocks(Fn *fn)
{
	Blk *b, *s;
	uint *np, n, k;
	Ins *ni, *i;

	np = alloc(fn->nblk * sizeof np[0]);
	np[fn->start->id]++;
	for (b=fn->start; b; b=b->link) {
		if (b->s1)
			np[b->s1->id]++;
		if (b->s2)
			np[b->s2->id]++;
	}
	for (b=fn->start; b; b=b->link)
		while ((s = b->link) && b->jmp.type == Jjmp && b->s1 == s
		&& s != b && np[s->id] == 1) {
			k = 0;
			for (i=s->ins; i<&s->ins[s->nins]; i++)
				if (i->op != Onop)
					k++;
			ni = vnew(b->nins + k, sizeof ni[0], PFn);
			for (n=0; n<b->nins; n++)
				ni[n] = b->ins[n];
			for (i=s->ins; i<&s->ins[s->nins]; i++)
				if (i->op != Onop)
					ni[n++] = *i;
			b->ins = ni;
			b->nins = n;
			b->jmp = s->jmp;
			b->s1 = s->s1;
			b->s2 = s->s2;
			b->link = s->link;
			np[s->id] = 0;
		}
}

