#include "all.h"

/*
 * Known-extension facts for IE64 integer temporaries.
 *
 * For each temporary the analysis records u, a bound such that the value
 * is below 2^u (unsigned, u = 64 when unknown), and s, a bound such that
 * the value lies in [-2^(s-1), 2^(s-1)).  The facts describe the register
 * contents the emitter produces, and they have to survive a spill:
 *
 *  - A w temporary keeps only its zero-extended low 32 bits across a spill
 *    (store.l / load.l), so for class w only u is recorded, and only when
 *    the defining instruction leaves the upper half clear.  A signed w
 *    value, whose register holds a sign extension, is recorded as unknown.
 *  - An l temporary is spilled whole (store.q / load.q), so both facts hold.
 *
 * Definitions are visited until nothing changes.  Unseen temporaries are
 * optimistic (bottom) during the walk, so a phi cycle does not lose facts.
 */

typedef struct Fact Fact;
struct Fact {
	signed char u, s;
	char def;
};

static Fact *facts;
static uint nfacts;
static int optimistic;

static int
max(int a, int b)
{
	return a > b ? a : b;
}

static int
min(int a, int b)
{
	return a < b ? a : b;
}

static int
bitlen(uint64_t v)
{
	int n;

	for (n=0; v; n++)
		v >>= 1;
	return n;
}

static int
sbits(int64_t v)
{
	return bitlen(v < 0 ? ~(uint64_t)v : (uint64_t)v) + 1;
}

/* Facts of an operand used with class k. */
static void
argfact(Fn *fn, Ref r, int k, int *u, int *s)
{
	Con *c;
	int64_t v;

	*u = *s = 64;
	if (rtype(r) == RCon) {
		c = &fn->con[r.val];
		if (c->type != CBits || c->flt)
			return;
		v = c->bits.i;
		if (k == Kw) {
			*u = bitlen((uint32_t)v);
			*s = 64;
			return;
		}
		*u = v < 0 ? 64 : bitlen(v);
		*s = sbits(v);
		return;
	}
	if (rtype(r) != RTmp || isreg(r) || r.val >= nfacts)
		return;
	if (facts[r.val].def) {
		*u = facts[r.val].u;
		*s = facts[r.val].s;
	} else if (optimistic)
		*u = *s = 0;
}

static int
up(int u)
{
	return u > 64 ? 64 : u;
}

