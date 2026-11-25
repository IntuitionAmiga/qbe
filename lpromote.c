#include "all.h"

/*
 * Promotion of a loop's memory accumulator to a register.
 *
 * The pass looks for a loop of two blocks, a test block H and a body B
 * that jumps back to it, whose body loads a memory location, changes it and
 * stores it again on every iteration:
 *
 *	C[i*N+j] += A[i*N+k] * B[k*N+j];
 *
 * The location lives in a temporary instead.  The load before the loop
 * moves out of it, the stores leave it, and one store follows the loop:
 *
 *	@pre	... %acc0 ...
 *	@H	%acc =w phi @pre %acc0, @B %acc1
 *	@B	... uses %acc, defines %acc1 ...
 *	@exit	storew %acc, %addr
 *
 * Every other memory access must have a provably disjoint byte range or
 * an explicit incompatible C alias class. Access width alone proves
 * nothing about overlap. Volatile accesses, calls and private operations
 * stop the pass. The pass runs with -fstrict-aliasing.
 *
 * The loads and stores move across the test of the loop, so the address
 * must be valid when the loop runs no iteration: a symbol, or an address
 * that the block before the loop already accesses.
 *
 * Runs after GCM.  The address is then defined outside the loop if it
 * does not depend on the loop.
 */

int strictalias;

/* An address is a base and a constant offset.  GCM copies an add of a
 * constant to each memory access that uses it, so the accesses of one
 * location do not share a temporary. */
typedef struct Loc Loc;
struct Loc {
	Ref base;
	uint64_t off;
};

static Loc
resolve(Fn *fn, Ref r)
{
	Loc l;
	Tmp *t;
	Ins *d;
	Con *c;
	int n;

	l.base = r;
	l.off = 0;
	for (n=0; n<4 && rtype(l.base)==RTmp; n++) {
		t = &fn->tmp[l.base.val];
		d = t->def;
		if (!d || (d->op != Oadd && d->op != Osub) || d->cls != Kl
		|| rtype(d->arg[0]) != RTmp || rtype(d->arg[1]) != RCon)
			break;
		c = &fn->con[d->arg[1].val];
		if (c->type != CBits || c->flt)
			break;
		l.off += d->op == Oadd ? (uint64_t)c->bits.i : -(uint64_t)c->bits.i;
		l.base = d->arg[0];
	}
	return l;
}

static int
sameloc(Loc a, Loc b)
{
	return req(a.base, b.base) && a.off == b.off;
}

/* The address operand of a load or a store. */
static Ref
addrof(Ins *i)
{
	return isload(i->op) ? i->arg[0] : i->arg[1];
}

static int
inloop(Fn *fn, Ref r, Blk *h, Blk *b)
{
	Tmp *t;

	if (rtype(r) != RTmp)
		return 0;
	t = &fn->tmp[r.val];
	return t->bid == h->id || t->bid == b->id;
}

/* Only compare byte ranges relative to a common base.  Unsigned
 * distances also account for offsets on either side of the base. */
static int
overlaps(Loc a, int sz, Loc b, int n)
{
	return !req(a.base, b.base)
		|| b.off - a.off < (uint64_t)sz
		|| a.off - b.off < (uint64_t)n;
}

/* May the loop run with the location in a register?  Its loads and stores
 * of the location must be of one width and in the body, and nothing else in
 * the loop may reach memory in a way that overlaps. */
