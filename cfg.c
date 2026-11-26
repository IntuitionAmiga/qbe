#include "all.h"

Blk *
newblk()
{
	static Blk z;
	Blk *b;

	b = alloc(sizeof *b);
	*b = z;
	b->ins = vnew(0, sizeof b->ins[0], PFn);
	b->pred = vnew(0, sizeof b->pred[0], PFn);
	return b;
}

static Blk **
edgeslot(Blk **s1, Blk **s2, JTab *tab, uint n)
{
	if (tab)
		return n < tab->nsucc ? &tab->succ[n] : 0;
	if (n == 0 && *s1)
		return s1;
	if (n == 1 && *s2)
		return s2;
	return 0;
}

Blk **
succedge(Blk *b, uint n)
{
	return edgeslot(&b->s1, &b->s2, b->jmp.tab, n);
}

Blk *
succ(Blk *b, uint n)
{
	Blk **p;

	p = succedge(b, n);
	return p ? *p : 0;
}

static int
hassucc(Blk *b, Blk *target)
{
	Blk *s;
	uint n;

	for (n=0; (s=succ(b, n)); n++)
		if (s == target)
			return 1;
	return 0;
}

/* Keep the dispatch in the caller on a call-free block. The inliner
 * splits ordinary call blocks and leaves switch callees out of line. */
void
splitcallswitches(Fn *fn)
{
	Blk *b, *d, *s;
	Ins *i;
	Phi *p;
	uint n, j;

	for (b=fn->start; b; b=b->link) {
		if (!b->jmp.tab)
			continue;
		for (i=b->ins; i<&b->ins[b->nins] && i->op != Ocall; i++)
			;
		if (i == &b->ins[b->nins])
			continue;
		d = newblk();
		d->name = strf(PFn, "%s.switch", b->name);
		d->jmp = b->jmp;
		for (n=0; (s=succ(d, n)); n++)
			for (p=s->phi; p; p=p->link)
				for (j=0; j<p->narg; j++)
					if (p->blk[j] == b)
						p->blk[j] = d;
		d->link = b->link;
		b->link = d;
		b->jmp.type = Jjmp;
		b->jmp.arg = R;
		b->jmp.tab = 0;
		b->s1 = d;
		b = d;
	}
}

static void
fixphis(Fn *f)
{
	Blk *b, *bp;
	Phi *p;
	uint n, n0;

	for (b=f->start; b; b=b->link) {
		assert(b->id < f->nblk);
		for (p=b->phi; p; p=p->link) {
			for (n=n0=0; n<p->narg; n++) {
				bp = p->blk[n];
				if (bp->id != -1u)
				if (hassucc(bp, b)) {
					p->blk[n0] = bp;
					p->arg[n0] = p->arg[n];
					n0++;
				}
			}
			assert(n0 > 0);
			p->narg = n0;
		}
	}
}

static void
addpred(Blk *bp, Blk *b)
{
	vgrow(&b->pred, ++b->npred);
	b->pred[b->npred-1] = bp;
}

void
fillpreds(Fn *f)
{
	Blk *b, *s;
	uint n;

	for (b=f->start; b; b=b->link)
		b->npred = 0;
	for (b=f->start; b; b=b->link)
		for (n=0; (s=succ(b, n)); n++)
			if (!s->npred || s->pred[s->npred-1] != b)
				addpred(b, s);
}

static void
porec(Blk *b, uint *npo)
{
	Blk *s1, *s2;
	uint n;

	if (!b || b->id != -1u)
		return;
	b->id = 0; /* marker */
	s1 = b->s1;
	s2 = b->s2;
	if (s1 && s2 && s1->loop > s2->loop) {
		s1 = b->s2;
		s2 = b->s1;
	}
	if (b->jmp.tab)
		for (n=0; n<b->jmp.tab->nsucc; n++)
			porec(b->jmp.tab->succ[n], npo);
	else {
		porec(s1, npo);
		porec(s2, npo);
	}
	b->id = (*npo)++;
}

static void
fillrpo(Fn *f)
{
	Blk *b, **p;

	for (b=f->start; b; b=b->link)
		b->id = -1u;
	f->nblk = 0;
	porec(f->start, &f->nblk);
	vgrow(&f->rpo, f->nblk);
	for (p=&f->start; (b=*p);) {
		if (b->id == -1u) {
			*p = b->link;
		} else {
			b->id = f->nblk-b->id-1;
			f->rpo[b->id] = b;
			p = &b->link;
		}
	}
}

