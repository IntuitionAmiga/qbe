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

/* Load elimination can create a loop phi whose only non-self input is
 * invariant. GVN can see its backedge before that edge has been rewritten. */
static int
selfphis(Fn *fn, Ref *map)
{
	Blk *b;
	Phi *p, **pp;
	Ref r;
	uint n;
	int self;

	for (b=fn->start; b; b=b->link)
		for (pp=&b->phi; (p=*pp); pp=&p->link) {
			r = R;
			self = 0;
			for (n=0; n<p->narg; n++) {
				if (req(p->arg[n], p->to))
					self = 1;
				else if (req(r, R))
					r = p->arg[n];
				else if (!req(r, p->arg[n]))
					break;
			}
			if (n != p->narg || !self || req(r, R))
				continue;
			map[p->to.val] = r;
			*pp = p->link;
			return 1;
		}
	return 0;
}

/* IE64 is little endian. Keep the load at its original point and do
 * not change the width of a volatile access or one with other users. */
static void
narrowloads(Fn *fn)
{
	Blk *b;
	Ins *i, *d;
	Tmp *t;
	int64_t mask;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (i->op != Oand || i->cls != Kw || rtype(i->arg[0]) != RTmp
			|| !isconst(fn, i->arg[1], Kw, &mask) || mask != 255)
				continue;
			t = &fn->tmp[i->arg[0].val];
			d = t->def;
			if (t->nuse != 1 || !d || d->cls != Kw
			|| (d->op != Oloadsh && d->op != Oloaduh))
				continue;
			d->op = Oloadub;
			d->mtype = MUnknown;
			i->op = Ocopy;
			i->arg[1] = R;
		}
}

/* Find a definition preserving its word result when widened to l.
 * Signed narrow values must remain sign-extended across spills. */
static Ins *
narrowdef(Fn *fn, Ref r)
{
	Ins *d, *src;
	int op;

	if (rtype(r) != RTmp || r.val < Tmp0 || !(d = fn->tmp[r.val].def))
		return 0;
	op = d->op;
	if (op == Oextsb || op == Oextsh || op == Oextub || op == Oextuh) {
		r = d->arg[0];
		if (rtype(r) == RTmp && r.val >= Tmp0
		&& (src = fn->tmp[r.val].def) && src->op == Oloadsb + op - Oextsb)
			return src;
		return d;
	}
	if (op == Oloadsb || op == Oloadsh || op == Oloadub || op == Oloaduh)
		return d;
	if (op == Ocopy && d->cls == Kw && rtype(d->arg[0]) == RTmp
	&& (src = fn->tmp[d->arg[0].val].def) && src->op == Oloadub)
		return src;
	return 0;
}

static void
narrowequality(Fn *fn)
{
	Blk *b;
	Ins *i, *a, *z;
	int cc, k;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (!iscmp(i->op, &k, &cc) || k != Kw
			|| (cc != Cieq && cc != Cine))
				continue;
			a = narrowdef(fn, i->arg[0]);
			z = narrowdef(fn, i->arg[1]);
			if (!a || !z)
				continue;
			a->cls = z->cls = Kl;
			i->arg[0] = a->to;
			i->arg[1] = z->to;
			i->op += Ocmpl - Ocmpw;
		}
}

/* Let the load produce the long value directly. This keeps the sign
 * through spills and leaves the original access width and location. */
static void
extendloads(Fn *fn)
{
	Blk *b;
	Ins *i, *d;
	int ext, load, op;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (!isext(i->op) || i->cls != Kl
			|| rtype(i->arg[0]) != RTmp)
				continue;
			d = fn->tmp[i->arg[0].val].def;
			if (!d)
				continue;
			op = d->op;
			/* Keep the volatile opcode until the normal late lowering.
			 * The signed load only changes the width of the result. */
			if (ie64_signed_loads && (op == Oloadvsb || op == Oloadvsh || op == Oloadvsw))
				op = Oloadsb + op - Oloadvsb;
			if (op < Oloadsb || op > Oloaduw)
				continue;
			ext = i->op - Oextsb;
			load = op - Oloadsb;
			if ((ext ^ load) & 1 || ext < load)
				continue;
			d->cls = Kl;
			i->op = Ocopy;
		}
}