static int
loopok(Fn *fn, Blk *h, Blk *b, Loc a, int sz, int k, int mt)
{
	Blk *x;
	Ins *i;
	Loc other;
	int nld, nst;

	nld = nst = 0;
	for (x=h; x; x=(x == h ? b : 0))
		for (i=x->ins; i<&x->ins[x->nins]; i++) {
			if (i->op == Onop || i->op == Odbgloc)
				continue;
			if (!pinned(i))
				continue;
			if (!isload(i->op) && !isstore(i->op))
				return 0;
			if (isvload(i->op) || isvstore(i->op))
				return 0;
			other = resolve(fn, addrof(i));
			if (sameloc(other, a)) {
				/* An access of the location itself. */
				if (x != b || (mt && i->mtype != mt))
					return 0;
				if (isload(i->op)) {
					if (i->cls != k)
						return 0;
					if (i->op != Oload && !(k == Kw
					&& (i->op == Oloadsw || i->op == Oloaduw)))
						return 0;
					nld++;
				} else {
					if (i->op != (k == Kw ? Ostorew : Ostorel))
						return 0;
					/* The address itself must not be the value. */
					if (req(i->arg[0], a.base))
						return 0;
					nst++;
				}
				continue;
			}
			/* A store to allocated storage can change its effective
			 * type. A different store class cannot prove disjointness. */
			if (mt && isload(i->op) && i->mtype && mt != i->mtype)
				continue;
			if (overlaps(a, sz, other,
				isload(i->op) ? loadsz(i) : storesz(i)))
				return 0;
		}
	return nst > 0 && nld + nst > 0;
}

/* Does the block access the address? */
static int
accesses(Fn *fn, Blk *b, Loc a)
{
	Ins *i;

	for (i=b->ins; i<&b->ins[b->nins]; i++)
		if ((isload(i->op) && !isvload(i->op))
		|| (isstore(i->op) && !isvstore(i->op)))
			if (sameloc(resolve(fn, addrof(i)), a))
				return 1;
	return 0;
}

static void
insert(Blk *b, Ins *v, uint nv, int atend)
{
	Ins *n;

	n = alloc((b->nins + nv) * sizeof n[0]);
	if (atend) {
		icpy(n, b->ins, b->nins);
		icpy(n + b->nins, v, nv);
	} else {
		icpy(n, v, nv);
		icpy(n + nv, b->ins, b->nins);
	}
	idup(b, n, b->nins + nv);
}

/* Put an access of the location, after an add of the offset if it has one,
 * at the start or the end of a block outside the loop.  The code generator
 * folds the add into the access.  Returns the address operand slot. */
static void
access(Fn *fn, Blk *b, int atend, Loc l, Ins i, int isloadins)
{
	Ins v[2];
	uint nv;
	Ref ad;

	nv = 0;
	ad = l.base;
	if (l.off != 0) {
		ad = newtmp("pa", Kl, fn);
		v[nv++] = (Ins){.op = Oadd, .cls = Kl, .to = ad,
			.arg = {l.base, getcon(l.off, fn)}};
	}
	if (isloadins)
		i.arg[0] = ad;
	else
		i.arg[1] = ad;
	v[nv++] = i;
	insert(b, v, nv, atend);
}

/* The value the block leaves in the location, if its last write to
 * memory is a store to it; R otherwise. */
static Ref
lastvalue(Fn *fn, Blk *p, Loc a, int sz, int k, int mt)
{
	Ins *i;

	for (i=&p->ins[p->nins]; i-- > p->ins;) {
		if (i->op == Onop || !pinned(i))
			continue;
		if (isload(i->op) && !isvload(i->op))
			continue;
		if (isstore(i->op) && !isvstore(i->op)
		&& i->op == (k == Kw ? Ostorew : Ostorel)
		&& (mt == MUnknown || i->mtype == mt)
		&& storesz(i) == sz && sameloc(resolve(fn, i->arg[1]), a))
			return i->arg[0];
		return R;
	}
	return R;
}