/* fill rpo, preds; prune dead blks */
void
fillcfg(Fn *f)
{
	Blk *b;
	JTab *t;
	int64_t i;

	for (b=f->start; b; b=b->link)
		if ((t=b->jmp.tab) && isconbits(f, b->jmp.arg, &i)) {
			b->s1 = t->succ[(uint64_t)i < t->nentry ? t->entry[i] : 0];
			b->jmp.type = Jjmp;
			b->jmp.arg = R;
			b->jmp.tab = 0;
		}
	fillrpo(f);
	fillpreds(f);
	fixphis(f);
}

/* for dominators computation, read
 * "A Simple, Fast Dominance Algorithm"
 * by K. Cooper, T. Harvey, and K. Kennedy.
 */

static Blk *
inter(Blk *b1, Blk *b2)
{
	Blk *bt;

	if (b1 == 0)
		return b2;
	while (b1 != b2) {
		if (b1->id < b2->id) {
			bt = b1;
			b1 = b2;
			b2 = bt;
		}
		while (b1->id > b2->id) {
			b1 = b1->idom;
			assert(b1);
		}
	}
	return b1;
}

void
filldom(Fn *fn)
{
	Blk *b, *d;
	int ch;
	uint n, p;

	for (b=fn->start; b; b=b->link) {
		b->idom = 0;
		b->dom = 0;
		b->dlink = 0;
	}
	do {
		ch = 0;
		for (n=1; n<fn->nblk; n++) {
			b = fn->rpo[n];
			d = 0;
			for (p=0; p<b->npred; p++)
				if (b->pred[p]->idom
				||  b->pred[p] == fn->start)
					d = inter(d, b->pred[p]);
			if (d != b->idom) {
				ch++;
				b->idom = d;
			}
		}
	} while (ch);
	for (b=fn->start; b; b=b->link)
		if ((d=b->idom)) {
			assert(d != b);
			b->dlink = d->dom;
			d->dom = b;
		}
}

int
sdom(Blk *b1, Blk *b2)
{
	assert(b1 && b2);
	if (b1 == b2)
		return 0;
	while (b2->id > b1->id)
		b2 = b2->idom;
	return b1 == b2;
}

int
dom(Blk *b1, Blk *b2)
{
	return b1 == b2 || sdom(b1, b2);
}

static void
addfron(Blk *a, Blk *b)
{
	uint n;

	for (n=0; n<a->nfron; n++)
		if (a->fron[n] == b)
			return;
	if (!a->nfron)
		a->fron = vnew(++a->nfron, sizeof a->fron[0], PFn);
	else
		vgrow(&a->fron, ++a->nfron);
	a->fron[a->nfron-1] = b;
}

/* fill the dominance frontier */
void
fillfron(Fn *fn)
{
	Blk *a, *b, *s;
	uint n;

	for (b=fn->start; b; b=b->link)
		b->nfron = 0;
	for (b=fn->start; b; b=b->link)
		for (n=0; (s=succ(b, n)); n++)
			for (a=b; !sdom(a, s); a=a->idom)
				addfron(a, s);
}

static void
loopmark(Blk *hd, Blk *b, void f(Blk *, Blk *))
{
	uint p;

	if (b->id < hd->id || b->visit == hd->id)
		return;
	b->visit = hd->id;
	f(hd, b);
	for (p=0; p<b->npred; ++p)
		loopmark(hd, b->pred[p], f);
}

void
loopiter(Fn *fn, void f(Blk *, Blk *))
{
	uint n, p;
	Blk *b;

	for (b=fn->start; b; b=b->link)
		b->visit = -1u;
	for (n=0; n<fn->nblk; ++n) {
		b = fn->rpo[n];
		for (p=0; p<b->npred; ++p)
			if (b->pred[p]->id >= n)
				loopmark(b, b->pred[p], f);
	}
}

