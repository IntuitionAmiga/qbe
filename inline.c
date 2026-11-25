#include "all.h"

/*
 * Inlining of small functions of one compilation unit.
 *
 * The functions of a unit are held after parsing, before any pass has
 * run, so every body is still the plain IL a front end wrote: stack
 * variables are allocs, phis come only from the front end's conditional
 * expressions, and a temporary may be defined more than once.  A call to a candidate is replaced by a copy of the
 * callee's blocks.  The argument values are copied into the callee's
 * parameter temporaries, each return becomes a copy of the result and a
 * jump to the code after the call, and the callee's allocs move to the
 * entry block of the caller, where the memory passes expect static slots.
 * The ordinary passes then see one function.  The callee stays in the
 * output, as it may be called from elsewhere.
 *
 * Leaf candidates may take and return aggregates. Scalar candidates may
 * also forward one direct call, or call a parameter known at the site
 * to name a strong function defined in this unit.
 * All candidates exclude dynamic allocation and target operations that
 * touch machine state, and have size bounds. Functions are taken in
 * the order the unit defines them, so a function that has inlined its own
 * callees can itself become a candidate when it is still within the bound.
 *
 * Target operations.  An IE64 operation is accepted when ie64eff says that
 * its effects are its operands, its declared memory accesses and the FPSR
 * flags that it sets.  MMU, interrupt, system, atomic and FPSR move
 * operations are not accepted.  Inlining copies the operations in order and
 * the later passes keep them pinned, so their order with respect to each
 * other is the order of the calls.
 *
 * Aggregate parameters.  The ABI gives the callee a private copy of an
 * aggregate argument, and the copy stays unless all of this holds:
 *   - the argument is a stack variable of the caller: the result of an
 *     alloc of the entry block with a constant size and no other
 *     definition;
 *   - the body uses the address of the parameter, and addresses derived
 *     from it, only to load, to read as the source of a blit or to feed
 *     the read operand of a target operation, so that it cannot compare,
 *     store, pass or return the address, and it cannot write the
 *     parameter;
 *   - every write of the body goes to memory that the body owns: a stack
 *     variable of the callee or the copy of a parameter that stays.  A
 *     write through another pointer, to a global or by a call may reach
 *     the argument and keeps the copy.
 * Otherwise the call site makes the copy itself, in a new stack slot of
 * the caller.
 *
 * Aggregate results.  The result is the address of an object that holds
 * the value.  When the callee returns one of its own variables, the
 * result is that variable, which is a slot of the caller after inlining.
 * Otherwise the value is copied to a new slot, so that a result never
 * names storage that the caller can change.
 */

enum {
	MaxIns = 140,		/* instructions of a candidate */
	MaxBlk = 24,		/* blocks of a candidate */
	MaxWrapperIns = 24,
	MaxWrapperBlk = 4,
	MaxCallbackIns = 180,
	MaxCallbackBlk = 40,
	MaxSites = 48,		/* inlined calls in one function */
	MaxGrow = 6000,		/* instructions a function may reach */
	MaxAgg = 128,		/* bytes of an aggregate parameter or result */
	MaxCount = 1 << 20,	/* a count that no bound can accept */
};

typedef struct Cand Cand;
struct Cand {
	Fn *fn;
	int ok;
	int npar;
	int par[8];	/* parameter classes */
	int ptyp[8];	/* aggregate type of a parameter, or -1 */
	int ret;	/* class of the returned value, or Kx */
	int retty;	/* aggregate type of the result, or -1 */
	uint nret;	/* blocks that return a value */
	uint dropok;	/* aggregate parameters whose copy the body allows to go */
	uchar *own;	/* per temporary: names only memory the body owns */
	Ins *call;	/* the sole call of a non-leaf candidate */
	int callback;	/* parameter supplying its target, or -1 */
};

static void
renumber(Fn *fn)
{
	Blk *b;
	uint n;

	n = 0;
	for (b=fn->start; b; b=b->link)
		b->id = n++;
}

/*
 * The instructions a copy of an aggregate of sz bytes becomes: for each
 * chunk of eight bytes, and for the tail in chunks of four, two and one,
 * a load, a store and two address additions.
 */
