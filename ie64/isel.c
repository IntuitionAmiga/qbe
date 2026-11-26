#include "all.h"

/* Keep tuple operations in SSA through GVN and GCM. Once both passes
 * finish, give each instruction its actual inputs and outputs. */
void
ie64_lower_fpext(Fn *fn)
{
	Blk *b;
	Ins *i, *pair;
	Ref *cos;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (i->op == Oie64fsincos || i->op == Oie64dsincos
			|| i->op == Oie64fcosresult || i->op == Oie64dcosresult || isie64fused(i->op))
				goto lower;
	return;

lower:
	cos = alloc(fn->ntmp * sizeof(*cos));
	filluse(fn);
	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (i->op == Oie64fmapair || i->op == Oie64dfmapair) {
				Tmp *tmp = &fn->tmp[i->to.val];
				int consumer = i->op == Oie64fmapair ? Oie64fma : Oie64dfma;
				for (Use *u=tmp->use; u<&tmp->use[tmp->nuse]; u++)
					if (u->type != UIns || u->u.ins->op != consumer
					|| !req(u->u.ins->arg[1], i->to) || req(u->u.ins->arg[0], i->to))
						die("IE64 fused operand pair used outside its matching fused operation");
			}
			if (i->op != Oie64fcosresult && i->op != Oie64dcosresult)
				continue;
			Ref from = i->arg[0];
			pair = rtype(from) == RTmp ? fn->tmp[from.val].def : 0;
			if (!pair || pair->op != (i->op == Oie64fcosresult ? Oie64fsincos : Oie64dsincos))
				die("IE64 cosine projection requires an FSINCOS or DSINCOS result");
			if (req(cos[from.val], R)) {
				cos[from.val] = i->to;
				*i = (Ins){.op = Onop};
			} else {
				i->op = Ocopy;
				i->arg[0] = cos[from.val];
			}
		}
	for (b=fn->start; b; b=b->link) {
		curi = &insb[NIns];
		for (i=&b->ins[b->nins]; i!=b->ins;) {
			--i;
			switch (i->op) {
			case Oie64fmapair:
			case Oie64dfmapair:
				break;
			case Oie64fma:
			case Oie64dfma: {
				Ins ins = *i;
				Ref from = ins.arg[1];
				pair = rtype(from) == RTmp ? fn->tmp[from.val].def : 0;
				int wide = ins.op == Oie64dfma;
				if (!pair || pair->op != (wide ? Oie64dfmapair : Oie64fmapair))
					die("IE64 fused operation requires a matching operand pair");
				ins.arg[1] = pair->arg[0];
				ins.arg[2] = pair->arg[1];
				emiti(ins);
				break;
			}
			case Oie64fsincos:
			case Oie64dsincos: {
				Ins ins = *i;
				Ref other = cos[i->to.val];

				ins.arg[2] = other;
				fn->tmp[i->to.val].fppair = req(other, R) ? 1 : (int)other.val;
				if (!req(other, R))
					fn->tmp[other.val].fppair = -(int)i->to.val;
				emiti(ins);
				break;
			}
			default:
				emiti(*i);
				break;
			}
		}
		/* Keep the old instruction storage until all tuple definitions
		 * have been used. idup can overwrite it. */
		b->ins = vnew(0, sizeof(b->ins[0]), PFn);
		idup(b, curi, &insb[NIns]-curi);
	}
}

static int
memarg(Ref *r, Ins *i)
{
	return (isload(i->op) && r == &i->arg[0])
		|| (isstore(i->op) && r == &i->arg[1]);
}

enum { NSlotmemo = 16 };

typedef struct Slotmemo Slotmemo;
struct Slotmemo {
	int n;
	int slot[NSlotmemo];
	Ref tmp[NSlotmemo];
};

/* Replace a reference to an allocation slot with a materialised address.
 * Every slot operand in a block is rewritten by the one scan below, after
 * selection has emitted the block, so all of these land at the block head and
 * uses of the same slot share a temporary. */
static void
slotarg(Ref *r, Slotmemo *m, Fn *fn)
{
	Ref t;
	int s, j;

	if (rtype(*r) != RTmp || isreg(*r))
		return;
	s = fn->tmp[r->val].slot;
	if (s == -1)
		return;
	for (j=0; j<m->n; j++)
		if (m->slot[j] == s) {
			*r = m->tmp[j];
			return;
		}
	t = newtmp("isel", Kl, fn);
	emit(Oaddr, Kl, t, SLOT(s), R);
	if (m->n < NSlotmemo) {
		m->slot[m->n] = s;
		m->tmp[m->n] = t;
		m->n++;
	}
	*r = t;
}