/* dominator tree depth */
void
filldepth(Fn *fn)
{
	Blk *b, *d;
	int depth;

	for (b=fn->start; b; b=b->link)
		b->depth = -1;

	fn->start->depth = 0;

	for (b=fn->start; b; b=b->link) {
		if (b->depth != -1)
			continue;
		depth = 1;
		for (d=b->idom; d->depth==-1; d=d->idom)
			depth++;
		depth += d->depth;
		b->depth = depth;
		for (d=b->idom; d->depth==-1; d=d->idom)
			d->depth = --depth;
	}
}

/* least common ancestor in dom tree */
Blk *
lca(Blk *b1, Blk *b2)
{
	if (!b1)
		return b2;
	if (!b2)
		return b1;
	while (b1->depth > b2->depth)
		b1 = b1->idom;
	while (b2->depth > b1->depth)
		b2 = b2->idom;
	while (b1 != b2) {
		b1 = b1->idom;
		b2 = b2->idom;
	}
	return b1;
}

void
multloop(Blk *hd, Blk *b)
{
	(void)hd;
	b->loop *= 10;
}

void
fillloop(Fn *fn)
{
	Blk *b;

	for (b=fn->start; b; b=b->link)
		b->loop = 1;
	loopiter(fn, multloop);
}

static void
uffind(Blk **pb, Blk **uf)
{
	Blk **pb1;

	pb1 = &uf[(*pb)->id];
	if (*pb1) {
		uffind(pb1, uf);
		*pb = *pb1;
	}
}

/* requires rpo and no phis, breaks cfg */
void
simpljmp(Fn *fn)
{

	Blk **uf; /* union-find */
	Blk **p, **edge, *b, *ret;
	uint n;

	ret = newblk();
	ret->id = fn->nblk++;
	ret->jmp.type = Jret0;
	uf = emalloc(fn->nblk * sizeof uf[0]);
	for (b=fn->start; b; b=b->link) {
		assert(!b->phi);
		if (b->jmp.type == Jret0) {
			b->jmp.type = Jjmp;
			b->s1 = ret;
		}
		if (b->nins == 0)
		if (b->jmp.type == Jjmp) {
			uffind(&b->s1, uf);
			if (b->s1 != b)
				uf[b->id] = b->s1;
		}
	}
	for (p=&fn->start; (b=*p); p=&b->link) {
		for (n=0; (edge=succedge(b, n)); n++)
			uffind(edge, uf);
		if (b->jmp.tab) {
			JTab *tab = b->jmp.tab;

			for (n=1; n<tab->nsucc && tab->succ[n] == tab->succ[0]; n++)
				;
			if (n == tab->nsucc) {
				b->jmp.type = Jjmp;
				b->jmp.arg = R;
				b->jmp.tab = 0;
				b->s1 = tab->succ[0];
			}
		}
		if (b->s1 && b->s1 == b->s2) {
			b->jmp.type = Jjmp;
			b->s2 = 0;
		}
	}
	*p = ret;
	free(uf);
}

static int
reachrec(Blk *b, Blk *to)
{
	Blk *s;
	uint n;

	if (b == to)
		return 1;
	if (!b || b->visit)
		return 0;

	b->visit = 1;
	for (n=0; (s=succ(b, n)); n++)
		if (reachrec(s, to))
			return 1;

	return 0;
}

/* Blk.visit needs to be clear at entry */
int
reaches(Fn *fn, Blk *b, Blk *to)
{
	int r;

	assert(to);
	r = reachrec(b, to);
	for (b=fn->start; b; b=b->link)
		b->visit = 0;
	return r;
}

/* can b reach 'to' not through excl
 * Blk.visit needs to be clear at entry */
int
reachesnotvia(Fn *fn, Blk *b, Blk *to, Blk *excl)
{
	excl->visit = 1;
	return reaches(fn, b, to);
}

int
ifgraph(Blk *ifb, Blk **pthenb, Blk **pelseb, Blk **pjoinb)
{
	Blk *s1, *s2, **t;

	if (ifb->jmp.type != Jjnz)
		return 0;

	s1 = ifb->s1;
	s2 = ifb->s2;
	if (s1->id > s2->id) {
		s1 = ifb->s2;
		s2 = ifb->s1;
		t = pthenb;
		pthenb = pelseb;
		pelseb = t;
	}
	if (s1 == s2)
		return 0;

	if (s1->jmp.type != Jjmp || s1->npred != 1)
		return 0;

	if (s1->s1 == s2) {
		/* if-then / if-else */
		if (s2->npred != 2)
			return 0;
		*pthenb = s1;
		*pelseb = ifb;
		*pjoinb = s2;
		return 1;
	}

	if (s2->jmp.type != Jjmp || s2->npred != 1)
		return 0;
	if (s1->s1 != s2->s1 || s1->s1->npred != 2)
		return 0;

	assert(s1->s1 != ifb);
	*pthenb = s1;
	*pelseb = s2;
	*pjoinb = s1->s1;
	return 1;
}