static uint
copycost(uint64_t sz)
{
	uint n, r;

	n = sz / 8;
	for (r=sz%8; r; r&=r-1)
		n++;
	return 4 * n;
}

/*
 * The size of a block, in the instructions it is about to become: a blit
 * stands for a load and a store of each eight bytes.
 */
static uint
bcount(Blk *b)
{
	Ins *i;
	uint64_t n;

	n = 0;
	for (i=b->ins; i<&b->ins[b->nins]; i++)
		if (i->op == Oblit0)
			n += copycost(rsval((i+1)->arg[0]));
		else if (i->op != Oblit1 && i->op != Onop)
			n++;
	return n > MaxCount ? MaxCount : n;
}

static uint
count(Fn *fn)
{
	Blk *b;
	uint n;

	n = 0;
	for (b=fn->start; b; b=b->link)
		n += bcount(b);
	return n;
}

/*
 * The instructions that a call site to c adds to the body of c: a copy of
 * each parameter and of the result of each return, and for an aggregate
 * parameter that keeps its copy, a slot and the blit.  The result of an
 * aggregate may need a slot and a blit at each return.  The call and its
 * arguments go, which is not counted.
 */
static uint
sitecost(Cand *c, uint drop)
{
	uint n;
	int j;

	n = c->npar + c->nret;
	for (j=0; j<c->npar; j++)
		if (c->ptyp[j] >= 0 && !(drop >> j & 1))
			n += 1 + copycost(typ[c->ptyp[j]].size);
	if (c->retty >= 0)
		n += 1 + c->nret * copycost(typ[c->retty].size);
	return n;
}

/*
 * The effects of a private IE64 operation, for the inliner.  Returns 0 for
 * an operation that is not accepted.  Otherwise it returns 1 and sets
 * *rd and *wr to the masks of the operands that the operation uses as the
 * address of memory it reads and of memory it writes, as memeff() says.
 * Everything else that such an operation does is a result in a register or
 * a change of the FPSR flags, which the operands and the program order
 * describe.  An operation joins the list below once its effects are in
 * memeff(); one that is a barrier stays a call whatever the list says.
 *
 * Not accepted: MMU (tlbflush, tlbinval, suaen, suadis), interrupt (sei,
 * cli, wait, eret, rti), system (halt, smode, syscall, mfcr, mtcr), the
 * FPCR (fmovcr, fmovcc), which changes how other operations round, the
 * FPSR moves (fmovsr, fmovsc), the atomics (xchg, faa, fand, for, fxor,
 * cas, casd) and vaarg.
 *
 * The FPSR moves read and write the flags that every other FP operation
 * sets, and those other operations are plain IL that the passes may
 * merge or move.  A call keeps them apart, so the moves stay calls: once
 * inlined, a comparison before the clear could serve the same comparison
 * after it.  The FP builtins that only compute are pinned, and their
 * flags are set where the call put them.
 */
static int
ie64eff(Ins *i, uint *rd, uint *wr)
{
	MemEff e;
	int n;

	*rd = *wr = 0;
	switch (i->op) {
	case Oie64nop:
	case Oie64fmovecr: case Oie64dmovecr:
	case Oie64fmod: case Oie64dmod:
	case Oie64fabs: case Oie64dabs:
	case Oie64fint: case Oie64dint:
	case Oie64fcvtsd: case Oie64fcvtds:
	case Oie64fsin: case Oie64fcos: case Oie64ftan: case Oie64fatan:
	case Oie64flog: case Oie64fexp: case Oie64fpow: case Oie64fsqrt:
	case Oie64dsin: case Oie64dcos: case Oie64dtan: case Oie64datan:
	case Oie64dlog: case Oie64dexp: case Oie64dpow: case Oie64dsqrt:
	case Oie64fipr: case Oie64ftrv:
		break;
	default:
		return 0;
	}
	memeff(i, &e);
	if (e.barrier)
		return 0;
	for (n=0; n<e.n; n++) {
		if (e.span[n].rw & MEread)
			*rd |= 1u << e.span[n].arg;
		if (e.span[n].rw & MEwrite)
			*wr |= 1u << e.span[n].arg;
	}
	return 1;
}