void
ie64_narrow(Fn *fn)
{
	Ref *map;

	map = alloc(fn->ntmp * sizeof map[0]);
	for (;;) {
		memset(map, 0, fn->ntmp * sizeof map[0]);
		if (!selfphis(fn, map))
			break;
		substitute(fn, map, 0);
		filluse(fn);
	}
	narrowloads(fn);
	narrowequality(fn);
	extendloads(fn);
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

/* A chain of integer operations of the form ((root >> shift) & mask) ^ bits.
 * Track only logical shifts and arithmetic shifts with a known zero sign. */
typedef struct Bitchain Bitchain;
struct Bitchain {
	Ref root;
	uint64_t mask, bits;
	int shift;
};

static uint64_t
lowmask(int n)
{
	return n == 64 ? UINT64_MAX : (UINT64_C(1) << n) - 1;
}

static int
chain(Fn *fn, Blk *b, int k, Ref in, Ref out, Bitchain *v)
{
	Ins *i;
	int width;
	int64_t imm;
	uint64_t c;

	width = k == Kw ? 32 : 64;
	if (b->nins > 8)
		return 0;
	for (i=b->ins; i<&b->ins[b->nins]; i++) {
		if (i->op == Onop)
			continue;
		if (i->cls != k || (k == Kl && isext(i->op)))
			return 0;
		if (req(v->root, R)) {
			v->root = i->arg[0];
			v->mask = lowmask(width);
			in = v->root;
		}
		if (!req(i->arg[0], in))
			return 0;
		in = i->to;
		if (i->op == Oextub)
			c = 255;
		else if (i->op == Oextuh)
			c = 65535;
		else if (isconst(fn, i->arg[1], k, &imm))
			c = imm;
		else
			return 0;
		switch (i->op) {
		case Oand: case Oextub: case Oextuh:
			v->mask &= c;
			v->bits &= c;
			break;
		case Oxor:
			v->bits ^= c;
			break;
		case Oor:
			v->mask &= ~c;
			v->bits |= c;
			break;
		case Osar:
			if ((v->mask | v->bits) >> (width - 1))
				return 0;
			/* fall through */
		case Oshr:
			if (c >= (uint64_t)(width - v->shift))
				return 0;
			v->shift += c;
			v->mask >>= c;
			v->bits >>= c;
			break;
		default:
			return 0;
		}
	}
	if (req(v->root, R)) {
		v->root = out;
		v->mask = lowmask(width);
		return 1;
	}
	return req(in, out);
}

/* Every use of a disappearing definition must disappear with it. */
static int
chainlocal(Fn *fn, Ref r, Blk *mid, Blk *end, Blk *arms[4], Blk *start)
{
	Tmp *t = &fn->tmp[r.val];
	Use *u;
	int n;

	for (u=t->use; u<&t->use[t->nuse]; u++) {
		if (u->type == UPhi && (u->u.phi == mid->phi || u->u.phi == end->phi))
			continue;
		if (u->type != UIns)
			return 0;
		if (u->bid == mid->id)
			continue;
		for (n=0; n<4; n++)
			if (arms[n] != start && arms[n] != mid && arms[n]->id == u->bid)
				break;
		if (n == 4)
			return 0;
	}
	return 1;
}

static int
bitdiamond(Fn *fn, Blk *b)
{
	Blk *arms[4], *mid, *end, *a;
	Phi *p, *q;
	Ins *i;
	Bitchain v[2] = {0};
	Ref x, bit, neg, mask, val;
	uint64_t m, c;
	int n, k, w, ext;

	if (!ifgraph(b, &arms[0], &arms[1], &mid) || !mid->phi
	|| !req(b->jmp.arg, mid->jmp.arg)
	|| !ifgraph(mid, &arms[2], &arms[3], &end) || !end->phi
	|| end == b || end == mid || mid == b)
		return 0;
	p = mid->phi;
	q = end->phi;
	k = p->cls;
	if (p->link || q->link || q->cls != k || KBASE(k) != 0
	|| end->nins > NIns - 10)
		return 0;
	w = k == Kw ? 32 : 64;
	for (n=0; n<2; n++) {
		a = n ? b->s2 : b->s1;
		if (a == mid) {
			v[n].root = phiarg(p, b);
			v[n].mask = lowmask(w);
		} else if (!chain(fn, a, k, R, phiarg(p, a), &v[n]))
			return 0;
		a = n ? mid->s2 : mid->s1;
		x = phiarg(q, a == end ? mid : a);
		/* The first operation of the second arm, when present, reads
		 * the result of the common block. */
		val = x;
		if (a != end)
			for (i=a->ins; i<&a->ins[a->nins]; i++)
				if (i->op != Onop) {
					val = i->arg[0];
					break;
				}
		if (!chain(fn, mid, k, p->to, val, &v[n])
		|| (a != end && !chain(fn, a, k, val, x, &v[n])))
			return 0;
	}
	if (rtype(v[0].root) != RTmp || !req(v[0].root, v[1].root)
	|| v[0].shift != v[1].shift || v[0].mask != v[1].mask)
		return 0;
	if (!chainlocal(fn, p->to, mid, end, arms, b))
		return 0;
	for (n=0; n<=4; n++) {
		a = n == 4 ? mid : arms[n];
		if (n < 4 && (a == b || a == mid))
			continue;
		if (a == end || (a->phi && a != mid))
			return 0;
		for (i=a->ins; i<&a->ins[a->nins]; i++)
			if (i->op != Onop && !chainlocal(fn, i->to, mid, end, arms, b))
				return 0;
	}

	m = v[0].mask;
	c = v[0].bits ^ v[1].bits;
	/* Instructions are emitted backwards. Use an arithmetic mask even
	 * when the incoming condition is a non-boolean word. */
	curi = &insb[NIns];
	icpy(curi -= end->nins, end->ins, end->nins);
	val = q->to;
	mask = R;
	if (c) {
		mask = newtmp("bitmask", k, fn);
		x = newtmp("bitbase", k, fn);
		emit(Oxor, k, val, x, mask);
		val = x;
	}
	if (v[1].bits) {
		x = newtmp("bitbase", k, fn);
		emit(Oxor, k, val, x, getcon(v[1].bits, fn));
		val = x;
	}
	/* Express a mask below a right shift as an input extension when
	 * it came from a subword value. This exposes loop-invariant work. */
	ext = 0;
	if (k == Kw && v[0].shift < 8 && m == lowmask(8 - v[0].shift))
		ext = Oextub;
	else if (k == Kw && v[0].shift < 16 && m == lowmask(16 - v[0].shift))
		ext = Oextuh;
	if (!ext && m != lowmask(w - v[0].shift)) {
		x = newtmp("bitbase", k, fn);
		emit(Oand, k, val, x, getcon(m, fn));
		val = x;
	}
	x = ext ? newtmp("bitinput", k, fn) : v[0].root;
	if (v[0].shift)
		emit(ext ? Osar : Oshr, k, val, x, getcon(v[0].shift, fn));
	else
		emit(Ocopy, k, val, x, R);
	if (ext)
		emit(ext, k, x, v[0].root, R);
	if (c) {
		ie64_extfacts(fn);
		bit = b->jmp.arg;
		x = newtmp("bitneg", k, fn);
		emit(Oand, k, mask, x, getcon(c, fn));
		neg = k == Kw ? x : newtmp("bitnegw", Kw, fn);
		if (k == Kl)
			emit(Oextsw, Kl, x, neg, R);
		if (ie64_uwidth(fn, bit, Kw) <= 1)
			emit(Oneg, Kw, neg, bit, R);
		else {
			/* A nonzero word or its negation has the sign bit set. */
			Ref either = newtmp("bitsign", Kw, fn);
			Ref opposite = newtmp("bitopp", Kw, fn);

			emit(Osar, Kw, neg, either, getcon(31, fn));
			emit(Oor, Kw, either, bit, opposite);
			emit(Oneg, Kw, opposite, bit, R);
		}
	}
	idup(end, curi, &insb[NIns]-curi);
	end->phi = 0;
	b->jmp.type = Jjmp;
	b->jmp.arg = R;
	b->s1 = end;
	b->s2 = 0;
	return 1;
}

/* A bounded count entering at a constant cannot wrap its subword before
 * the backedge when that edge requires the true path of count < limit. */
static void
boundcounter(Fn *fn, Blk *b, Phi *p, int entry)
{
	Ins *add, *ext, *cmp;
	Ref r;
	int64_t initial, increment, limit;
	int max;

	r = p->arg[1-entry];
	if (rtype(r) != RTmp || isreg(r) || !(add = fn->tmp[r.val].def)
	|| add->op != Oadd || add->cls != Kw
	|| !isconst(fn, add->arg[1], Kw, &increment) || increment != 1)
		return;
	r = add->arg[0];
	if (rtype(r) != RTmp || isreg(r) || !(ext = fn->tmp[r.val].def)
	|| ext->cls != Kw || !req(ext->arg[0], p->to))
		return;
	max = ext->op == Oextub ? UINT8_MAX : ext->op == Oextuh ? UINT16_MAX : 0;
	if (!max || !isconst(fn, p->arg[entry], Kw, &initial) || initial > max
	|| b->jmp.type != Jjnz || b->s1->npred != 1
	|| !dom(b->s1, p->blk[1-entry]))
		return;
	r = b->jmp.arg;
	if (rtype(r) != RTmp || isreg(r) || !(cmp = fn->tmp[r.val].def)
	|| (cmp->op != Ocmpw + Cislt && cmp->op != Ocmpw + Ciult)
	|| !req(cmp->arg[0], ext->to)
	|| !isconst(fn, cmp->arg[1], Kw, &limit) || limit > max)
		return;
	ext->op = Ocopy;
	cmp->op = Ocmpw + Ciult;
}

/* Given a zero extension or low mask, report how many input bits it uses. */
static int
lowuse(Fn *fn, Ins *i, Ref r)
{
	int64_t c;

	if (!req(i->arg[0], r))
		return 0;
	if (i->op == Oextub)
		return 8;
	if (i->op == Oextuh)
		return 16;
	if (i->op != Oand || !isconst(fn, i->arg[1], i->cls, &c))
		return 0;
	if ((uint64_t)c <= UINT8_MAX)
		return 8;
	return (uint64_t)c <= UINT16_MAX ? 16 : 0;
}

/* If all users read only low bits and the loop already regenerates a
 * narrow value, extend the incoming value once on the entry edge. */
static void
narrowloopphis(Fn *fn)
{
	Blk *b, *pre;
	Phi *p;
	Tmp *t;
	Use *u;
	Ins ext;
	int n, width, w;

	for (b=fn->start; b; b=b->link)
		for (p=b->phi; p; p=p->link) {
			if (p->cls != Kw || p->narg != 2
			|| dom(b, p->blk[0]) == dom(b, p->blk[1]))
				continue;
			n = dom(b, p->blk[0]);
			boundcounter(fn, b, p, n);
			pre = p->blk[n];
			if (pre->jmp.type != Jjmp || pre->nins >= NIns)
				continue;
			t = &fn->tmp[p->to.val];
			width = 0;
			for (u=t->use; u<&t->use[t->nuse]; u++) {
				w = u->type == UIns ? lowuse(fn, u->u.ins, p->to) : 0;
				if (!w)
					break;
				if (w > width)
					width = w;
			}
			if (u != &t->use[t->nuse] || !width
			|| ie64_uwidth(fn, p->arg[n], Kw) <= width
			|| ie64_uwidth(fn, p->arg[1-n], Kw) > width)
				continue;
			ext = (Ins){.op = width == 8 ? Oextub : Oextuh, .cls = Kw,
				.to = newtmp("bitentry", Kw, fn), .arg = {p->arg[n], R}};
			vgrow(&pre->ins, pre->nins + 1);
			pre->ins[pre->nins++] = ext;
			p->arg[n] = ext.to;
			/* Growing the predecessor's instructions can move uses. */
			filluse(fn);
		}
}

/* Pull a shared low-bit mask out of an XOR when neither masked input
 * is used elsewhere. */
static void
maskedxor(Fn *fn)
{
	Blk *b;
	Ins *i, *a, *d;
	Ref l, r, mask, x, y;
	int64_t c;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			l = i->arg[0];
			r = i->arg[1];
			if (i->op != Oxor || rtype(l) != RTmp || rtype(r) != RTmp
			|| isreg(l) || isreg(r) || req(l, r)
			|| fn->tmp[l.val].nuse != 1 || fn->tmp[r.val].nuse != 1)
				continue;
			a = fn->tmp[l.val].def;
			d = fn->tmp[r.val].def;
			if (!a || !d || a->op != Oand || d->op != Oand
			|| a->cls != i->cls || d->cls != i->cls
			|| !req(a->arg[1], d->arg[1])
			|| !isconst(fn, a->arg[1], i->cls, &c) || c != 1
			|| fn->tmp[l.val].bid != b->id || fn->tmp[r.val].bid != b->id)
				continue;
			/* Both inputs are available at the later of the two masks. */
			if (d < a) {
				Ins *s = a;
				a = d;
				d = s;
			}
			mask = a->arg[1];
			x = a->arg[0];
			y = d->arg[0];
			*a = (Ins){.op = Onop};
			d->op = Oxor;
			d->arg[0] = x;
			d->arg[1] = y;
			i->op = Oand;
			i->arg[0] = d->to;
			i->arg[1] = mask;
		}
}