/* Is r a zero operand of an integer compare that the emitter turns into
 * a branch (or a branch and a constant), so that it can name r0? */
static int
iszerocmp(Ins *i, Ref *r, Fn *fn)
{
	Con *c;
	int ck, cc;

	return iscmp(i->op, &ck, &cc) && KBASE(ck) == 0
		&& (r == &i->arg[0] || r == &i->arg[1])
		&& rtype(*r) == RCon
		&& (c = &fn->con[r->val])->type == CBits && !c->flt
		&& c->bits.i == 0;
}

/* The compare-and-branch with an immediate.  Given the condition of a
 * compare, whether its constant is the left operand, and the constant
 * as the 64-bit compare sees it, find the branch mnemonic and the signed
 * 16-bit immediate that make the same decision on the other operand.
 * Unsigned >= and < have no immediate form: N-1 with bhi or bls does the
 * same, unless N is zero.  The result does not change when the branch is
 * inverted, because that keeps each condition in its family. */
int
ie64_brimm(int cc, int constleft, int64_t v, char **name, int64_t *imm)
{
	static char *nm[] = {
		[Cieq] = "beq", [Cine] = "bne",
		[Cisge] = "bge", [Cisgt] = "bgt", [Cisle] = "ble", [Cislt] = "blt",
		[Ciugt] = "bhi", [Ciule] = "bls",
	};
	uint64_t u = v;

	if (constleft)
		switch (cc) {
		case Cisge: cc = Cisle; break;
		case Cisgt: cc = Cislt; break;
		case Cisle: cc = Cisge; break;
		case Cislt: cc = Cisgt; break;
		case Ciuge: cc = Ciule; break;
		case Ciugt: cc = Ciult; break;
		case Ciule: cc = Ciuge; break;
		case Ciult: cc = Ciugt; break;
		}
	if (cc == Ciuge || cc == Ciult) {
		if (u == 0)
			return 0;
		u--;
		cc = cc == Ciuge ? Ciugt : Ciule;
	}
	v = (int64_t)u;
	if (cc < 0 || cc >= (int)(sizeof nm / sizeof nm[0]) || !nm[cc])
		return 0;
	if (v < -32768 || v > 32767)
		return 0;
	*name = nm[cc];
	*imm = v;
	return 1;
}

/* Does the integer compare i feed the conditional jump of block b? */
static int
brimmfeeds(Blk *b, Ins *i)
{
	return b->jmp.type == Jjnz && req(i->to, b->jmp.arg);
}

/* A numeric constant, not zero (zero names r0)? */
static int
brimmcon(Ref r, int64_t *v, Fn *fn)
{
	Con *c;

	if (rtype(r) != RCon || (c = &fn->con[r.val])->type != CBits || c->flt
	|| c->bits.i == 0)
		return 0;
	*v = c->bits.i;
	return 1;
}

static void
fixarg(Ref *r, int k, Ins *i, Fn *fn)
{
	Ref t;
	int ck, cc;

	if (rtype(*r) != RCon || memarg(r, i))
		return;
	if (isie64immop(i->op) && r == &i->arg[0])
		return;
	if (i->op == Osalloc)
		return;
	/* A compare against zero reads r0: no load needed. */
	if (iszerocmp(i, r, fn))
		return;
	/* Keep direct call targets symbolic for the IE64 JSR form. */
	if (i->op == Ocall && r == &i->arg[0]
	&& fn->con[r->val].type == CAddr)
		return;
	/* IE64 has immediate forms only for the right arithmetic operand. */
	if (!isie64op(i->op) && r == &i->arg[1] && fn->con[r->val].type == CBits
	&& KBASE(k) == 0 && !iscmp(i->op, &ck, &cc)) {
		uint64_t bits = (uint64_t)fn->con[r->val].bits.i;
		if (bits <= UINT32_MAX)
			return;
	}
	t = newtmp("isel", k, fn);
	emit(Ocopy, k, t, *r, R);
	*r = t;
}

#define extisnop ie64_extisnop

typedef struct Cmparg Cmparg;
struct Cmparg {
	Ref use;	/* operand of the 64-bit compare */
	Ref orig;	/* operand of the w compare */
	Ref con;	/* extended constant, when orig is one */
	Ref cpy;	/* w temporary a non-numeric constant is copied to */
	int how;	/* 0 use as is, 1 numeric constant, 2 extend */
};

/* Decide how a w compare operand reaches 64 bits.  A numeric constant is
 * extended at compile time.  A temporary is used as is when its known
 * width already has the extension the compare needs; the zero-extension
 * of w values by the emitter makes this the usual case. */