/* The temporary a reference names, or 0. */
static uint
tmpof(Ref r)
{
	return rtype(r) == RTmp && r.val >= Tmp0 ? r.val : 0;
}

/* Is the instruction one that adds an offset to an address, or copies it? */
static int
derives(Ins *i)
{
	return i->cls == Kl
		&& (i->op == Oadd || i->op == Osub || i->op == Ocopy);
}

/* The operands of a deriving instruction that carry the address. */
static int
nbase(Ins *i)
{
	return i->op == Oadd ? 2 : 1;
}

/*
 * Find out what the body does with the addresses of its aggregate
 * parameters, and which temporaries name only memory the body owns.
 * It sets c->dropok and c->own.
 */
static void
analyse(Cand *c)
{
	Fn *g;
	Blk *b;
	Ins *i;
	Phi *p;
	uchar *taint, *own, *addr;
	uint bad, t, j, nt, rd, wr;
	int changed, wrok, v;

	g = c->fn;
	nt = g->ntmp;
	taint = alloc(nt * sizeof taint[0]);
	own = alloc(nt * sizeof own[0]);
	addr = alloc(nt * sizeof addr[0]);
	c->own = own;
	c->dropok = 0;
	for (j=0; j<(uint)c->npar; j++)
		if (c->ptyp[j] >= 0)
			taint[g->start->ins[j].to.val] |= 1u << j;

	/* The parameters whose address may reach each temporary. */
	do {
		changed = 0;
		for (b=g->start; b; b=b->link) {
			for (p=b->phi; p; p=p->link)
				for (j=0; j<p->narg; j++)
					if ((t = tmpof(p->arg[j]))
					&& (taint[t] & ~taint[p->to.val])) {
						taint[p->to.val] |= taint[t];
						changed = 1;
					}
			for (i=b->ins; i<&b->ins[b->nins]; i++) {
				if (!tmpof(i->to) || !derives(i))
					continue;
				for (j=0; j<(uint)nbase(i); j++)
					if ((t = tmpof(i->arg[j]))
					&& (taint[t] & ~taint[i->to.val])) {
						taint[i->to.val] |= taint[t];
						changed = 1;
					}
			}
		}
	} while (changed);

	/*
	 * The temporaries that may hold an address of the body's variables or
	 * of a parameter, or a number computed from one, such as the difference
	 * of two addresses.  Such a number is not an offset: adding it to an
	 * address of unknown origin gives an address of unknown origin.  A
	 * 64-bit value that comes from memory or from a parameter may be an
	 * address too, whatever its class; a value of 32 bits or less from
	 * memory is a number.
	 */
	for (b=g->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (tmpof(i->to)
			&& (isalloc(i->op) || i->op == Oparc
			|| ((isload(i->op) || ispar(i->op))
			&& (i->cls == Kl || i->cls == Kd))))
				addr[i->to.val] = 1;
	do {
		changed = 0;
		for (b=g->start; b; b=b->link) {
			for (p=b->phi; p; p=p->link)
				for (j=0; j<p->narg; j++)
					if ((t = tmpof(p->arg[j])) && addr[t]
					&& !addr[p->to.val]) {
						addr[p->to.val] = 1;
						changed = 1;
					}
			for (i=b->ins; i<&b->ins[b->nins]; i++) {
				if (!tmpof(i->to) || addr[i->to.val]
				|| isload(i->op) || ispar(i->op))
					continue;
				for (j=0; j<2; j++)
					if ((t = tmpof(i->arg[j])) && addr[t]) {
						addr[i->to.val] = 1;
						changed = 1;
						break;
					}
			}
		}
	} while (changed);

	/* A use other than a load, a blit source or a read operand. */
	bad = 0;
	for (b=g->start; b; b=b->link) {
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (ispar(i->op))
				continue;
			rd = 0;
			if (derives(i))
				rd = (1u << nbase(i)) - 1;
			else if (isload(i->op) && !isvload(i->op))
				rd = 1;
			else if (i->op == Oblit0)
				rd = 1;
			else if (isie64op(i->op)) {
				ie64eff(i, &rd, &wr);
				rd &= ~wr;
			}
			for (j=0; j<2; j++)
				if ((t = tmpof(i->arg[j])) && !(rd >> j & 1))
					bad |= taint[t];
		}
		if ((t = tmpof(b->jmp.arg)))
			bad |= taint[t];
	}
	/* The parameters that keep their copy are memory the body owns. */
	for (b=g->start; b; b=b->link) {
		for (p=b->phi; p; p=p->link)
			own[p->to.val] = 1;
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (tmpof(i->to))
				own[i->to.val] = 1;
	}
	do {
		changed = 0;
		for (b=g->start; b; b=b->link) {
			for (p=b->phi; p; p=p->link) {
				if (!own[p->to.val])
					continue;
				v = 1;
				for (j=0; j<p->narg; j++)
					v &= (t = tmpof(p->arg[j])) && own[t];
				if (!v) {
					own[p->to.val] = 0;
					changed = 1;
				}
			}
			for (i=b->ins; i<&b->ins[b->nins]; i++) {
				if (!tmpof(i->to) || !own[i->to.val])
					continue;
				if (isalloc(i->op))
					v = 1;
				else if (i->op == Oparc)
					v = bad >> (i - g->start->ins) & 1;
				else if (derives(i)) {
					/* An owned address, and an offset that is
					 * not itself made of addresses. */
					uint x = tmpof(i->arg[0]), y = tmpof(i->arg[1]);

					if (i->op == Ocopy)
						v = x && own[x];
					else if (i->op == Osub)
						v = x && own[x] && !(y && addr[y]);
					else
						v = (x && own[x] && !(y && addr[y]))
						|| (y && own[y] && !(x && addr[x]));
				} else
					v = 0;
				if (!v) {
					own[i->to.val] = 0;
					changed = 1;
				}
			}
		}
	} while (changed);

	/* Every write goes to memory that the body owns. */
	wrok = 1;
	for (b=g->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			wr = 0;
			if (isstore(i->op))
				wr = 2;
			else if (i->op == Oblit0)
				wr = 2;
			else if (isie64op(i->op))
				ie64eff(i, &rd, &wr);
			for (j=0; j<2; j++)
				if ((wr >> j & 1) && !((t = tmpof(i->arg[j])) && own[t]))
					wrok = 0;
		}
	if (wrok)
		for (j=0; j<(uint)c->npar; j++)
			if (c->ptyp[j] >= 0 && !(bad >> j & 1))
				c->dropok |= 1u << j;
}