static int
nestedext(Fn *fn)
{
	Blk *b;
	Ins *i, *d;
	Ref r;
	int changed = 0;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (!isext(i->op))
				continue;
			r = i->arg[0];
			if (rtype(r) != RTmp || isreg(r) || !(d = fn->tmp[r.val].def)
			|| !isext(d->op))
				continue;
			/* Either extension preserves all bits the outer one reads. */
			if ((i->op - Oextsb) / 2 <= (d->op - Oextsb) / 2) {
				i->arg[0] = d->arg[0];
				changed = 1;
			}
		}
	return changed;
}

void
ie64_bitopt(Fn *fn)
{
	Blk *b;
	int changed = 0;

	for (b=fn->start; b; b=b->link)
		if (bitdiamond(fn, b)) {
			fillcfg(fn);
			filluse(fn);
			filldom(fn);
			changed = 1;
		}
	if (changed) {
		ie64_extfacts(fn);
		narrowloopphis(fn);
		filluse(fn);
		maskedxor(fn);
		filluse(fn);
		ssacheck(fn);
	}
	if (nestedext(fn)) {
		/* Remove intermediates made dead by the rewrites. */
		filluse(fn);
		gcm(fn);
		filluse(fn);
		ssacheck(fn);
	}
}

/* All paths out of the old loop passed through its exit. Uses outside
 * the loop therefore see the result phi, including a caller's phi when
 * the search has been inlined. */