typedef struct Jmp Jmp;

struct Jmp {
	int type;
	Ref arg;
	Blk *s1, *s2;
	JTab *tab;
};

static int
jmpeq(Jmp *a, Jmp *b)
{
	return !a->tab && !b->tab && a->type == b->type && req(a->arg, b->arg)
		&& a->s1 == b->s1 && a->s2 == b->s2;
}

static int
jmpnophi(Jmp *j)
{
	if (j->s1 && j->s1->phi)
		return 0;
	if (j->s2 && j->s2->phi)
		return 0;
	return 1;
}

/* require cfg rpo, breaks use */
void
simplcfg(Fn *fn)
{
	Ins cpy, *i;
	Blk *b, *bb, **pb;
	Jmp *jmp, *j, *jj;
	Phi *p;
	int *empty, done;
	uint n, k;

	if (debug['C']) {
		fprintf(stderr, "\n> Before CFG simplification:\n");
		printfn(fn, stderr);
	}

	cpy = (Ins){.op = Ocopy};
	for (b=fn->start; b; b=b->link)
		if (b->npred == 1) {
			bb = b->pred[0];
			for (p=b->phi; p; p=p->link) {
				cpy.cls = p->cls;
				cpy.to = p->to;
				cpy.arg[0] = phiarg(p, bb);
				addins(&bb->ins, &bb->nins, &cpy);
			}
			b->phi = 0;
		}

	jmp = emalloc(fn->nblk * sizeof jmp[0]);
	empty = emalloc(fn->nblk * sizeof empty[0]);
	for (b=fn->start; b; b=b->link) {
		jmp[b->id].type = b->jmp.type;
		jmp[b->id].arg = b->jmp.arg;
		jmp[b->id].s1 = b->s1;
		jmp[b->id].s2 = b->s2;
		jmp[b->id].tab = b->jmp.tab;
		empty[b->id] = !b->phi;
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (i->op != Onop && i->op != Odbgloc) {
				empty[b->id] = 0;
				break;
			}
	}

	do {
		done = 1;
		for (b=fn->start; b; b=b->link) {
			if (b->id == -1u)
				continue;
			j = &jmp[b->id];
			if (j->type == Jjmp && j->s1->npred == 1) {
				assert(!j->s1->phi);
				addbins(&b->ins, &b->nins, j->s1);
				empty[b->id] &= empty[j->s1->id];
				jj = &jmp[j->s1->id];
				for (k=0; (pb=edgeslot(&jj->s1, &jj->s2, jj->tab, k)); k++)
					for (p=(*pb)->phi; p; p=p->link) {
						n = phiargn(p, j->s1);
						p->blk[n] = b;
					}
				j->s1->id = -1u;
				*j = *jj;
				done = 0;
			}
			else if (j->type == Jjnz
			&& empty[j->s1->id] && empty[j->s2->id]
			&& jmpeq(&jmp[j->s1->id], &jmp[j->s2->id])
			&& jmpnophi(&jmp[j->s1->id])) {
				*j = jmp[j->s1->id];
				done = 0;
			}
		}
	} while (!done);

	for (b=fn->start; b; b=b->link)
		if (b->id != -1u) {
			j = &jmp[b->id];
			b->jmp.type = j->type;
			b->jmp.arg = j->arg;
			b->s1 = j->s1;
			b->s2 = j->s2;
			b->jmp.tab = j->tab;
			for (k=0; (bb=succ(b, k)); k++)
				assert(bb->id != -1u);
		}

	fillcfg(fn);
	free(empty);
	free(jmp);

	if (debug['C']) {
		fprintf(stderr, "\n> After CFG simplification:\n");
		printfn(fn, stderr);
	}
}