static char *
callsym(Fn *fn, Ref r)
{
	Con *c;

	if (rtype(r) != RCon)
		return 0;
	c = &fn->con[r.val];
	if (c->type != CAddr || (c->sym.type & ~SExt) != SGlo || c->bits.i)
		return 0;
	return str(c->sym.id);
}

/* A single-definition chain of long copies can carry a callback
 * parameter. Slot promotion exposes the usual front-end parameter spill
 * only if the slot is private and its accesses have uniform width. */
static int
callbackpar(Cand *c)
{
	Fn *g = c->fn;
	Ref r = c->call->arg[0];
	Tmp *t;
	Ins *d;
	int k, n;

	filluse(g);
	promote(g);
	filluse(g);
	for (n=0; n<g->ntmp && tmpof(r); n++) {
		t = &g->tmp[r.val];
		if (t->ndef != 1 || !(d = t->def) || d->cls != Kl)
			break;
		if (d->op == Opar) {
			k = d - g->start->ins;
			assert(k >= 0 && k < c->npar);
			return k;
		}
		if (d->op != Ocopy)
			break;
		r = d->arg[0];
	}
	return -1;
}

static void
classify(Cand *c)
{
	Fn *g;
	Blk *b;
	Ins *i;
	uint nb, ni, rd, wr;
	int ret, k;

	g = c->fn;
	c->ok = 0;
	c->npar = 0;
	c->ret = Kx;
	c->retty = -1;
	c->nret = 0;
	c->dropok = 0;
	c->own = 0;
	c->call = 0;
	c->callback = -1;
	if (g->vararg || g->dynalloc || g->lnk.weak)
		return;
	if (g->retty >= 0) {
		if (typ[g->retty].size == 0 || typ[g->retty].size > MaxAgg)
			return;
		c->retty = g->retty;
	}
	nb = ni = 0;
	ret = -2;
	for (b=g->start; b; b=b->link) {
		nb++;
		if (b->phi && b == g->start)
			return;
		ni += bcount(b);
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (ispar(i->op)) {
				if (b != g->start
				|| (i->op != Opar && i->op != Oparc)
				|| c->npar >= 8 || i != &b->ins[c->npar])
					return;
				c->ptyp[c->npar] = -1;
				if (i->op == Oparc) {
					Typ *t = &typ[i->arg[0].val];

					if (t->size == 0 || t->size > MaxAgg)
						return;
					c->ptyp[c->npar] = i->arg[0].val;
				}
				c->par[c->npar++] = i->cls;
				continue;
			}
			if (i->op == Ocall) {
				if (c->call || rtype(i->arg[1]) == RType)
					return;
				c->call = i;
			}
			if (i->op == Ovastart
			|| i->op == Ovaarg || i->op == Oargc
			|| i->op == Oargv || i->op == Oarge
			|| (isie64op(i->op) && !ie64eff(i, &rd, &wr))
			|| i->op == Osalloc)
				return;
			if (isalloc(i->op)
			&& (rtype(i->arg[0]) != RCon
			|| g->con[i->arg[0].val].type != CBits))
				return;
		}
		switch (b->jmp.type) {
		case Jjmp: case Jjnz: case Jret0:
			break;
		case Jretw: case Jretl: case Jrets: case Jretd:
		case Jretc:
			k = b->jmp.type == Jretw ? Kw : b->jmp.type == Jrets ? Ks
				: b->jmp.type == Jretd ? Kd : Kl;
			if (ret != -2 && ret != k)
				return;
			ret = k;
			c->nret++;
			break;
		default:
			return;
		}
	}
	c->ret = ret == -2 ? Kx : ret;
	if (c->call) {
		if (c->retty >= 0)
			return;
		for (k=0; k<c->npar; k++)
			if (c->ptyp[k] >= 0)
				return;
		if (callsym(g, c->call->arg[0])) {
			if (ni > MaxWrapperIns || nb > MaxWrapperBlk)
				return;
		} else {
			if (ni > MaxCallbackIns || nb > MaxCallbackBlk
			|| (c->callback = callbackpar(c)) < 0)
				return;
		}
		c->ok = 1;
		return;
	}
	if (ni > MaxIns || nb > MaxBlk)
		return;
	c->ok = 1;
	analyse(c);
}