static void
searchliveout(Fn *fn, Blk *h, Blk *m, Blk *b, Ref from, Ref to)
{
	Tmp *t;
	Use *u;
	Blk *x;
	Phi *p;
	Ins *i;
	uint n;

	t = &fn->tmp[from.val];
	for (u=t->use; u<&t->use[t->nuse]; u++) {
		x = fn->rpo[u->bid];
		if (x == h || x == m || x == b)
			continue;
		switch (u->type) {
		case UIns:
			i = u->u.ins;
			for (n=0; n<2; n++)
				if (req(i->arg[n], from))
					i->arg[n] = to;
			break;
		case UJmp:
			x->jmp.arg = to;
			break;
		case UPhi:
			p = u->u.phi;
			for (n=0; n<p->narg; n++)
				if (req(p->arg[n], from))
					p->arg[n] = to;
			break;
		default:
			die("invalid use of search pointer");
		}
	}
}

/* Rotate a short-circuit pointer search. The first null test still
 * guards all loads. An invariant ordinary load in the comparison block
 * can use a preheader when the entire original loop is read-only. */
static int
rotatesearch(Fn *fn, Blk *h)
{
	Blk *m, *b, *x, *e, *pre;
	Phi *p, *out;
	Ins *c, *i, *key;
	Ins check;
	Ref first, next, addr;
	uint n;

	p = h->phi;
	if (!p || p->link || p->cls != Kl || p->narg != 2 || h->npred != 2
	|| h->nins != 1 || h->jmp.type != Jjnz)
		return 0;
	c = h->ins;
	if (c->op != Ocmpl + Cine || !req(c->arg[0], p->to)
	|| !req(c->to, h->jmp.arg) || fn->tmp[c->to.val].nuse != 1)
		return 0;
	if (rtype(c->arg[1]) != RCon || fn->con[c->arg[1].val].type != CBits
	|| fn->con[c->arg[1].val].bits.i != 0)
		return 0;
	m = h->s1;
	x = h->s2;
	if (m == h || m == x || m->phi || m->npred != 1 || m->jmp.type != Jjnz)
		return 0;
	b = m->s1 == x ? m->s2 : m->s1;
	if ((m->s1 != x && m->s2 != x) || b == m || b == x || b == h
	|| b->npred != 1 || b->phi || b->jmp.type != Jjmp || b->s1 != h)
		return 0;
	if (x->phi || x->npred != 2)
		return 0;
	n = p->blk[0] == b;
	pre = p->blk[n];
	first = p->arg[n];
	next = p->arg[1-n];
	if (rtype(next) != RTmp || next.val < Tmp0 || fn->tmp[next.val].bid != b->id)
		return 0;
	key = 0;
	for (e=m; e; e=e == m ? b : 0)
		for (i=e->ins; i<&e->ins[e->nins]; i++) {
			if (pinned(i) && !isoptload(i->op))
				return 0;
			if (e != m || key || !isoptload(i->op))
				continue;
			addr = i->arg[0];
			if ((rtype(addr) == RCon && fn->con[addr.val].type == CAddr)
			|| (rtype(addr) == RTmp && addr.val >= Tmp0
			    && dom(fn->rpo[fn->tmp[addr.val].bid], pre)))
				key = i;
		}
	e = h;
	if (key) {
		e = newblk();
		e->name = strf(PFn, "%s.key", m->name);
		e->link = h->link;
		h->link = e;
		e->jmp.type = Jjmp;
		e->s1 = m;
		h->s1 = e;
		idup(e, key, 1);
		*key = (Ins){.op = Onop};
	}
	h->phi = 0;
	m->phi = p;
	p->blk[n] = e;
	c->arg[0] = first;
	check = *c;
	check.to = newtmp("search", Kw, fn);
	check.arg[0] = next;
	addins(&b->ins, &b->nins, &check);
	b->jmp.type = Jjnz;
	b->jmp.arg = check.to;
	b->s1 = m;
	b->s2 = x;
	out = alloc(sizeof *out);
	out->cls = Kl;
	out->to = newtmp("search", Kl, fn);
	out->narg = 3;
	out->blk = vnew(3, sizeof out->blk[0], PFn);
	out->arg = vnew(3, sizeof out->arg[0], PFn);
	out->blk[0] = h;
	out->arg[0] = first;
	out->blk[1] = m;
	out->arg[1] = p->to;
	out->blk[2] = b;
	out->arg[2] = next;
	searchliveout(fn, h, m, b, p->to, out->to);
	x->phi = out;
	return 1;
}

void
ie64_searchloops(Fn *fn)
{
	Blk *h;

	for (h=fn->start; h; h=h->link)
		if (rotatesearch(fn, h)) {
			fillcfg(fn);
			filluse(fn);
			filldom(fn);
			ssacheck(fn);
		}
}