/* Compute the facts of an instruction result. */
static void
transfer(Fn *fn, Ins *i, int *ru, int *rs)
{
	int ua, sa, ub, sb, k, kw, ck, cc;
	int64_t v;
	Con *c;

	*ru = *rs = 64;
	k = i->cls;
	kw = k == Kw;
	if (KBASE(k) != 0)
		return;
	argfact(fn, i->arg[0], argcls(i, 0), &ua, &sa);
	argfact(fn, i->arg[1], argcls(i, 1), &ub, &sb);
	if (isext(i->op)) {
		switch (i->op) {
		case Oextub: *ru = min(8, ua); break;
		case Oextuh: *ru = min(16, ua); break;
		case Oextuw: *ru = min(32, ua); break;
		case Oextsb: *rs = min(8, min(sa, ua + 1)); *ru = ua < 8 ? ua : 64; break;
		case Oextsh: *rs = min(16, min(sa, ua + 1)); *ru = ua < 16 ? ua : 64; break;
		case Oextsw: *rs = min(32, min(sa, ua + 1)); *ru = ua < 32 ? ua : 64; break;
		}
		goto Done;
	}
	if (isload(i->op)) {
		switch (i->op) {
		case Oloadub: *ru = 8; break;
		case Oloaduh: *ru = 16; break;
		case Oloaduw: *ru = 32; break;
		case Oload:
			if (kw)
				*ru = 32;
			break;
		case Oloadsb: *rs = 8; break;
		case Oloadsh: *rs = 16; break;
		case Oloadsw:
			*rs = 32;
			if (kw)
				*ru = 32;
			break;
		}
		/* A w result is the sign-extended register unless the load zero
		 * extends; the emitter sign-extends signed loads in every class. */
		if (kw && (i->op == Oloadsb || i->op == Oloadsh || i->op == Oloadsw))
			*ru = 64;
		goto Done;
	}
	if (iscmp(i->op, &ck, &cc)) {
		*ru = 1;
		goto Done;
	}
	switch (i->op) {
	case Oie64cmp3sw: case Oie64cmp3sl: case Oie64cmp3uw: case Oie64cmp3ul:
		*rs = 2;
		break;
	case Ocopy:
		if (rtype(i->arg[0]) == RCon) {
			c = &fn->con[i->arg[0].val];
			if (c->type != CBits || c->flt)
				break;
			/* A w constant is loaded zero-extended. */
			*ru = ua;
			*rs = kw ? 64 : sa;
			goto Done;
		}
		if (rtype(i->arg[0]) != RTmp || isreg(i->arg[0]))
			break;
		*ru = ua;
		*rs = sa;
		goto Done;
	case Oand:
		*ru = min(ua, ub);
		if (!kw && (*ru < 64))
			*rs = *ru + 1;
		break;
	case Oor:
	case Oxor:
		*ru = max(ua, ub);
		*rs = max(sa, sb);
		break;
	case Oshr:
		if (rtype(i->arg[1]) == RCon && (c = &fn->con[i->arg[1].val])->type == CBits
		&& 0 < (v = c->bits.i) && v < (kw ? 32 : 64)) {
			if (kw)
				*ru = ua <= 32 ? max(ua - (int)v, 0) : 32 - (int)v;
			else
				*ru = ua < 64 ? max(ua - (int)v, 0) : 64 - (int)v;
		}
		break;
	case Osar:
		/* A w value below 2^31 is non-negative, so the shift is a
		 * logical one. */
		if (kw && ua <= 31 && rtype(i->arg[1]) == RCon
		&& (c = &fn->con[i->arg[1].val])->type == CBits
		&& 0 < (v = c->bits.i) && v < 32) {
			*ru = max(ua - (int)v, 0);
			break;
		}
		if (!kw && rtype(i->arg[1]) == RCon
		&& (c = &fn->con[i->arg[1].val])->type == CBits
		&& 0 < (v = c->bits.i) && v < 64) {
			*rs = max(sa - (int)v, 1);
			*ru = ua < 64 ? max(ua - (int)v, 0) : 64;
		}
		break;
	case Oie64rol: case Oie64ror:
	case Oadd: case Osub: case Omul: case Oneg: case Oshl:
	case Odiv: case Orem: case Oudiv: case Ourem:
		break;
	default:
		return;
	}
Done:
	if (kw) {
		/* Only zero-extended results are facts for w. */
		*rs = 64;
		/* w arithmetic leaves the upper half clear. */
		switch (i->op) {
		case Oadd: case Osub: case Omul: case Oneg: case Oshl: case Osar:
		case Odiv: case Orem: case Oudiv: case Ourem: case Oshr:
		case Oand: case Oor: case Oxor: case Oie64rol: case Oie64ror:
			*ru = min(*ru, 32);
			break;
		}
		if (*ru > 32)
			*ru = 64;
	}
	*ru = up(*ru);
	if (*ru < 64 && *rs > *ru + 1)
		*rs = *ru + 1;
	if (kw)
		*rs = 64;
}

static int
join(Ref r, int u, int s)
{
	Fact *f;

	if (rtype(r) != RTmp || isreg(r) || r.val >= nfacts)
		return 0;
	f = &facts[r.val];
	if (!f->def) {
		f->def = 1;
		f->u = u;
		f->s = s;
		return 1;
	}
	if (u <= f->u && s <= f->s)
		return 0;
	f->u = max(f->u, u);
	f->s = max(f->s, s);
	return 1;
}