static Cand *
find(Cand *cs, int nc, Fn *fn, Ref r)
{
	char *name;
	int n;

	name = callsym(fn, r);
	if (!name)
		return 0;
	for (n=0; n<nc; n++)
		if (cs[n].fn != fn && !cs[n].fn->lnk.weak
		&& strcmp(cs[n].fn->name, name) == 0)
			return &cs[n];
	return 0;
}

typedef struct Map Map;
struct Map {
	Ref *tmp;
	Ref *con;
	Blk **blk;
	Fn *from, *to;
};

static Ref
remap(Map *m, Ref r)
{
	switch (rtype(r)) {
	case RTmp:
		return r.val < Tmp0 ? r : m->tmp[r.val];
	case RCon:
		return m->con[r.val];
	}
	return r;
}

/* Copy the phis of a callee block. */
static void
copyphis(Map *m, Blk *gb, Blk *nb)
{
	Phi *p, *np, **tail;
	uint j;

	tail = &nb->phi;
	for (p=gb->phi; p; p=p->link) {
		np = alloc(sizeof *np);
		*np = *p;
		np->to = remap(m, p->to);
		np->arg = alloc(p->narg * sizeof np->arg[0]);
		np->blk = alloc(p->narg * sizeof np->blk[0]);
		for (j=0; j<p->narg; j++) {
			np->arg[j] = remap(m, p->arg[j]);
			np->blk[j] = m->blk[p->blk[j]->id];
		}
		np->link = 0;
		*tail = np;
		tail = &np->link;
	}
}