static void
cmpplan(Ref r, int sign, int forcmp, Cmparg *a, Fn *fn)
{
	Con *c;
	int64_t v;

	a->orig = a->use = r;
	a->cpy = a->con = R;
	a->how = 0;
	if (rtype(r) == RCon && (c = &fn->con[r.val])->type == CBits && !c->flt) {
		v = c->bits.i;
		v = sign ? (int64_t)(int32_t)v : (int64_t)(uint32_t)v;
		/* A compare against zero reads r0: the constant stays an
		 * operand and the emitter names the register. */
		if (v == 0 && forcmp) {
			a->use = getcon(0, fn);
			return;
		}
		/* HOOK(branch-immediate): when IE64 gains a compare-and-branch
		 * with an immediate, keep this constant as an RCon operand of a
		 * compare that feeds a branch (v is the extended value, in
		 * int32 range or not) instead of loading it into a register,
		 * and let emitbranch() in emit.c print the immediate form. */
		a->how = 1;
		a->con = getcon(v, fn);
		a->use = newtmp("ie64cmp", Kl, fn);
		return;
	}
	if (rtype(r) == RTmp && !isreg(r)
	&& (sign ? ie64_swidth(fn, r, Kw) <= 32 : ie64_uwidth(fn, r, Kw) <= 32))
		return;
	a->how = 2;
	a->use = newtmp("ie64cmp", Kl, fn);
	if (rtype(r) == RCon)
		a->cpy = newtmp("ie64cmp", Kw, fn);
}

static int
exactlongtodouble(Ref r, Blk *b, Fn *fn)
{
	Ins *d;

	if (ie64_swidth(fn, r, Kl) <= 54)
		return 1;
	if (rtype(r) != RTmp || isreg(r) || fn->tmp[r.val].bid != b->id
	|| !(d = fn->tmp[r.val].def))
		return 0;
	/* Truncating a binary32 or binary64 cannot add significant bits.
	 * A saturated INT64_MAX rounds to 2^63 in both widths. */
	return d->cls == Kl && (d->op == Ostosi || d->op == Odtosi);
}

static void
cmpemit(Cmparg *a, int sign, Fn *fn)
{
	(void)fn;
	if (a->how == 1) {
		emit(Ocopy, Kl, a->use, a->con, R);
	} else if (a->how == 2) {
		emit(sign ? Oextsw : Oextuw, Kl, a->use,
			req(a->cpy, R) ? a->orig : a->cpy, R);
		if (!req(a->cpy, R))
			emit(Ocopy, Kw, a->cpy, a->orig, R);
	}
}

static int
integerconst(Ref r, int64_t *v, Fn *fn)
{
	Con *c;

	if (rtype(r) != RCon || (c = &fn->con[r.val])->type != CBits || c->flt)
		return 0;
	*v = c->bits.i;
	return 1;
}

static Ins *
selectdef(Ref r, int op, int k, int single, Fn *fn)
{
	Tmp *t;
	Ins *d;

	if (rtype(r) != RTmp || isreg(r))
		return 0;
	t = &fn->tmp[r.val];
	d = t->def;
	return t->ndef == 1 && (!single || t->nuse == 1) && d
		&& d->op == op && d->cls == k ? d : 0;
}

static Ins *
negatedcount(Ref r, Ref positive, Fn *fn)
{
	Ins *d;
	int64_t zero;

	d = selectdef(r, Oneg, Kw, 0, fn);
	if (d && req(d->arg[0], positive))
		return d;
	d = selectdef(r, Osub, Kw, 0, fn);
	if (d && req(d->arg[1], positive)
	&& integerconst(d->arg[0], &zero, fn) && zero == 0)
		return d;
	return 0;
}

static void
rotate(Ins *i, Fn *fn)
{
	Ins *d[2], *mask[2], *neg;
	Ref left, right, n;
	int dir, width, op;
	int64_t a, z;

	if (i->op != Oor || KBASE(i->cls) != 0)
		return;
	for (dir=0; dir<2; dir++) {
		d[0] = selectdef(i->arg[dir], Oshl, i->cls, 1, fn);
		d[1] = selectdef(i->arg[1-dir], Oshr, i->cls, 1, fn);
		if (d[0] && d[1])
			break;
	}
	if (dir == 2 || !req(d[0]->arg[0], d[1]->arg[0]))
		return;
	left = d[0]->arg[1];
	right = d[1]->arg[1];
	width = i->cls == Kw ? 32 : 64;
	op = Oie64rol;
	if (integerconst(left, &a, fn) && integerconst(right, &z, fn)) {
		if (a <= 0 || a >= width || z != width - a)
			return;
		n = left;
	} else {
		mask[0] = selectdef(left, Oand, Kw, 1, fn);
		mask[1] = selectdef(right, Oand, Kw, 1, fn);
		if (!mask[0] || !mask[1]
		|| !integerconst(mask[0]->arg[1], &a, fn) || a != width - 1
		|| !integerconst(mask[1]->arg[1], &z, fn) || z != width - 1)
			return;
		left = mask[0]->arg[0];
		right = mask[1]->arg[0];
		neg = negatedcount(right, left, fn);
		n = left;
		if (!neg) {
			neg = negatedcount(left, right, fn);
			if (!neg)
				return;
			op = Oie64ror;
			n = right;
		}
		if (fn->tmp[neg->to.val].nuse == 1)
			*neg = (Ins){.op = Onop};
		*mask[0] = *mask[1] = (Ins){.op = Onop};
	}
	i->op = op;
	i->arg[0] = d[0]->arg[0];
	i->arg[1] = n;
	*d[0] = *d[1] = (Ins){.op = Onop};
}

