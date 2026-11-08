#include "all.h"

/*
 * Range checks.  `x >= lo & x <= hi` on one operand is the single
 * unsigned compare `x - lo <= hi - lo`, which costs one subtraction and
 * one compare in place of two compares that each build a flag value.
 */

/* One bound on x: x REL c, with the constant on the right. */
typedef struct Bound Bound;
struct Bound {
	Ref x;
	int cc;		/* signed or unsigned order, Cisge..Ciult */
	int64_t c;
	Ins *def;
};

static int
swapcc(int cc)
{
	switch (cc) {
	case Cisge: return Cisle;
	case Cisgt: return Cislt;
	case Cisle: return Cisge;
	case Cislt: return Cisgt;
	case Ciuge: return Ciule;
	case Ciugt: return Ciult;
	case Ciule: return Ciuge;
	case Ciult: return Ciugt;
	}
	return cc;
}

static int
isconst(Fn *fn, Ref r, int k, int64_t *v)
{
	Con *c;

	if (rtype(r) != RCon || (c = &fn->con[r.val])->type != CBits || c->flt)
		return 0;
	*v = c->bits.i;
	if (k == Kw)
		*v = (int64_t)(uint32_t)*v;
	return 1;
}

static int
bound(Fn *fn, Ref r, int k, Bound *bd)
{
	Ins *d;
	Tmp *t;
	int ck, cc;
	int64_t v;

	if (rtype(r) != RTmp || isreg(r) || r.val < Tmp0)
		return 0;
	t = &fn->tmp[r.val];
	if (t->ndef != 1 || t->nuse != 1 || !(d = t->def))
		return 0;
	if (!iscmp(d->op, &ck, &cc) || ck != k || KBASE(ck) != 0)
		return 0;
	if (cc < Cisge || cc > Ciult)
		return 0;
	if (rtype(d->arg[0]) == RTmp && !isreg(d->arg[0])
	&& isconst(fn, d->arg[1], k, &v)) {
		bd->x = d->arg[0];
	} else if (rtype(d->arg[1]) == RTmp && !isreg(d->arg[1])
	&& isconst(fn, d->arg[0], k, &v)) {
		bd->x = d->arg[1];
		cc = swapcc(cc);
	} else
		return 0;
	bd->cc = cc;
	bd->c = v;
	bd->def = d;
	return 1;
}

/* Normalise a bound to x >= lo or x <= hi in the order of its kind.
 * Returns 1 for a lower bound, 2 for an upper bound, 0 if the bound
 * cannot be written that way.  The value is in the domain of the
 * class: signed for the signed orders, unsigned otherwise. */
static int
norm(Bound *bd, int k, int64_t *v)
{
	int sign = bd->cc <= Cislt;
	int64_t c, mn, mx;

	if (k == Kw) {
		c = sign ? (int64_t)(int32_t)bd->c : (int64_t)(uint32_t)bd->c;
		mn = sign ? INT32_MIN : 0;
		mx = sign ? INT32_MAX : (int64_t)UINT32_MAX;
	} else if (sign) {
		c = bd->c;
		mn = INT64_MIN;
		mx = INT64_MAX;
	} else {
		/* 64-bit unsigned bounds above INT64_MAX are not handled. */
		if (bd->c < 0)
			return 0;
		c = bd->c;
		mn = 0;
		mx = INT64_MAX;
	}
	switch (bd->cc) {
	case Cisge: case Ciuge:
		*v = c;
		return 1;
	case Cisgt: case Ciugt:
		if (c == mx)
			return 0;
		*v = c + 1;
		return 1;
	case Cisle: case Ciule:
		*v = c;
		return 2;
	case Cislt: case Ciult:
		if (c == mn)
			return 0;
		*v = c - 1;
		return 2;
	}
	return 0;
}

void
ie64_rangecmp(Fn *fn)
{
	Blk *b;
	Ins *i;
	Bound ba, bb;
	int ka, kb, k;
	int64_t lv, hv, d;
	Ref t;
	uint64_t range;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (i->op != Oand || i->cls != Kw)
				continue;
			if (!bound(fn, i->arg[0], Kw, &ba)
			|| !bound(fn, i->arg[1], Kw, &bb)
			|| !req(ba.x, bb.x) || ba.def == bb.def)
				continue;
			if ((ba.cc <= Cislt) != (bb.cc <= Cislt))
				continue;
			k = Kw;
			ka = norm(&ba, k, &lv);
			kb = norm(&bb, k, &hv);
			if (ka == 2 && kb == 1) {
				d = lv;
				lv = hv;
				hv = d;
			} else if (ka != 1 || kb != 2)
				continue;
			if (lv > hv)
				continue;
			range = (uint64_t)hv - (uint64_t)lv;
			if (range > UINT32_MAX)
				continue;
			t = newtmp("ie64rng", k, fn);
			/* The first definition becomes the subtraction, the
			 * and becomes the compare. */
			if (lv < 0)
				*ba.def = (Ins){.op = Oadd, .cls = k, .to = t,
					.arg = {ba.x, getcon((uint32_t)-lv, fn)}};
			else
				*ba.def = (Ins){.op = Osub, .cls = k, .to = t,
					.arg = {ba.x, getcon(lv, fn)}};
			*bb.def = (Ins){.op = Onop};
			*i = (Ins){.op = Ocmpw + Ciule, .cls = Kw, .to = i->to,
				.arg = {t, getcon((int64_t)range, fn)}};
		}
}