/* Queue an alloc of the callee, or of the call site, for the entry block. */
static void
hoistalloc(Ins **hoist, uint *nhoist, uint *capp, Ins i)
{
	if (*nhoist >= *capp) {
		Ins *nh = alloc(2 * *capp * sizeof nh[0]);

		memcpy(nh, *hoist, *nhoist * sizeof nh[0]);
		*hoist = nh;
		*capp *= 2;
	}
	(*hoist)[(*nhoist)++] = i;
}

/* A new stack slot of the caller for an aggregate of a given type. */
static Ref
newslot(Fn *fn, int ty, Ins **hoist, uint *nhoist, uint *capp)
{
	Ref r;
	int al;

	al = typ[ty].align;
	r = newtmp("agg", Kl, fn);
	hoistalloc(hoist, nhoist, capp, (Ins){
		.op = al >= 4 ? Oalloc16 : al == 3 ? Oalloc8 : Oalloc4,
		.cls = Kl, .to = r,
		.arg = {getcon(typ[ty].size, fn), R}});
	return r;
}

/* A copy of an aggregate: the two instructions of a blit. */
static Ins *
blit(Ins *o, Ref src, Ref dst, int ty)
{
	*o++ = (Ins){.op = Oblit0, .arg = {src, dst}};
	*o++ = (Ins){.op = Oblit1, .arg = {INT(typ[ty].size), R}};
	return o;
}

/*
 * Inline the call at index k of block b.  The parameters in the mask drop
 * keep no copy of their aggregate argument.  Returns the continuation
 * block.
 */