static int
promote1(Fn *fn, Blk *h, Blk *b, Blk *p, Blk *x)
{
	Blk *e;
	Ins *i, st;
	Phi *ph, *xp;
	Ref init, acc, cur;
	Loc a;
	Con *c;
	int sz, k, ldop;
	uint nn;

	for (i=b->ins; i<&b->ins[b->nins]; i++) {
		if (!isstore(i->op) || isvstore(i->op))
			continue;
		if (i->op != Ostorew && i->op != Ostorel)
			continue;
		a = resolve(fn, i->arg[1]);
		k = i->op == Ostorew ? Kw : Kl;
		sz = storesz(i);
		if (rtype(a.base) == RTmp) {
			if (inloop(fn, a.base, h, b))
				continue;
			if (!accesses(fn, p, a))
				continue;
		} else if (rtype(a.base) == RCon) {
			c = &fn->con[a.base.val];
			if (c->type != CAddr || a.off != 0)
				continue;
		} else
			continue;
		if (loopok(fn, h, b, a, sz, k, MUnknown)
		|| (i->mtype && !req(lastvalue(fn, p, a, sz, k, i->mtype), R)
		    && loopok(fn, h, b, a, sz, k, i->mtype)))
			goto Found;
	}
	return 0;
Found:
	ldop = k == Kw ? Oloadsw : Oload;
	/* The value on entry. */
	init = lastvalue(fn, p, a, sz, k, MUnknown);
	if (req(init, R)) {
		init = newtmp("pl", k, fn);
		access(fn, p, 1, a, (Ins){.op = ldop, .cls = k, .to = init}, 1);
	}
	acc = newtmp("acc", k, fn);
	/* The value inside the body. */
	cur = acc;
	for (i=b->ins; i<&b->ins[b->nins]; i++) {
		if (i->op == Onop || !pinned(i))
			continue;
		if (!isload(i->op) && !isstore(i->op))
			continue;
		if (!sameloc(resolve(fn, addrof(i)), a))
			continue;
		if (isload(i->op))
			*i = (Ins){.op = Ocopy, .cls = k, .to = i->to,
				.arg = {cur, R}};
		else {
			cur = i->arg[0];
			*i = (Ins){.op = Onop};
		}
	}
	/* The phi of the test block. */
	ph = alloc(sizeof *ph);
	ph->cls = k;
	ph->to = acc;
	ph->narg = 2;
	ph->arg = vnew(2, sizeof ph->arg[0], PFn);
	ph->blk = vnew(2, sizeof ph->blk[0], PFn);
	ph->arg[0] = init;
	ph->blk[0] = p;
	ph->arg[1] = cur;
	ph->blk[1] = b;
	ph->link = h->phi;
	h->phi = ph;
	/* The store after the loop. */
	if (x->npred == 1)
		e = x;
	else {
		e = newblk();
		e->name = strf(PFn, "%s.pl", h->name);
		e->jmp.type = Jjmp;
		e->s1 = x;
		e->loop = h->loop;
		e->link = h->link;
		h->link = e;
		if (h->s1 == x)
			h->s1 = e;
		if (h->s2 == x)
			h->s2 = e;
		for (xp=x->phi; xp; xp=xp->link)
			for (nn=0; nn<xp->narg; nn++)
				if (xp->blk[nn] == h)
					xp->blk[nn] = e;
	}
	st = (Ins){.op = k == Kw ? Ostorew : Ostorel, .cls = Kw};
	st.arg[0] = acc;
	access(fn, e, 0, a, st, 0);
	return 1;
}

/* Recognise a guarded two-block loop. */
static int
loopblocks(Blk *h, Blk **body, Blk **pre, Blk **exit)
{
	Blk *b, *p, *x;

	if (h->jmp.type != Jjnz || !h->s1 || !h->s2
	|| h->s1 == h->s2 || h->npred != 2)
		return 0;
	b = h->s1;
	x = h->s2;
	if (b->npred != 1 || b->jmp.type != Jjmp || b->s1 != h) {
		b = h->s2;
		x = h->s1;
		if (b->npred != 1 || b->jmp.type != Jjmp || b->s1 != h)
			return 0;
	}
	if (b == h || x == h || x == b || b->phi)
		return 0;
	p = h->pred[0] == b ? h->pred[1] : h->pred[0];
	if (p == b || p == h || p->jmp.type != Jjmp || p->s1 != h)
		return 0;
	*body = b;
	*pre = p;
	*exit = x;
	return 1;
}