void
ie64_extfacts(Fn *fn)
{
	Blk *b;
	Phi *p;
	Ins *i;
	uint n;
	int chg, u, s, ua, sa;

	nfacts = fn->ntmp;
	facts = alloc(nfacts * sizeof facts[0]);
	optimistic = 1;
	do {
		chg = 0;
		for (b=fn->start; b; b=b->link) {
			for (p=b->phi; p; p=p->link) {
				if (KBASE(p->cls) != 0)
					continue;
				u = s = 0;
				for (n=0; n<p->narg; n++) {
					argfact(fn, p->arg[n], p->cls, &ua, &sa);
					u = max(u, ua);
					s = max(s, sa);
				}
				if (p->cls == Kw)
					s = 64;
				chg |= join(p->to, u, s);
			}
			for (i=b->ins; i<&b->ins[b->nins]; i++) {
				if (rtype(i->to) != RTmp || isreg(i->to))
					continue;
				transfer(fn, i, &u, &s);
				chg |= join(i->to, u, s);
			}
		}
	} while (chg);
	optimistic = 0;
}

/* Unsigned width bound of a value read as class k: below 2^result. */
int
ie64_uwidth(Fn *fn, Ref r, int k)
{
	int u, s;

	argfact(fn, r, k, &u, &s);
	return u;
}

/* Signed width bound: in [-2^(result-1), 2^(result-1)). */
int
ie64_swidth(Fn *fn, Ref r, int k)
{
	int u, s;

	argfact(fn, r, k, &u, &s);
	if (u < 64 && s > u + 1)
		s = u + 1;
	return s;
}

/*
 * Canonical zero-extension of a w value read by several operations that
 * need it.  A value of unknown extension (a parameter, a call result, a
 * sign-extending load) used by two or more such operations, or by one in
 * a loop, is zero-extended once right after its definition, and the
 * operations read the extended copy.
 */
static int
zextuse(Ins *i, int n)
{
	int ck, cc;

	if (i->cls == Kw && n == 0 && (i->op == Oshr))
		return 1;
	if (i->cls == Kw && (i->op == Oudiv || i->op == Ourem))
		return 1;
	if (i->op == Oextuw && n == 0)
		return 1;
	if (iscmp(i->op, &ck, &cc) && ck == Kw)
		return !(cc >= Cisge && cc <= Cislt);
	return 0;
}

/* Is an extension a no-op for what is known about its operand? */
int
ie64_extisnop(Ins *i, Fn *fn)
{
	Ref r = i->arg[0];

	if (rtype(r) != RTmp || isreg(r))
		return 0;
	switch (i->op) {
	case Oextub: return ie64_uwidth(fn, r, Kw) <= 8;
	case Oextuh: return ie64_uwidth(fn, r, Kw) <= 16;
	case Oextuw: return ie64_uwidth(fn, r, Kw) <= 32;
	case Oextsb: return ie64_swidth(fn, r, Kw) <= 8;
	case Oextsh: return ie64_swidth(fn, r, Kw) <= 16;
	case Oextsw: return ie64_swidth(fn, r, Kw) <= 32;
	}
	return 0;
}

/*
 * A w value is held in a register as its zero or sign extension, and a
 * conditional branch tests the whole register, as does the callee of a call
 * that takes its w parameter from one.  An l temporary that stands in for a
 * w value breaks this: forwarding a store of a w slot to its load, or the
 * removal of a w copy of an l value, leaves the l temporary where a w one
 * was meant, and its upper half may be set.  A symbol address does the same:
 * it is all 64 bits.  Extend such a value to a w temporary with a zero
 * extension where a branch tests it, where it becomes a w argument of a
 * call, and where it enters a w phi that one of these reads.  An l temporary
 * whose value is known to fit 32 bits needs none.  The extension is not a
 * copy: the register allocator drops a copy whose two sides share a register.
 */
static int
wdirty(Fn *fn, Ref r)
{
	Con *c;

	if (rtype(r) == RCon) {
		c = &fn->con[r.val];
		return c->type == CAddr;
	}
	return rtype(r) == RTmp && !isreg(r) && fn->tmp[r.val].cls == Kl
		&& ie64_uwidth(fn, r, Kw) > 32;
}