static Blk *
inlinecall(Fn *fn, Cand *c, Blk *b, uint k, uint a0, uint drop, int ownres, Ins **hoist, uint *nhoist, uint *capp)
{
	static int n;
	Map m;
	Fn *g;
	Blk *gb, *nb, *cont, *first, *last, *entry;
	Ins *i, *call, *out, *o;
	Ref to, res, v;
	uint t, nin, nblk;
	int j;
	int cls;

	g = c->fn;
	call = &b->ins[k];
	to = call->to;
	cls = call->cls;
	n++;
	res = R;
	m.from = g;
	m.to = fn;
	m.tmp = alloc(g->ntmp * sizeof m.tmp[0]);
	m.con = alloc(g->ncon * sizeof m.con[0]);
	nblk = 0;
	for (gb=g->start; gb; gb=gb->link)
		nblk++;
	m.blk = alloc(nblk * sizeof m.blk[0]);
	for (t=Tmp0; t<(uint)g->ntmp; t++)
		m.tmp[t] = newtmp(g->tmp[t].name, g->tmp[t].cls, fn);
	m.con[0] = UNDEF;
	m.con[1] = CON_Z;
	for (t=2; t<(uint)g->ncon; t++)
		m.con[t] = newcon(&g->con[t], fn);
	for (gb=g->start; gb; gb=gb->link) {
		nb = newblk();
		nb->name = strf(PFn, "%s.i%d.%s", g->name, n, gb->name);
		nb->loop = b->loop;
		m.blk[gb->id] = nb;
	}
	cont = newblk();
	cont->name = strf(PFn, "%s.i%d.cont", g->name, n);
	cont->loop = b->loop;
	cont->jmp = b->jmp;
	cont->s1 = b->s1;
	cont->s2 = b->s2;
	cont->link = b->link;
	/* The successors now follow the continuation, not the split block. */
	for (j=0; j<2; j++) {
		Blk *sb = j ? b->s2 : b->s1;
		Phi *p;
		uint pj;

		if (!sb || (j && sb == b->s1))
			continue;
		for (p=sb->phi; p; p=p->link)
			for (pj=0; pj<p->narg; pj++)
				if (p->blk[pj] == b)
					p->blk[pj] = cont;
	}
	nin = b->nins - k - 1;
	out = alloc((nin + 1) * sizeof out[0]);
	icpy(out, &b->ins[k+1], nin);
	idup(cont, out, nin);

	first = last = 0;
	entry = m.blk[g->start->id];
	for (gb=g->start; gb; gb=gb->link) {
		nb = m.blk[gb->id];
		out = alloc((gb->nins + 3 * c->npar + 4) * sizeof out[0]);
		o = out;
		if (gb == g->start)
			for (t=0; t<(uint)c->npar; t++) {
				Ins *a = &b->ins[a0+t];

				if (c->ptyp[t] < 0)
					v = a->arg[0];
				else if (drop >> t & 1)
					v = a->arg[1];
				else {
					v = newslot(fn, c->ptyp[t], hoist, nhoist, capp);
					o = blit(o, a->arg[1], v, c->ptyp[t]);
				}
				*o++ = (Ins){.op = Ocopy, .cls = c->par[t],
					.to = m.tmp[gb->ins[t].to.val],
					.arg = {v, R}};
			}
		for (i=gb->ins; i<&gb->ins[gb->nins]; i++) {
			if (ispar(i->op))
				continue;
			if (isalloc(i->op)) {
				Ins h = *i;

				h.to = remap(&m, i->to);
				h.arg[0] = remap(&m, i->arg[0]);
				hoistalloc(hoist, nhoist, capp, h);
				continue;
			}
			*o = *i;
			o->to = remap(&m, i->to);
			o->arg[0] = remap(&m, i->arg[0]);
			o->arg[1] = remap(&m, i->arg[1]);
			if (i == c->call && c->callback >= 0)
				o->arg[0] = b->ins[a0 + c->callback].arg[0];
			o++;
		}
		copyphis(&m, gb, nb);
		nb->jmp = gb->jmp;
		if (isret(gb->jmp.type)) {
			if (gb->jmp.type != Jret0 && !req(to, R)) {
				v = remap(&m, gb->jmp.arg);
				if (c->retty >= 0
				&& !(ownres && c->own[tmpof(gb->jmp.arg)])) {
					/* Not a variable of the callee: copy it. */
					if (req(res, R))
						res = newslot(fn, c->retty, hoist,
							nhoist, capp);
					o = blit(o, v, res, c->retty);
					v = res;
				}
				*o++ = (Ins){.op = Ocopy, .cls = cls, .to = to,
					.arg = {v, R}};
			}
			nb->jmp.type = Jjmp;
			nb->jmp.arg = R;
			nb->s1 = cont;
			nb->s2 = 0;
		} else {
			nb->jmp.arg = remap(&m, gb->jmp.arg);
			nb->s1 = gb->s1 ? m.blk[gb->s1->id] : 0;
			nb->s2 = gb->s2 ? m.blk[gb->s2->id] : 0;
		}
		idup(nb, out, o - out);
		if (last)
			last->link = nb;
		else
			first = nb;
		last = nb;
	}
	last->link = cont;

	/* The head of the caller block ends in a jump to the callee. */
	out = alloc((a0 + 1) * sizeof out[0]);
	icpy(out, b->ins, a0);
	idup(b, out, a0);
	b->jmp.type = Jjmp;
	b->jmp.arg = R;
	b->s1 = entry;
	b->s2 = 0;
	b->link = first;
	return cont;
}

/*
 * May the result of the call at index k of block b be the callee's own
 * variable?  That variable is one slot of the caller, which the next
 * execution of the call overwrites.  So it must be the case that the
 * result has no other definition and no use before the call, in another
 * block or in a phi: the next execution of the call then comes after the
 * last use.
 */
static int
resultlocal(Fn *fn, Blk *b, uint k, Ref to)
{
	Blk *bb;
	Ins *i;
	Phi *p;
	uint n, j, ndef;

	ndef = 0;
	for (bb=fn->start; bb; bb=bb->link) {
		for (p=bb->phi; p; p=p->link) {
			if (req(p->to, to))
				return 0;
			for (j=0; j<p->narg; j++)
				if (req(p->arg[j], to))
					return 0;
		}
		for (i=bb->ins; i<&bb->ins[bb->nins]; i++) {
			n = i - bb->ins;
			if (req(i->to, to))
				ndef++;
			for (j=0; j<2; j++)
				if (req(i->arg[j], to) && (bb != b || n < k))
					return 0;
		}
		if (req(bb->jmp.arg, to) && bb != b)
			return 0;
	}
	return ndef == 1;
}