void
lpromote(Fn *fn)
{
	Blk *h, *b, *p, *x;
	int changed, again;
	uint n;

	changed = 0;
	do {
		again = 0;
		for (n=0; n<fn->nblk && !again; n++) {
			h = fn->rpo[n];
			if (!loopblocks(h, &b, &p, &x))
				continue;
			if (!promote1(fn, h, b, p, x))
				continue;
			/* The blocks changed under the definitions and the
			 * predecessor lists that the checks read. */
			fillcfg(fn);
			filluse(fn);
			changed = again = 1;
		}
	} while (again);
	if (changed) {
		filldom(fn);
		ssacheck(fn);
	}
}

/* Reduce affine addresses to an initial value and a constant step in the
 * same arithmetic class. A w index remains modulo 2^32. Only a plain
 * zero-based w counter guarded by an unsigned upper bound can cross an
 * extuw into an l recurrence. Its body never observes a wrapped counter. */
enum { MaxAffine = 5 };

typedef struct Affine Affine;
struct Affine {
	Ins *op[MaxAffine];
	int vary[MaxAffine];
	int n;
};

static Ins *
defins(Fn *fn, Ref r)
{
	return rtype(r) == RTmp && r.val >= Tmp0 ? fn->tmp[r.val].def : 0;
}

static int
affine(Fn *fn, Blk *h, Blk *b, Ref r, Phi *p, int k, int wide,
    Affine *a, int depth)
{
	Ins *i;
	int v;

	if (req(r, p->to))
		return p->cls == k;
	if (!depth || !(i = defins(fn, r)) || i->cls != k
	|| fn->tmp[r.val].bid != b->id)
		return 0;
	v = 0;
	switch (i->op) {
	case Oadd:
	case Omul:
	case Osub:
		if (inloop(fn, i->arg[1], h, b)) {
			if (i->op == Osub || inloop(fn, i->arg[0], h, b))
				return 0;
			v = 1;
		}
		if (!affine(fn, h, b, i->arg[v], p, k, wide, a, depth-1))
			return 0;
		break;
	case Oextuw:
		if (!wide || k != Kl || !req(i->arg[0], p->to))
			return 0;
		break;
	default:
		return 0;
	}
	a->op[a->n] = i;
	a->vary[a->n++] = v;
	return 1;
}

/* Do not make a private induction variable if the arithmetic it replaces
 * would stay live for other users. A two-op word chain still wins when
 * it contains the multiply of a wrapping index. */
static int
affinewin(Fn *fn, Affine *a, int k)
{
	int n;

	if (a->n < (k == Kw ? 2 : 3))
		return 0;
	for (n=0; n<a->n-1; n++)
		if (fn->tmp[a->op[n]->to.val].nuse != 1)
			return 0;
	return 1;
}

static int
consteq(Fn *fn, Ref r, int64_t v)
{
	Con *c;

	if (rtype(r) != RCon)
		return 0;
	c = &fn->con[r.val];
	return c->type == CBits && !c->flt && c->bits.i == v;
}

static Ref
affineins(Fn *fn, Ins *v, uint *n, int op, int k, Ref a, Ref b)
{
	Ref r;

	if (op == Oextuw && rtype(a) == RCon
	&& fn->con[a.val].type == CBits && !fn->con[a.val].flt)
		return getcon((uint32_t)fn->con[a.val].bits.i, fn);
	if (op == Omul && consteq(fn, a, 0))
		return a;
	if (op == Omul) {
		if (consteq(fn, b, 0) || consteq(fn, a, 1))
			return b;
		if (consteq(fn, b, 1))
			return a;
	}
	if (op == Oadd && consteq(fn, a, 0))
		return b;
	if ((op == Oadd || op == Osub) && consteq(fn, b, 0))
		return a;
	r = newtmp("stride", k, fn);
	v[(*n)++] = (Ins){.op = op, .cls = k, .to = r, .arg = {a, b}};
	return r;
}