/* Write the instructions that extend r into v; give the w temporary. */
static uint
wext(Fn *fn, Ref r, Ins *v, Ref *t)
{
	uint k;

	k = 0;
	if (rtype(r) == RCon) {
		v[k++] = (Ins){.op = Ocopy, .cls = Kl,
			.to = newtmp("ie64w", Kl, fn), .arg = {r, R}};
		r = v[0].to;
	}
	*t = newtmp("ie64w", Kw, fn);
	v[k++] = (Ins){.op = Oextuw, .cls = Kw, .to = *t, .arg = {r, R}};
	return k;
}

/* Insert k instructions before the one at index at. */
static void
insat(Blk *b, uint at, Ins *v, uint k)
{
	Ins *ni;

	ni = vnew(b->nins + k, sizeof ni[0], PFn);
	memcpy(ni, b->ins, at * sizeof ni[0]);
	memcpy(ni + at, v, k * sizeof ni[0]);
	memcpy(ni + at + k, b->ins + at, (b->nins - at) * sizeof ni[0]);
	b->ins = ni;
	b->nins += k;
}

/* Is the instruction the copy of a w argument to an argument register? */
static int
wargcopy(Ins *i)
{
	return i->op == Ocopy && i->cls == Kw && isreg(i->to)
		&& i->to.val >= R1 && i->to.val <= R8;
}

static void
wnorm(Fn *fn)
{
	Blk *b;
	Phi *p, **def, **stk;
	Ins v[2];
	uint n, k, nt, ns, at, lc;
	char *need;
	Ref r, t;

	/* A phi is mended only when a sink reads it, directly or through
	 * other phis.  The other readers of a w value ignore the upper half. */
	nt = fn->ntmp;
	def = alloc(nt * sizeof def[0]);
	stk = alloc(nt * sizeof stk[0]);
	need = alloc(nt);
	ns = 0;
	for (b=fn->start; b; b=b->link)
		for (p=b->phi; p; p=p->link)
			if (p->cls == Kw && rtype(p->to) == RTmp && p->to.val < nt)
				def[p->to.val] = p;
#define SINK(r) do { \
		if (rtype(r) == RTmp && (r).val < nt && def[(r).val] \
		&& !need[(r).val]) { \
			need[(r).val] = 1; \
			stk[ns++] = def[(r).val]; \
		} \
	} while (0)
	for (b=fn->start; b; b=b->link) {
		/* The argument copies of a call come before its last one. */
		lc = 0;
		for (n=0; n<b->nins; n++)
			if (iscallop(b->ins[n].op))
				lc = n;
		for (n=lc; n>0; n--) {
			at = n - 1;
			if (!wargcopy(&b->ins[at]))
				continue;
			r = b->ins[at].arg[0];
			SINK(r);
			if (wdirty(fn, r)) {
				k = wext(fn, r, v, &t);
				b->ins[at].arg[0] = t;
				insat(b, at, v, k);
			}
		}
		if (b->jmp.type != Jjnz)
			continue;
		r = b->jmp.arg;
		SINK(r);
		if (wdirty(fn, r)) {
			k = wext(fn, r, v, &t);
			insat(b, b->nins, v, k);
			b->jmp.arg = t;
		}
	}
	while (ns > 0) {
		p = stk[--ns];
		for (n=0; n<p->narg; n++) {
			r = p->arg[n];
			if (wdirty(fn, r)) {
				k = wext(fn, r, v, &t);
				insat(p->blk[n], p->blk[n]->nins, v, k);
				p->arg[n] = t;
			} else
				SINK(r);
		}
	}
#undef SINK
}

/* A sign-extended word must be kept as an l temporary. A w spill
 * would otherwise discard the upper half and its extension fact. */
static int
signedusers(Tmp *t)
{
	Use *u;
	Ins *i;
	int k, cc;

	if (!t->nuse)
		return 0;
	for (u=t->use; u<&t->use[t->nuse]; u++) {
		if (u->type != UIns)
			return 0;
		i = u->u.ins;
		if (i->op == Oextsw || i->op == Oswtof || i->op == Oie64cmp3sw)
			continue;
		if (iscmp(i->op, &k, &cc) && k == Kw && cc >= Cisge && cc <= Cislt)
			continue;
		return 0;
	}
	return 1;
}

