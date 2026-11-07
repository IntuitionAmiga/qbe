#include "all.h"

/*
 * Inlining of small leaf functions of one compilation unit.
 *
 * The functions of a unit are held after parsing, before any pass has
 * run, so every body is still the plain IL a front end wrote: stack
 * variables are allocs, there are no phis, and a temporary may be defined
 * more than once.  A call to a candidate is replaced by a copy of the
 * callee's blocks.  The argument values are copied into the callee's
 * parameter temporaries, each return becomes a copy of the result and a
 * jump to the code after the call, and the callee's allocs move to the
 * entry block of the caller, where the memory passes expect static slots.
 * The ordinary passes then see one function.  The callee stays in the
 * output, as it may be called from elsewhere.
 *
 * A candidate is a non-variadic function without aggregate parameters or
 * results, without calls, phis, private target operations or dynamic
 * allocation, within a size bound.  Functions are taken in the order the
 * unit defines them, so a function that has inlined its own callees can
 * itself become a candidate when it is still within the bound.
 */

enum {
	MaxIns = 140,		/* instructions of a candidate */
	MaxBlk = 24,		/* blocks of a candidate */
	MaxSites = 48,		/* inlined calls in one function */
	MaxGrow = 6000,		/* instructions a function may reach */
};

typedef struct Cand Cand;
struct Cand {
	Fn *fn;
	int ok;
	int npar;
	int par[8];	/* parameter classes */
	int ret;	/* class of the returned value, or Kx */
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

static uint
count(Fn *fn)
{
	Blk *b;
	uint n;

	n = 0;
	for (b=fn->start; b; b=b->link)
		n += b->nins;
	return n;
}

static void
classify(Cand *c)
{
	Fn *g;
	Blk *b;
	Ins *i;
	uint nb, ni;
	int ret, k;

	g = c->fn;
	c->ok = 0;
	c->npar = 0;
	c->ret = Kx;
	if (g->vararg || g->retty >= 0 || g->dynalloc || g->lnk.weak)
		return;
	nb = ni = 0;
	ret = -2;
	for (b=g->start; b; b=b->link) {
		nb++;
		if (b->phi)
			return;
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			ni++;
			if (ispar(i->op)) {
				if (b != g->start || i->op != Opar
				|| c->npar >= 8)
					return;
				c->par[c->npar++] = i->cls;
				continue;
			}
			if (i->op == Ocall || i->op == Ovastart
			|| i->op == Ovaarg || isarg(i->op) || isie64op(i->op)
			|| i->op == Oblit0 || i->op == Oblit1
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
			k = b->jmp.type == Jretw ? Kw : b->jmp.type == Jretl ? Kl
				: b->jmp.type == Jrets ? Ks : Kd;
			if (ret != -2 && ret != k)
				return;
			ret = k;
			break;
		default:
			return;
		}
	}
	if (ni > MaxIns || nb > MaxBlk)
		return;
	c->ret = ret == -2 ? Kx : ret;
	c->ok = 1;
}

static Cand *
find(Cand *cs, int nc, Fn *fn, Ref r)
{
	Con *c;
	int n;

	if (rtype(r) != RCon)
		return 0;
	c = &fn->con[r.val];
	if (c->type != CAddr || (c->sym.type & ~SExt) != SGlo || c->bits.i)
		return 0;
	for (n=0; n<nc; n++)
		if (cs[n].fn != fn && cs[n].ok
		&& strcmp(cs[n].fn->name, str(c->sym.id)) == 0)
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

/* Inline the call at index k of block b.  Returns the continuation block. */
static Blk *
inlinecall(Fn *fn, Cand *c, Blk *b, uint k, uint a0, Ins **hoist, uint *nhoist, uint *capp)
{
	static int n;
	Map m;
	Fn *g;
	Blk *gb, *nb, *cont, *first, *last, *entry;
	Ins *i, *call, *out, *o;
	Ref to;
	uint t, nin, h, nblk;
	int j;
	int cls;

	g = c->fn;
	call = &b->ins[k];
	to = call->to;
	cls = call->cls;
	n++;
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
		out = alloc((gb->nins + c->npar + 2) * sizeof out[0]);
		o = out;
		if (gb == g->start)
			for (t=0; t<(uint)c->npar; t++) {
				Ins *a = &b->ins[a0+t];

				*o++ = (Ins){.op = Ocopy, .cls = c->par[t],
					.to = m.tmp[gb->ins[t].to.val],
					.arg = {a->arg[0], R}};
			}
		for (i=gb->ins; i<&gb->ins[gb->nins]; i++) {
			if (ispar(i->op))
				continue;
			if (isalloc(i->op)) {
				if (*nhoist >= *capp) {
					Ins *nh = alloc(2 * *capp * sizeof nh[0]);

					memcpy(nh, *hoist, *nhoist * sizeof nh[0]);
					*hoist = nh;
					*capp *= 2;
				}
				h = (*nhoist)++;
				(*hoist)[h] = *i;
				(*hoist)[h].to = remap(&m, i->to);
				continue;
			}
			*o = *i;
			o->to = remap(&m, i->to);
			o->arg[0] = remap(&m, i->arg[0]);
			o->arg[1] = remap(&m, i->arg[1]);
			o++;
		}
		nb->jmp = gb->jmp;
		if (isret(gb->jmp.type)) {
			if (gb->jmp.type != Jret0 && !req(to, R))
				*o++ = (Ins){.op = Ocopy, .cls = cls, .to = to,
					.arg = {remap(&m, gb->jmp.arg), R}};
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

void
inlinefns(Fn **fns, int nf)
{
	Cand *cs, *c;
	Fn *fn;
	Blk *b;
	Ins *i, *hoist, *ni;
	uint k, a0, nhoist, cap, sites, nargs, j, grow;
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
				if (i->op != Ocall || rtype(i->arg[1]) == RType)
					continue;
				c = find(cs, nf, fn, i->arg[0]);
				if (!c || sites >= MaxSites
				|| grow + count(c->fn) > MaxGrow)
					continue;
				a0 = k;
				while (a0 > 0 && isarg(b->ins[a0-1].op))
					a0--;
				nargs = k - a0;
				ok = nargs == (uint)c->npar;
				for (j=0; ok && j<nargs; j++)
					ok = b->ins[a0+j].op == Oarg
						&& b->ins[a0+j].cls == c->par[j];
				if (ok && !req(i->to, R))
					ok = c->ret == i->cls;
				if (!ok)
					continue;
				grow += count(c->fn);
				sites++;
				inlinecall(fn, c, b, k, a0, &hoist, &nhoist, &cap);
				break;
			}
		}
		if (nhoist) {
			ni = alloc((fn->start->nins + nhoist) * sizeof ni[0]);
			icpy(ni, hoist, nhoist);
			icpy(ni + nhoist, fn->start->ins, fn->start->nins);
			idup(fn->start, ni, fn->start->nins + nhoist);
		}
		if (sites)
			renumber(fn);
		classify(&cs[f]);
	}
}