static int
reduce1(Fn *fn, Blk *h, Blk *b, Blk *pre, Phi *p)
{
	Ins *i, *inc, *test, code[MaxAffine * 2], next;
	Ref init, step, r, a0[2];
	Phi *iv;
	Affine a;
	uint n;
	int j, k, wide;

	if (p->cls != Kw && p->cls != Kl)
		return 0;
	j = p->blk[0] != pre;
	init = p->arg[j];
	inc = defins(fn, p->arg[1-j]);
	if (!inc || inc->cls != p->cls || inc->op != Oadd
	|| !req(inc->arg[0], p->to) || inloop(fn, inc->arg[1], h, b))
		return 0;
	step = inc->arg[1];
	test = defins(fn, h->jmp.arg);
	wide = p->cls == Kw && h->s1 == b && test && test->op == Ocmpw + Ciult
		&& req(test->arg[0], p->to) && consteq(fn, init, 0)
		&& consteq(fn, step, 1);

	/* Start at a memory address and stop at the first profitable
	 * recurrence. A width change with possible wrap stays outside it. */
	for (i=b->ins; i<&b->ins[b->nins]; i++) {
		if (!isload(i->op) && !isstore(i->op))
			continue;
		r = addrof(i);
		for (j=0; j<MaxAffine; j++) {
			Ins *d = defins(fn, r);

			if (!d || fn->tmp[r.val].bid != b->id)
				break;
			k = d->cls;
			a.n = 0;
			if (affine(fn, h, b, r, p, k, wide, &a, MaxAffine) && affinewin(fn, &a, k))
				goto Found;
			if (isext(d->op) || d->op == Oshl
			|| (d->op == Omul && !inloop(fn, d->arg[1], h, b)))
				r = d->arg[0];
			else if (d->op == Oadd && !inloop(fn, d->arg[0], h, b))
				r = d->arg[1];
			else if (d->op == Oadd && !inloop(fn, d->arg[1], h, b))
				r = d->arg[0];
			else
				break;
		}
	}
	return 0;
Found:
	n = 0;
	for (j=0; j<a.n; j++) {
		i = a.op[j];
		a0[0] = i->arg[0];
		a0[1] = i->arg[1];
		a0[a.vary[j]] = init;
		init = affineins(fn, code, &n, i->op, i->cls, a0[0], a0[1]);
		if (i->op == Omul)
			step = affineins(fn, code, &n, Omul, i->cls, step, i->arg[1-a.vary[j]]);
	}
	iv = alloc(sizeof *iv);
	iv->cls = k;
	iv->to = r;
	iv->narg = 2;
	iv->arg = vnew(2, sizeof iv->arg[0], PFn);
	iv->blk = vnew(2, sizeof iv->blk[0], PFn);
	iv->arg[0] = init;
	iv->blk[0] = pre;
	iv->arg[1] = newtmp("stride", k, fn);
	iv->blk[1] = b;
	iv->link = h->phi;
	h->phi = iv;
	next = (Ins){.op = Oadd, .cls = k, .to = iv->arg[1], .arg = {r, step}};
	*a.op[a.n-1] = (Ins){.op = Onop};
	if (n)
		insert(pre, code, n, 1);
	insert(b, &next, 1, 1);
	return 1;
}

int
lreduce(Fn *fn)
{
	Blk *h, *b, *pre, *exit;
	Phi *p;
	uint n;
	int changed;

	changed = 0;
	for (n=0; n<fn->nblk; n++) {
		h = fn->rpo[n];
		if (!loopblocks(h, &b, &pre, &exit))
			continue;
		for (p=h->phi; p; p=p->link)
			if (reduce1(fn, h, b, pre, p)) {
				filluse(fn);
				changed = 1;
				break;
			}
	}
	return changed;
}

enum { MaxRows = 2 };
typedef struct Row Row;
struct Row {
	Affine index, address;
	Ref init, step;
	Phi *root;
};

/* Find a single-use address expression above a zero-extended word
 * index. The arithmetic above the extension is modulo 2^64. */