static void
selectloadwidth(Fn *fn)
{
	Blk *b;
	Ins *i, *d;
	Tmp *t;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (i->op == Oextub && rtype(i->arg[0]) == RTmp && !isreg(i->arg[0])) {
				t = &fn->tmp[i->arg[0].val];
				d = t->def;
				/* A non-volatile little-endian halfword read used solely
				 * for its low byte can use an unsigned byte load. */
				if (t->nuse == 1 && d && (d->op == Oloaduh || d->op == Oloadsh)) {
					d->op = Oloadub;
					d->mtype = MUnknown;
				}
			}
			if (i->cls != Kw || rtype(i->to) != RTmp || isreg(i->to))
				continue;
			t = &fn->tmp[i->to.val];
			switch (i->op) {
			case Oload: case Oloadsw: case Oloaduw:
				if ((!ie64_signed_loads && t->nuse < 2) || !signedusers(t))
					break;
				i->op = Oloadsw;
				i->cls = Kl;
				break;
			case Oloadsb: case Oloadsh: case Oextsb: case Oextsh:
				if (signedusers(t))
					i->cls = Kl;
				break;
			}
		}
	filluse(fn);
}

void
ie64_canon(Fn *fn)
{
	Blk *b;
	Phi *p;
	Ins *i, *ni;
	uint n, t, nt, nins, k;
	int *zc, w;
	Ref *to, r;

	filluse(fn);
	selectloadwidth(fn);
	ie64_extfacts(fn);
	wnorm(fn);
	nt = fn->ntmp;
	zc = alloc(nt * sizeof zc[0]);
	to = alloc(nt * sizeof to[0]);
	for (t=0; t<nt; t++)
		to[t] = R;
	for (b=fn->start; b; b=b->link) {
		w = b->loop > 1 ? 4 : 1;
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			for (n=0; n<2; n++) {
				r = i->arg[n];
				if (!zextuse(i, n) || rtype(r) != RTmp || isreg(r))
					continue;
				if (ie64_uwidth(fn, r, Kw) <= 32)
					continue;
				zc[r.val] += w;
			}
	}
	k = 0;
	for (t=Tmp0; t<nt; t++)
		if (zc[t] >= 2) {
			to[t] = newtmp("ie64z", Kw, fn);
			k++;
		}
	if (!k)
		return;
	for (b=fn->start; b; b=b->link) {
		/* Rewrite the uses that need the extension. */
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			for (n=0; n<2; n++)
				if (zextuse(i, n) && rtype(i->arg[n]) == RTmp
				&& i->arg[n].val < nt && !req(to[i->arg[n].val], R))
					i->arg[n] = to[i->arg[n].val];
		/* Insert the extensions after the definitions. */
		nins = b->nins;
		for (p=b->phi; p; p=p->link)
			if (rtype(p->to) == RTmp && p->to.val < nt
			&& !req(to[p->to.val], R))
				nins++;
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (rtype(i->to) == RTmp && i->to.val < nt
			&& !req(to[i->to.val], R))
				nins++;
		if (nins == b->nins)
			continue;
		ni = vnew(nins, sizeof ni[0], PFn);
		k = 0;
		for (p=b->phi; p; p=p->link)
			if (rtype(p->to) == RTmp && p->to.val < nt
			&& !req(to[p->to.val], R))
				ni[k++] = (Ins){.op = Oextuw, .cls = Kw,
					.to = to[p->to.val], .arg = {p->to, R}};
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			ni[k++] = *i;
			if (rtype(i->to) == RTmp && i->to.val < nt
			&& !req(to[i->to.val], R))
				ni[k++] = (Ins){.op = Oextuw, .cls = Kw,
					.to = to[i->to.val], .arg = {i->to, R}};
		}
		b->ins = ni;
		b->nins = nins;
	}
	ie64_extfacts(fn);
}