static Ins *
shiftdef(Ref r, int op, int k, int64_t *n, Fn *fn)
{
	Ins *d = selectdef(r, op, k, 1, fn);

	return d && integerconst(d->arg[1], n, fn)
		&& *n > 0 && *n < (k == Kw ? 32 : 64) ? d : 0;
}

/* Combine a packed high-field XOR with the retained low field. */
static int
fieldxor(Ins *i, int h, uint64_t full, Fn *fn)
{
	Ins *lo, *mask, *sh, *x, *hi;
	Ref field;
	int k, a;
	int64_t n, z, lm;
	uint64_t hm;

	k = i->cls;
	lo = selectdef(i->arg[1-h], Oand, k, 1, fn);
	if (!lo || !integerconst(lo->arg[1], &lm, fn))
		return 0;
	field = i->arg[h];
	mask = selectdef(field, Oand, k, 1, fn);
	sh = shiftdef(mask ? mask->arg[0] : field, Oshl, k, &n, fn);
	if (!sh)
		return 0;
	hm = (full << n) & full;
	if (((uint64_t)lm & full) != (full ^ hm)
	|| (mask && (!integerconst(mask->arg[1], &z, fn) || ((uint64_t)z & full) != hm)))
		return 0;
	x = selectdef(sh->arg[0], Oxor, k, 1, fn);
	if (!x)
		return 0;
	for (a=0; a<2; a++) {
		hi = shiftdef(x->arg[a], Oshr, k, &z, fn);
		if (hi && z == n && req(hi->arg[0], lo->arg[0]))
			break;
	}
	if (a == 2)
		return 0;
	sh->arg[0] = x->arg[1-a];
	i->op = Oxor;
	i->arg[0] = lo->arg[0];
	i->arg[1] = sh->to;
	*lo = *x = *hi = (Ins){.op = Onop};
	if (mask)
		*mask = (Ins){.op = Onop};
	return 1;
}

static void
bitfield(Ins *i, Fn *fn)
{
	Ins *sh, *sub, *bound;
	int k;
	int64_t n, z, bias;
	uint64_t full, mask, low;

	k = i->cls;
	if (KBASE(k) != 0)
		return;
	full = k == Kw ? UINT32_MAX : UINT64_MAX;
	if (i->op == Oor) {
		if (!fieldxor(i, 0, full, fn))
			fieldxor(i, 1, full, fn);
		return;
	}
	if (!integerconst(i->arg[1], &z, fn))
		return;
	if (i->op == Oshr && (sh = shiftdef(i->arg[0], Oshl, k, &n, fn)) && z == n) {
		i->op = Oand;
		i->arg[0] = sh->arg[0];
		i->arg[1] = getcon(full >> n, fn);
		*sh = (Ins){.op = Onop};
		return;
	}
	if (i->op == Oshl && z > 0 && z < (k == Kw ? 32 : 64)) {
		bound = selectdef(i->arg[0], Oand, k, 1, fn);
		if (bound && integerconst(bound->arg[1], &n, fn)
		&& ((uint64_t)n & (full >> z)) == (full >> z)) {
			i->arg[0] = bound->arg[0];
			*bound = (Ins){.op = Onop};
		}
		return;
	}
	if (i->op != Oand || !(sh = shiftdef(i->arg[0], Oshl, k, &n, fn)))
		return;
	mask = (uint64_t)z & full;
	if ((mask | ((UINT64_C(1) << n) - 1)) != full) {
		/* Reduce the signed bias modulo the field width. The input
		 * mask makes subtracting half the range equivalent to toggling
		 * that bit, without a borrow into any adjacent field. */
		sub = selectdef(sh->arg[0], Osub, k, 1, fn);
		if (!sub || !integerconst(sub->arg[1], &bias, fn) || bias <= 0
		|| ((uint64_t)bias & ((uint64_t)bias - 1)))
			return;
		bound = selectdef(sub->arg[0], Oand, k, 0, fn);
		low = 2 * (uint64_t)bias - 1;
		if (!bound || !integerconst(bound->arg[1], &z, fn) || ((uint64_t)z & full) != low
		|| low > (full >> n) || mask != (low << n))
			return;
		sub->op = Oxor;
	}
	i->op = Ocopy;
	i->arg[1] = R;
}