static int
rowaddr(Fn *fn, Blk *h, Blk *b, Ref r, Row *row, Ref *index, int depth)
{
	Ins *i;
	int varying;

	i = defins(fn, r);
	if (!depth || !i || i->cls != Kl || fn->tmp[r.val].bid != b->id
	|| fn->tmp[r.val].nuse != 1)
		return 0;
	varying = 0;
	if (i->op == Oextuw)
		*index = i->arg[0];
	else {
		if (i->op != Oadd && i->op != Osub && i->op != Omul && i->op != Oshl)
			return 0;
		if (inloop(fn, i->arg[1], h, b)) {
			if ((i->op != Oadd && i->op != Omul) || inloop(fn, i->arg[0], h, b))
				return 0;
			varying = 1;
		}
		if (!rowaddr(fn, h, b, i->arg[varying], row, index, depth-1))
			return 0;
	}
	row->address.vary[row->address.n] = varying;
	row->address.op[row->address.n++] = i;
	return 1;
}

static int
rowindex(Fn *fn, Blk *h, Blk *b, Blk *pre, Ref index, Row *row)
{
	Phi *p;
	Ins *inc;

	for (p=h->phi; p; p=p->link) {
		if (p->cls != Kw)
			continue;
		inc = defins(fn, phiarg(p, b));
		if (!inc || inc->cls != Kw || inc->op != Oadd
		|| fn->tmp[inc->to.val].bid != b->id
		|| !req(inc->arg[0], p->to) || inloop(fn, inc->arg[1], h, b))
			continue;
		row->index.n = 0;
		if (!affine(fn, h, b, index, p, Kw, 0, &row->index, MaxAffine))
			continue;
		row->root = p;
		row->init = phiarg(p, pre);
		row->step = inc->arg[1];
		return 1;
	}
	return 0;
}

static void
extendphi(Phi *p, Blk *b, Ref r)
{
	uint n;

	n = p->narg++;
	vgrow(&p->blk, p->narg);
	vgrow(&p->arg, p->narg);
	p->blk[n] = b;
	p->arg[n] = r;
}

static Phi *
pairphi(int cls, Ref result, Blk *b, Ref rb, Blk *c, Ref rc, Blk *dst)
{
	Phi *p;

	p = alloc(sizeof *p);
	*p = (Phi){.cls = cls, .to = result, .link = dst->phi};
	p->blk = vnew(0, sizeof p->blk[0], PFn);
	p->arg = vnew(0, sizeof p->arg[0], PFn);
	extendphi(p, b, rb);
	extendphi(p, c, rc);
	dst->phi = p;
	return p;
}

static Ref
mapped(Ref *map, Ref r)
{
	if (rtype(r) == RTmp && !req(map[r.val], R))
		return map[r.val];
	return r;
}

static void
remap(Blk *b, Ref *map)
{
	Ins *i;

	for (i=b->ins; i<&b->ins[b->nins]; i++) {
		i->to = mapped(map, i->to);
		i->arg[0] = mapped(map, i->arg[0]);
		i->arg[1] = mapped(map, i->arg[1]);
	}
	b->jmp.arg = mapped(map, b->jmp.arg);
}

static void
rowdefs(Fn *fn, Blk *origin, Blk *fast, Blk *h, Blk *b, Blk *x, Ref *map, Ref *slow, int nt)
{
	int t;
	uint n;
	for (t=Tmp0; t<nt; t++) {
		if (fn->tmp[t].bid != origin->id)
			continue;
		map[t] = newtmp("row", fn->tmp[t].cls, fn);
		if (origin != h)
			continue;
		for (n=0; n<fn->tmp[t].nuse; n++) {
			Use *u = &fn->tmp[t].use[n];
			if (u->bid == h->id || u->bid == b->id
			|| (u->bid == x->id && u->type == UPhi))
				continue;
			slow[t] = newtmp("row", fn->tmp[t].cls, fn);
			pairphi(fn->tmp[t].cls, TMP(t), h, slow[t], fast, map[t], x);
			break;
		}
	}
}