/* Is the reference the address of a stack variable of the entry block? */
static int
isstackvar(Fn *fn, Ref r)
{
	Blk *b;
	Ins *i, *d;
	Phi *p;
	uint t, n;

	if (!(t = tmpof(r)))
		return 0;
	n = 0;
	d = 0;
	for (b=fn->start; b; b=b->link) {
		for (p=b->phi; p; p=p->link)
			if (req(p->to, r))
				n++;
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (req(i->to, r)) {
				n++;
				d = b == fn->start ? i : 0;
			}
	}
	return n == 1 && d && isalloc(d->op)
		&& rtype(d->arg[0]) == RCon
		&& fn->con[d->arg[0].val].type == CBits;
}

void
inlinefns(Fn **fns, int nf)
{
	Cand *cs, *c;
	Fn *fn;
	Blk *b;
	Ins *i, *hoist, *ni;
	uint k, a0, nhoist, cap, sites, nargs, j, grow, drop;
	int f, ok;

	cs = alloc(nf * sizeof cs[0]);
	for (f=0; f<nf; f++) {
		cs[f].fn = fns[f];
		renumber(fns[f]);
		classify(&cs[f]);
	}
	for (f=0; f<nf; f++) {
		fn = fns[f];
		cap = 16;
		nhoist = sites = 0;
		hoist = alloc(cap * sizeof hoist[0]);
		grow = count(fn);
		for (b=fn->start; b; b=b->link) {
			for (k=0; k<b->nins; k++) {
				i = &b->ins[k];
				if (i->op != Ocall)
					continue;
				c = find(cs, nf, fn, i->arg[0]);
				if (!c || !c->ok || sites >= MaxSites)
					continue;
				a0 = k;
				while (a0 > 0 && isarg(b->ins[a0-1].op))
					a0--;
				nargs = k - a0;
				ok = nargs == (uint)c->npar;
				drop = 0;
				for (j=0; ok && j<nargs; j++) {
					Ins *a = &b->ins[a0+j];

					if (c->ptyp[j] < 0)
						ok = a->op == Oarg && a->cls == c->par[j];
					else {
						ok = a->op == Oargc
							&& rtype(a->arg[0]) == RType
							&& a->arg[0].val == (uint)c->ptyp[j];
						if (ok && (c->dropok >> j & 1)
						&& isstackvar(fn, a->arg[1]))
							drop |= 1u << j;
					}
				}
				/* An aggregate result goes with an aggregate callee. */
				if (ok)
					ok = (rtype(i->arg[1]) == RType)
						== (c->retty >= 0);
				if (ok && c->retty >= 0)
					ok = i->arg[1].val == (uint)c->retty;
				if (ok && !req(i->to, R))
					ok = c->ret == i->cls;
				if (ok && c->callback >= 0)
					ok = find(cs, nf, fn, b->ins[a0+c->callback].arg[0]) != 0;
				if (!ok || grow + count(c->fn) + sitecost(c, drop) > MaxGrow)
					continue;
				grow += count(c->fn) + sitecost(c, drop);
				sites++;
				inlinecall(fn, c, b, k, a0, drop,
					c->retty >= 0 && !req(i->to, R)
					&& resultlocal(fn, b, k, i->to),
					&hoist, &nhoist, &cap);
				break;
			}
		}
		if (nhoist) {
			/* Keep the parameters first: a function inlined later
			 * takes its parameters from the head of its entry block. */
			Blk *s = fn->start;
			uint np = 0;

			while (np < s->nins && ispar(s->ins[np].op))
				np++;
			ni = alloc((s->nins + nhoist) * sizeof ni[0]);
			icpy(ni, s->ins, np);
			icpy(ni + np, hoist, nhoist);
			icpy(ni + np + nhoist, s->ins + np, s->nins - np);
			idup(s, ni, s->nins + nhoist);
		}
		if (sites) {
			/* The calls may all be gone. */
			fn->leaf = 1;
			for (b=fn->start; b; b=b->link)
				for (i=b->ins; i<&b->ins[b->nins]; i++)
					if (i->op == Ocall)
						fn->leaf = 0;
			renumber(fn);
		}
		classify(&cs[f]);
	}
}