/* Give each strict relation in greater-than order, so swapped operands
 * and a reversed opcode are equivalent. */
static int
strictcmp(Ref r, Ref greater[2], Ins **d, int *k, int *sign, Fn *fn)
{
	Tmp *t;
	int cc, swap;

	if (rtype(r) != RTmp || isreg(r))
		return 0;
	t = &fn->tmp[r.val];
	if (t->ndef != 1 || t->nuse != 1 || !(*d = t->def)
	|| !iscmp((*d)->op, k, &cc) || KBASE(*k) != 0)
		return 0;
	switch (cc) {
	case Cisgt: case Cislt: *sign = 1; break;
	case Ciugt: case Ciult: *sign = 0; break;
	default: return 0;
	}
	swap = cc == Cislt || cc == Ciult;
	greater[0] = (*d)->arg[swap];
	greater[1] = (*d)->arg[1-swap];
	return 1;
}

static void
cmp3(Ins *i, Fn *fn)
{
	Ins *a, *b;
	Ref gt[2], lt[2];
	int k, l, sign, other;

	if (i->op != Osub || KBASE(i->cls) != 0
	|| !strictcmp(i->arg[0], gt, &a, &k, &sign, fn)
	|| !strictcmp(i->arg[1], lt, &b, &l, &other, fn)
	|| k != l || sign != other || !req(gt[0], lt[1]) || !req(gt[1], lt[0]))
		return;
	i->op = (sign ? Oie64cmp3sw : Oie64cmp3uw) + (k == Kl);
	/* Preserve the sign through spills and when extending the result. */
	i->cls = Kl;
	i->arg[0] = gt[0];
	i->arg[1] = gt[1];
	*a = *b = (Ins){.op = Onop};
}

static int
fpwritecc(Ins *i)
{
	if (KBASE(i->cls) != 1)
		return 0;
	return i->op == Oadd || i->op == Osub || i->op == Omul || i->op == Odiv
		|| i->op == Ocast;
}

static int
fpobservecc(Ins *i)
{
	/* Target-specific ops and calls can expose the current status. */
	return isie64op(i->op) || iscallop(i->op);
}

/* FLOAD and DLOAD read the same bytes as the integer load and copy
 * their raw bits. Keep the read in place. Move the cast's condition
 * codes only over code that cannot observe them. Treat block exits
 * as observers because a successor can read the status. */
static void
floatloads(Fn *fn)
{
	Blk *b;
	Ins *i, *d, *barrier, *lastfp;
	Tmp *t;
	char *dead;
	uint n;
	int live, k, cc;

	filluse(fn);
	dead = vnew(0, sizeof dead[0], PFn);
	for (b=fn->start; b; b=b->link) {
		vgrow(&dead, b->nins);
		live = 1;
		for (n=b->nins; n>0;) {
			i = &b->ins[--n];
			dead[n] = !live;
			if (fpwritecc(i))
				live = 0;
			if (fpobservecc(i))
				live = 1;
		}

		barrier = lastfp = 0;
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (i->op == Ocast && KBASE(i->cls) == 1
			&& rtype(i->arg[0]) == RTmp && !isreg(i->arg[0])) {
				t = &fn->tmp[i->arg[0].val];
				d = t->def;
				if (t->bid == b->id && t->nuse == 1 && d && d < i
				&& (!barrier || d > barrier)
				&& (dead[i-b->ins] || !lastfp || d > lastfp)
				&& ((i->cls == Ks && d->cls == Kw
				    && (d->op == Oload || d->op == Oloadsw || d->op == Oloaduw))
				 || (i->cls == Kd && d->cls == Kl && d->op == Oload))) {
					d->op = Oload;
					d->cls = i->cls;
					d->to = i->to;
					*i = (Ins){.op = Onop};
					lastfp = i;
					continue;
				}
			}
			if (fpobservecc(i))
				barrier = i;
			if (KBASE(i->cls) == 1 || (iscmp(i->op, &k, &cc) && KBASE(k) == 1))
				lastfp = i;
		}
	}
}

/* Fold the stores of successive packed-field updates if each writes the
 * same bytes and the intervening integer operations cannot observe or
 * interrupt the write. Volatile accesses and other effects end the scan. */