/* The modular recurrence in the original loop may have no other use
 * in the fast loop. Remove its phi and increment together. */
static void
dropword(Fn *fn, Blk *fast, Blk *body, Blk *b, Row *row, Ref *map)
{
	Phi *p, **pp;
	Ref next;
	Ins *inc;

	p = row->root;
	next = phiarg(p, b);
	if (row->index.n || fn->tmp[p->to.val].nuse != 2 || fn->tmp[next.val].nuse != 1)
		return;
	inc = defins(fn, next);
	for (pp=&fast->phi; *pp; pp=&(*pp)->link)
		if (req((*pp)->to, map[p->to.val])) {
			*pp = (*pp)->link;
			body->ins[inc-b->ins] = (Ins){.op = Onop};
			return;
		}
}

/* Version a two-block loop with a dedicated exit. Repair the original
 * exit phis and give any other live header value an exit phi of its own. */
static void
rowversion(Fn *fn, Blk *h, Blk *b, Blk *pre, Blk *x, Row *row, int nr, Ref limit)
{
	Ins checks[2 + MaxRows * (MaxAffine * 4 + 9)], update[MaxRows], *i;
	Phi *p, *exitphis;
	Blk *fast, *body;
	Ref *map, *slow, a[2], init, step, last, top, tops, ok, one;
	uint n, j, t;
	int k, r, nt;

	fast = newblk();
	body = newblk();
	fast->name = strf(PFn, "%s.row", h->name);
	body->name = strf(PFn, "%s.row", b->name);
	fast->jmp = h->jmp;
	body->jmp = b->jmp;
	fast->s1 = body;
	fast->s2 = x;
	body->s1 = fast;
	fast->link = body;
	body->link = pre->link;
	pre->link = fast;

	nt = fn->ntmp;
	map = emalloc(nt * sizeof *map);
	slow = emalloc(nt * sizeof *slow);
	exitphis = x->phi;
	rowdefs(fn, h, fast, h, b, x, map, slow, nt);
	rowdefs(fn, b, fast, h, b, x, map, slow, nt);
	idup(fast, h->ins, h->nins);
	idup(body, b->ins, b->nins);
	remap(fast, map);
	remap(body, map);
	for (p=h->phi; p; p=p->link)
		pairphi(p->cls, map[p->to.val], pre, phiarg(p, pre),
			body, mapped(map, phiarg(p, b)), fast);
	for (r=0; r<nr; r++)
		dropword(fn, fast, body, b, &row[r], map);
	remap(h, slow);
	remap(b, slow);
	for (p=h->phi; p; p=p->link) {
		t = phiargn(p, b);
		p->arg[t] = mapped(slow, p->arg[t]);
		p->to = mapped(slow, p->to);
	}
	for (p=exitphis; p; p=p->link) {
		t = phiargn(p, h);
		extendphi(p, fast, mapped(map, p->arg[t]));
		p->arg[t] = mapped(slow, p->arg[t]);
	}

	n = 0;
	one = getcon(1, fn);
	last = affineins(fn, checks, &n, Osub, Kw, limit, one);
	last = affineins(fn, checks, &n, Oextuw, Kl, last, R);
	tops = R;
	for (r=0; r<nr; r++) {
		init = row[r].init;
		step = row[r].step;
		for (k=0; k<row[r].index.n; k++) {
			i = row[r].index.op[k];
			j = row[r].index.vary[k];
			a[0] = i->arg[0];
			a[1] = i->arg[1];
			a[j] = init;
			init = affineins(fn, checks, &n, i->op, Kw, a[0], a[1]);
			if (i->op == Omul)
				step = affineins(fn, checks, &n, Omul, Kw, step, i->arg[1-j]);
		}
		init = affineins(fn, checks, &n, Oextuw, Kl, init, R);
		step = affineins(fn, checks, &n, Oextuw, Kl, step, R);
		/* The product plus the initial index fits in a long, even
		 * for a zero-trip loop with last == UINT32_MAX. */
		top = affineins(fn, checks, &n, Omul, Kl, last, step);
		top = affineins(fn, checks, &n, Oadd, Kl, top, init);
		tops = req(tops, R) ? top : affineins(fn, checks, &n, Oor, Kl, tops, top);
		for (k=1; k<row[r].address.n; k++) {
			i = row[r].address.op[k];
			j = row[r].address.vary[k];
			a[0] = i->arg[0];
			a[1] = i->arg[1];
			a[j] = init;
			init = affineins(fn, checks, &n, i->op, Kl, a[0], a[1]);
			if (i->op == Omul || i->op == Oshl)
				step = affineins(fn, checks, &n, i->op, Kl, step, i->arg[1-j]);
		}
		i = row[r].address.op[row[r].address.n-1];
		t = i-b->ins;
		update[r] = (Ins){.op = Oadd, .cls = Kl,
			.to = newtmp("stride", Kl, fn), .arg = {body->ins[t].to, step}};
		pairphi(Kl, update[r].arg[0], pre, init, body, update[r].to, fast);
		for (k=0; k<row[r].address.n; k++) {
			t = row[r].address.op[k] - b->ins;
			body->ins[t] = (Ins){.op = Onop};
		}
	}
	top = affineins(fn, checks, &n, Oshr, Kl, tops, getcon(32, fn));
	ok = affineins(fn, checks, &n, Ocmpl + Cieq, Kw, top, CON_Z);
	insert(body, update, nr, 1);
	insert(pre, checks, n, 1);
	pre->jmp.type = Jjnz;
	pre->jmp.arg = ok;
	pre->s1 = fast;
	pre->s2 = h;
	free(map);
	free(slow);
}

