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

/* Replace every use of a temporary that map names by its image. */
static void
substitute(Fn *fn, Ref *map, Ref *jmap)
{
	Blk *b;
	Ins *i;
	Phi *p;
	Ref r;
	uint n;

	for (b=fn->start; b; b=b->link) {
		for (p=b->phi; p; p=p->link)
			for (n=0; n<p->narg; n++)
				if (rtype(p->arg[n]) == RTmp && p->arg[n].val >= Tmp0
				&& (r = map[p->arg[n].val]).val != 0)
					p->arg[n] = r;
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			for (n=0; n<2; n++)
				if (rtype(i->arg[n]) == RTmp && i->arg[n].val >= Tmp0
				&& (r = map[i->arg[n].val]).val != 0)
					i->arg[n] = r;
		if (rtype(b->jmp.arg) == RTmp && b->jmp.arg.val >= Tmp0) {
			if ((r = map[b->jmp.arg.val]).val != 0)
				b->jmp.arg = r;
			else if (b->jmp.type == Jjnz && jmap
			&& (r = jmap[b->jmp.arg.val]).val != 0)
				b->jmp.arg = r;
		}
	}
}

/*
 * `r = c ? 1 : 0` on a comparison c is the comparison.  Front ends write it
 * as a conditional jump to two empty blocks and a phi of 1 and 0.  The
 * jump goes, the users of the phi read the comparison, and GCM can copy it
 * to the jumps that test it, where it fuses.
 */
static int
boolphi(Fn *fn, Ref *map)
{
	Blk *j, *a, *z, *d;
	Phi *p;
	Tmp *t;
	Con *c0, *c1;
	int ck, cc, ia, any;

	any = 0;
	for (j=fn->start; j; j=j->link) {
		p = j->phi;
		if (!p || p->link || p->cls != Kw || p->narg != 2 || j->npred != 2)
			continue;
		if (rtype(p->arg[0]) != RCon || rtype(p->arg[1]) != RCon)
			continue;
		c0 = &fn->con[p->arg[0].val];
		c1 = &fn->con[p->arg[1].val];
		if (c0->type != CBits || c1->type != CBits || c0->flt || c1->flt)
			continue;
		if (c0->bits.i == 1 && c1->bits.i == 0)
			ia = 0;
		else if (c0->bits.i == 0 && c1->bits.i == 1)
			ia = 1;
		else
			continue;
		a = p->blk[ia];
		z = p->blk[1-ia];
		if (a == z || a->nins || z->nins || a->npred != 1 || z->npred != 1
		|| a->pred[0] != z->pred[0])
			continue;
		d = a->pred[0];
		if (d->jmp.type != Jjnz || d->s1 != a || d->s2 != z
		|| rtype(d->jmp.arg) != RTmp || d->jmp.arg.val < Tmp0)
			continue;
		t = &fn->tmp[d->jmp.arg.val];
		if (t->ndef != 1 || !t->def || t->def->cls != Kw
		|| !iscmp(t->def->op, &ck, &cc))
			continue;
		map[p->to.val] = d->jmp.arg;
		j->phi = 0;
		d->jmp.type = Jjmp;
		d->jmp.arg = R;
		d->s1 = j;
		d->s2 = 0;
		any = 1;
	}
	return any;
}

/*
 * A comparison builds 0 or 1, so the zero- or sign-extension of its
 * result from 8 or 16 bits changes nothing.  Front ends emit one for
 * a comparison stored in a small integer type.  Its users then read the
 * comparison itself.
 */
static int
cmpext(Fn *fn, Ref *map, Ref *jmap)
{
	Blk *b;
	Ins *i, *d;
	Tmp *t;
	int ck, cc, any;

	any = 0;
	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if ((i->cls != Kw && i->cls != Kl) || rtype(i->arg[0]) != RTmp
			|| rtype(i->to) != RTmp || i->to.val < Tmp0
			|| i->arg[0].val < Tmp0)
				continue;
			if (i->op != Oextub && i->op != Oextuh && i->op != Oextuw
			&& i->op != Oextsb && i->op != Oextsh && i->op != Oextsw)
				continue;
			if (i->cls == Kw && (i->op == Oextuw || i->op == Oextsw))
				continue;
			t = &fn->tmp[i->arg[0].val];
			d = t->def;
			if (t->ndef != 1 || !d || d->cls != Kw
			|| !iscmp(d->op, &ck, &cc))
				continue;
			/* A 64-bit result is read as a condition only. */
			if (i->cls == Kw)
				map[i->to.val] = i->arg[0];
			else
				jmap[i->to.val] = i->arg[0];
			any = 1;
		}
	return any;
}

/* Runs before GCM, on SSA form with up to date uses.  Returns whether the
 * control flow changed. */
int
ie64_preopt(Fn *fn)
{
	Ref *map, *jmap;
	int cfg;

	ie64_rangecmp(fn);
	/* The phis go first: the extension of one reads a comparison. */
	map = alloc(fn->ntmp * sizeof map[0]);
	jmap = alloc(fn->ntmp * sizeof jmap[0]);
	cfg = boolphi(fn, map);
	if (cfg) {
		substitute(fn, map, 0);
		fillcfg(fn);
		filluse(fn);
		filldom(fn);
		memset(map, 0, fn->ntmp * sizeof map[0]);
	}
	if (cmpext(fn, map, jmap))
		substitute(fn, map, jmap);
	return cfg;
}