static void
packedstores(Fn *fn)
{
	Ins *i, *store;
	Blk *b;

	for (b=fn->start; b; b=b->link) {
		store = 0;
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (i->op >= Ostoreb && i->op <= Ostorel) {
				if (store && store->op == i->op
				&& req(store->arg[1], i->arg[1]))
					*store = (Ins){.op = Onop};
				store = i;
				continue;
			}
			if (KBASE(i->cls) != 0) {
				store = 0;
				continue;
			}
			switch (i->op) {
			case Onop: case Ocopy: case Oadd: case Osub:
			case Oand: case Oor: case Oxor:
			case Oshl: case Oshr: case Osar:
				break;
			default:
				store = 0;
				break;
			}
		}
	}
}

extern int optlevel;

void
ie64_isel(Fn *fn)
{
	Blk *b, **sb;
	Ins *i, *ii, *sel0;
	Phi *p;
	Ref t;
	int64_t n;
	Slotmemo memo;
	uint pa;
	int a, ck, cc, pn, npend, pendsign;
	Cmparg pend[2];
	Con *c;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (!isie64immop(i->op))
				continue;
			if (rtype(i->arg[0]) != RCon
			|| (c = &fn->con[i->arg[0].val])->type != CBits)
				die("IE64 immediate operand must be an integer constant");
			n = c->bits.i;
			if ((i->op == Oie64mfcr || i->op == Oie64mtcr
			|| i->op == Oie64fmovecr || i->op == Oie64dmovecr)
			&& (n < 0 || n > 15))
				die("IE64 immediate operand is outside the supported range");
			if ((i->op == Oie64wait || i->op == Oie64syscall)
			&& (n < 0 || (uint64_t)n > UINT32_MAX))
				die("IE64 immediate operand is outside the supported range");
		}

	ie64_rangecmp(fn);
	filluse(fn);
	for (b=fn->start; b; b=b->link)
		for (i=&b->ins[b->nins]; i!=b->ins;) {
			--i;
			rotate(i, fn);
			bitfield(i, fn);
			cmp3(i, fn);
		}
	floatloads(fn);
	if (optlevel >= 2)
		packedstores(fn);
	ie64_canon(fn);

	/* ABI lowering uses Osalloc around calls with stack arguments.  Keep a
	 * stable frame base whenever those adjustments can coexist with slots. */
	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (i->op == Osalloc)
				fn->dynalloc = 1;

	/* Assign fixed local slots before turning allocation references into LEAs.
	 * ABI lowering may materialise aggregate copies at individual call sites. */
	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (isalloc(i->op) && rtype(i->arg[0]) == RCon
			&& fn->con[i->arg[0].val].type == CBits) {
				n = fn->con[i->arg[0].val].bits.i;
				if (n < 0 || n > INT_MAX - 15)
					die("invalid IE64 allocation size");
			n = (n + 3) & -4;
			a = i->op == Oalloc16 ? 4 : i->op == Oalloc8 ? 2 : 1;
			fn->slot = (fn->slot + a - 1) & -a;
			fn->tmp[i->to.val].slot = fn->slot;
				fn->slot += n / 4;
				/* Clear the whole instruction: a leftover destination keeps
				 * the alloc temporary live, and spill() then reuses the slot
				 * the allocation itself just claimed. */
				*i = (Ins){.op = Onop};
			}

	for (b=fn->start; b; b=b->link) {
		/* A w result is returned sign-extended.  A value known to be
		 * non-negative is the same register either way. */
		if (b->jmp.type == Jretw && rtype(b->jmp.arg) == RTmp
		&& !isreg(b->jmp.arg) && ie64_swidth(fn, b->jmp.arg, Kw) <= 32)
			b->jmp.type = Jretl;
		curi = &insb[NIns];
		for (i=&b->ins[b->nins]; i!=b->ins;) {
			--i;
			if (isalloc(i->op)) {
				salloc(i->to, i->arg[0], fn);
				continue;
			}
			if (i->op == Oexts) {
				emit(Oie64fcvtsd, Kd, i->to, i->arg[0], R);
				continue;
			}
			if (i->op == Otruncd) {
				emit(Oie64fcvtds, Ks, i->to, i->arg[0], R);
				continue;
			}
			if (i->op == Ostosi && i->cls == Kl) {
				t = newtmp("ie64conv", Kd, fn);
				emit(Odtosi, Kl, i->to, t, R);
				emit(Oie64fcvtsd, Kd, t, i->arg[0], R);
				continue;
			}
			if (i->op == Osltof && i->cls == Ks) {
				t = newtmp("ie64conv", Kd, fn);
				emit(Oie64fcvtds, Ks, i->to, t, R);
				emit(exactlongtodouble(i->arg[0], b, fn) ? Osltof : Oie64sltofprep,
					Kd, t, i->arg[0], R);
				fixarg(&curi->arg[0], Kl, curi, fn);
				continue;
			}
			if (i->op == Ouwtof) {
				Ref wide = newtmp("ie64conv", Kl, fn);

				if (i->cls == Ks) {
					Ref d = newtmp("ie64conv", Kd, fn);
					emit(Oie64fcvtds, Ks, i->to, d, R);
					emit(Osltof, Kd, d, wide, R);
				} else
					emit(Osltof, Kd, i->to, wide, R);
				emit(Oextuw, Kl, wide, i->arg[0], R);
				continue;
			}
			if (i->op == Oultof && i->cls == Ks) {
				t = newtmp("ie64conv", Kd, fn);
				emit(Oie64fcvtds, Ks, i->to, t, R);
				emit(Oie64ultofprep, Kd, t, i->arg[0], R);
				fixarg(&curi->arg[0], Kl, curi, fn);
				continue;
			}
			if (i->op == Oie64cmp3sw || i->op == Oie64cmp3uw) {
				int sign = i->op == Oie64cmp3sw;
				cmpplan(i->arg[0], sign, 1, &pend[0], fn);
				cmpplan(i->arg[1], sign, 1, &pend[1], fn);
				emit(i->op + 1, Kl, i->to, pend[0].use, pend[1].use);
				cmpemit(&pend[1], sign, fn);
				cmpemit(&pend[0], sign, fn);
				continue;
			}
			if (iscmp(i->op, &ck, &cc) && ck == Kw) {
				Cmparg ca[2];
				int sign = cc >= Cisge && cc <= Cislt;

				/* The compare runs on 64 bits: bring both w operands to
				 * the same extension first. */
				cmpplan(i->arg[0], sign, 1, &ca[0], fn);
				cmpplan(i->arg[1], sign, 1, &ca[1], fn);
				/* A compare that feeds a branch keeps a constant that
				 * fits the immediate form as an operand. */
				if (ie64_branch_imm && brimmfeeds(b, i)) {
					char *bn;
					int64_t imm;

					for (pn=0; pn<2; pn++)
						if (ca[pn].how == 1
						&& rtype(ca[1-pn].orig) != RCon
						&& ie64_brimm(cc, pn == 0,
							fn->con[ca[pn].con.val].bits.i, &bn, &imm)) {
							ca[pn].use = ca[pn].con;
							ca[pn].how = 0;
							break;
						}
				}
				i->arg[0] = ca[0].use;
				i->arg[1] = ca[1].use;
				i->op += Ocmpl - Ocmpw;
				emiti(*i);
				cmpemit(&ca[1], sign, fn);
				cmpemit(&ca[0], sign, fn);
				continue;
			}
			if (isext(i->op) && extisnop(i, fn)) {
				/* The operand already has the extension: a plain copy,
				 * which register allocation usually coalesces. */
				emit(Ocopy, i->cls, i->to, i->arg[0], R);
				continue;
			}
			if (i->op == Ostoui || i->op == Odtoui) {
				Ref x = i->arg[0];
				Ref converted;

				if (i->op == Ostoui) {
					x = newtmp("ie64conv", Kd, fn);
				}
				if (i->cls == Kw) {
					converted = newtmp("ie64conv", Kl, fn);
					emit(Ocopy, Kw, i->to, converted, R);
					emit(Odtosi, Kl, converted, x, R);
				} else {
					Ref threshold = newtmp("ie64conv", Kd, fn);
					Con thresholdbits = {.type = CBits, .flt = 2};
					Ref thresholdcon;
					Ref highw = newtmp("ie64conv", Kw, fn);
					Ref high = newtmp("ie64conv", Kl, fn);
					Ref highfp = newtmp("ie64conv", Kd, fn);
					Ref offset = newtmp("ie64conv", Kd, fn);
					Ref adjusted = newtmp("ie64conv", Kd, fn);
					Ref low = newtmp("ie64conv", Kl, fn);
					Ref highbit = newtmp("ie64conv", Kl, fn);

					emit(Oor, Kl, i->to, low, highbit);
					thresholdbits.bits.d = 0x1p63;
					thresholdcon = newcon(&thresholdbits, fn);
					emit(Oshl, Kl, highbit, high, getcon(63, fn));
					emit(Odtosi, Kl, low, adjusted, R);
					emit(Osub, Kd, adjusted, x, offset);
					emit(Omul, Kd, offset, highfp, threshold);
					emit(Osltof, Kd, highfp, high, R);
					emit(Oextuw, Kl, high, highw, R);
					emit(Ocmpd + Cfge, Kw, highw, x, threshold);
					emit(Ocopy, Kd, threshold, thresholdcon, R);
				}
				if (i->op == Ostoui)
					emit(Oie64fcvtsd, Kd, x, i->arg[0], R);
				continue;
			}
			/* Several w operations read more than the low 32 bits of a
			 * register: the 32-bit shift right and unsigned division
			 * take the register as it is, DIVS and the integer to
			 * double conversion take all 64 bits as signed.  Bring the
			 * operands to the extension they need. */
			npend = 0;
			if (i->cls == Kw || (i->op == Oswtof && i->cls == Kd)) {
				int first = 0, last = -1;

				if (i->op == Oshr || i->op == Oudiv || i->op == Ourem)
					last = i->op == Oshr ? 0 : 1;
				else if (i->op == Odiv)
					last = 1;
				else if (i->op == Oswtof)
					last = 0;
				pendsign = i->op == Odiv || i->op == Oswtof;
				for (pn=first; pn<=last; pn++) {
					cmpplan(i->arg[pn], pendsign, 0, &pend[npend], fn);
					if (pend[npend].how == 0)
						continue;
					i->arg[pn] = pend[npend].use;
					npend++;
				}
			}
			/* A multiplication by a power of two is a shift. */
			if (i->op == Omul && KBASE(i->cls) == 0
			&& rtype(i->arg[1]) == RCon
			&& (c = &fn->con[i->arg[1].val])->type == CBits && !c->flt
			&& c->bits.i > 1 && (c->bits.i & (c->bits.i - 1)) == 0
			&& (i->cls == Kl || c->bits.i <= INT32_MAX)) {
				for (n=0; (INT64_C(1) << n) != c->bits.i; n++)
					;
				i->op = Oshl;
				i->arg[1] = getcon(n, fn);
			}
			/* IE64 immediates are unsigned: x + (-k) is x - k. */
			if (i->op == Oadd && i->cls == Kl
			&& rtype(i->arg[1]) == RCon
			&& (c = &fn->con[i->arg[1].val])->type == CBits && !c->flt
			&& c->bits.i < 0 && c->bits.i >= -(int64_t)UINT32_MAX) {
				i->op = Osub;
				i->arg[1] = getcon(-c->bits.i, fn);
			}
			emiti(*i);
			ii = curi;
			if (ii->op != Onop && ii->op != Ocopy) {
				int keep = -1;

				if (ie64_branch_imm && iscmp(ii->op, &ck, &cc)
				&& KBASE(ck) == 0 && brimmfeeds(b, ii))
					for (pn=0; pn<2; pn++)
						if (brimmcon(ii->arg[pn], &n, fn)
						&& rtype(ii->arg[1-pn]) != RCon) {
							char *bn;
							int64_t imm;

							if (ie64_brimm(cc, pn == 0, n, &bn, &imm))
								keep = pn;
							break;
						}
				if (keep != 0)
					fixarg(&ii->arg[0], argcls(ii, 0), ii, fn);
				if (keep != 1)
					fixarg(&ii->arg[1], argcls(ii, 1), ii, fn);
				if (isie64fmavalue(ii->op))
					fixarg(&ii->arg[2], ii->cls, ii, fn);
			}
			for (pn=0; pn<npend; pn++)
				cmpemit(&pend[pn], pendsign, fn);
		}
		/* Materialise every allocation slot the block names -- instruction
		 * operands, successor phi arguments and the jump argument alike -- at
		 * the head of the block.  The address depends only on the frame
		 * pointer, so it is valid there, it dominates every use below, and one
		 * temporary serves all uses of a slot.  Emitting it inside the block
		 * would risk landing after a fused compare, whose operand registers
		 * emit reads again at the branch. */
		sel0 = curi;
		memo.n = 0;
		for (sb=(Blk*[3]){b->s1, b->s2, 0}; *sb; sb++)
			for (p=(*sb)->phi; p; p=p->link) {
				for (pa=0; p->blk[pa] != b; pa++)
					assert(pa+1 < p->narg);
				slotarg(&p->arg[pa], &memo, fn);
			}
		slotarg(&b->jmp.arg, &memo, fn);
		for (ii=sel0; ii<&insb[NIns]; ii++) {
			slotarg(&ii->arg[0], &memo, fn);
			slotarg(&ii->arg[1], &memo, fn);
			if (isie64fmavalue(ii->op))
				slotarg(&ii->arg[2], &memo, fn);
		}
		idup(b, curi, &insb[NIns]-curi);
	}
	if (debug['I']) {
		fprintf(stderr, "\n> After instruction selection:\n");
		printfn(fn, stderr);
	}
}