static int
rowreduce(Fn *fn, Blk *h)
{
	Blk *b, *pre, *x, *scan;
	Ins *test, *inc, *i;
	Phi *p;
	Ref limit, index;
	Row row[MaxRows];
	int nr;

	if (!loopblocks(h, &b, &pre, &x) || h->s1 != b || x->npred != 1
	|| h->nins + b->nins > 128)
		return 0;
	for (scan=h; scan; scan=scan == h ? b : 0)
		for (i=scan->ins; i<&scan->ins[scan->nins]; i++)
			if (isalloc(i->op))
				return 0;
	test = defins(fn, h->jmp.arg);
	if (!test || test->op != Ocmpw + Ciult)
		return 0;
	limit = test->arg[1];
	if (inloop(fn, limit, h, b))
		return 0;
	for (p=h->phi; p && !req(p->to, test->arg[0]); p=p->link)
		;
	if (!p || p->cls != Kw || !consteq(fn, phiarg(p, pre), 0))
		return 0;
	inc = defins(fn, phiarg(p, b));
	if (!inc || inc->cls != Kw || inc->op != Oadd
	|| !req(inc->arg[0], p->to) || !consteq(fn, inc->arg[1], 1))
		return 0;
	nr = 0;
	for (i=b->ins; i<&b->ins[b->nins] && nr<MaxRows; i++) {
		if (!isload(i->op) && !isstore(i->op))
			continue;
		row[nr].address.n = 0;
		if (rowaddr(fn, h, b, addrof(i), &row[nr], &index, MaxAffine)
		&& row[nr].address.n >= 3 && rowindex(fn, h, b, pre, index, &row[nr]))
			nr++;
	}
	if (!nr)
		return 0;
	rowversion(fn, h, b, pre, x, row, nr, limit);
	return 1;
}

int
lrowreduce(Fn *fn)
{
	Blk **original;
	uint n, nb;
	int changed;

	nb = fn->nblk;
	original = alloc(nb * sizeof *original);
	memcpy(original, fn->rpo, nb * sizeof *original);
	changed = 0;
	for (n=0; n<nb; n++)
		if (rowreduce(fn, original[n])) {
			fillcfg(fn);
			filluse(fn);
			filldom(fn);
			ssacheck(fn);
			changed = 1;
		}
	return changed;
}
