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
 * Memory is only reached through the temporary, so every other access of
 * the loop must be known not to overlap the location.  QBE has no types to
 * tell it so.  With -fstrict-aliasing an access of another width does not
 * overlap an access of this width, as in C; byte accesses, volatile ones,
 * calls and private operations stop the pass.  Without the option the pass
 * does not run.
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
	int64_t off;
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
		l.off += d->op == Oadd ? c->bits.i : -c->bits.i;
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

/* Is this an access that is not an accumulator access, and may overlap it? */
static int
overlaps(Ins *i, int sz)
{
	int n;

	if (isvload(i->op) || isvstore(i->op))
		return 1;
	n = isload(i->op) ? loadsz(i) : storesz(i);
	return n == sz || n == 1;
}

/* May the loop run with the location in a register?  Its loads and stores
 * of the location must be of one width and in the body, and nothing else in
 * the loop may reach memory in a way that overlaps. */
static int
loopok(Fn *fn, Blk *h, Blk *b, Loc a, int sz, int k)
{
	Blk *x;
	Ins *i;
	int mine, nld, nst;

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
			if (sameloc(resolve(fn, addrof(i)), a)) {
				/* An access of the location itself. */
				if (x != b)
					return 0;
				if (isload(i->op)) {
					if (!(i->op == Oload && i->cls == k)
					&& !(k == Kw && (i->op == Oloadsw
					|| i->op == Oloaduw)))
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
			mine = overlaps(i, sz);
			if (mine)
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
lastvalue(Fn *fn, Blk *p, Loc a, int sz, int k)
{
	Ins *i;

	for (i=&p->ins[p->nins]; i-- > p->ins;) {
		if (i->op == Onop || !pinned(i))
			continue;
		if (isload(i->op) && !isvload(i->op))
			continue;
		if (isstore(i->op) && !isvstore(i->op)
		&& i->op == (k == Kw ? Ostorew : Ostorel)
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
		if (loopok(fn, h, b, a, sz, k))
			goto Found;
	}
	return 0;
Found:
	ldop = k == Kw ? Oloadsw : Oload;
	/* The value on entry. */
	init = lastvalue(fn, p, a, sz, k);
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
			if (h->jmp.type != Jjnz || !h->s1 || !h->s2
			|| h->s1 == h->s2 || h->npred != 2)
				continue;
			b = h->s1;
			x = h->s2;
			if (b->npred != 1 || b->jmp.type != Jjmp || b->s1 != h) {
				b = h->s2;
				x = h->s1;
				if (b->npred != 1 || b->jmp.type != Jjmp
				|| b->s1 != h)
					continue;
			}
			if (b == h || x == h || x == b || b->phi)
				continue;
			p = h->pred[0] == b ? h->pred[1] : h->pred[0];
			if (p == b || p == h || p->jmp.type != Jjmp || p->s1 != h)
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
