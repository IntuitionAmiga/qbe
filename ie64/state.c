#include "../all.h"

enum { MaxShare = 12 };

/* Collect the final instructions of an arm, ignoring dead instructions.
 * We can move a shared suffix ahead of the remaining pure operations. */
static int
armsuffix(Blk *b, Ins **v)
{
	Ins *i;
	int n;

	n = 0;
	for (i=&b->ins[b->nins]; i!=b->ins;) {
		--i;
		if (i->op != Onop && n < MaxShare)
			v[n++] = i;
		else if (i->op != Onop && pinned(i))
			return 0;
	}
	return n;
}

static int
samearm(Fn *fn, Ins **a, Ins **b, int count)
{
	Ins *i, *j;
	Ref r;
	Use *u;
	int k, x, n;
	uint m;

	for (n=count-1; n>=0; n--) {
		i = a[n];
		j = b[n];
		if (i->op != j->op || i->cls != j->cls || i->mtype != j->mtype
		|| req(i->to, R) != req(j->to, R))
			return 0;
		/* Calls, volatile accesses and pseudo-operations stay put. */
		if (!(isstore(i->op) && !isvstore(i->op))
		&& !isoptload(i->op) && (pinned(i) || req(i->to, R)))
			return 0;
		for (k=0; k<2; k++) {
			r = j->arg[k];
			if (!req(r, R))
				for (x=n+1; x<count; x++)
					if (req(r, b[x]->to)) {
						r = a[x]->to;
						break;
					}
			if (!req(i->arg[k], r))
				return 0;
		}
		for (k=0; k<2; k++) {
			r = k ? j->to : i->to;
			if (req(r, R))
				continue;
			if (r.val < Tmp0)
				return 0;
			for (m=0; m<fn->tmp[r.val].nuse; m++) {
				u = &fn->tmp[r.val].use[m];
				for (x=0; x<n && u->type == UIns; x++)
					if (u->u.ins == (k ? b[x] : a[x]))
						break;
				if (x == n || u->type != UIns)
					return 0;
			}
		}
	}
	return 1;
}

/* Both arms run the same suffix. Execute it before the branch and
 * leave the arms in place to retain their distinct values and targets. */
static int
mergearms(Fn *fn, Blk *b)
{
	Blk *a, *c;
	Ins *left[MaxShare], *right[MaxShare], *p;
	int na, nc, n, k;

	if (b->jmp.type != Jjnz)
		return 0;
	a = b->s1;
	c = b->s2;
	if (a == c || a == b || c == b || a->npred != 1 || c->npred != 1
	|| a->phi || c->phi)
		return 0;
	na = armsuffix(a, left);
	nc = armsuffix(c, right);
	for (n=na<nc ? na : nc; n>0; n--) {
		for (k=n; k<na && !pinned(left[k]); k++)
			;
		if (k < na)
			return 0;
		for (k=n; k<nc && !pinned(right[k]); k++)
			;
		if (k < nc)
			return 0;
		if (samearm(fn, left, right, n))
			break;
	}
	if (!n)
		return 0;
	p = vnew(b->nins + n, sizeof *p, PFn);
	icpy(p, b->ins, b->nins);
	for (k=n-1; k>=0; k--) {
		p[b->nins++] = *left[k];
		*left[k] = *right[k] = (Ins){.op = Onop};
	}
	b->ins = p;
	filluse(fn);
	return 1;
}

enum { MaxBlocks = 256, MaxStates = 24, MaxVersions = 144 };
typedef struct Version Version;
typedef struct Thread Thread;

struct Version {
	Blk *src, *out;
	Ref val[MaxStates];
	int first, last;
	Version *edge[2];
};

struct Thread {
	Fn *fn;
	Blk *head;
	Phi *state[MaxStates];
	int nstate, nversion, nregion, limit, folded;
	unsigned char region[MaxBlocks];
	Version version[MaxVersions];
	Version *entry[MaxBlocks];
};

static int
stateid(Thread *t, Ref r)
{
	int n;

	for (n=0; n<t->nstate; n++)
		if (req(t->state[n]->to, r))
			return n;
	return -1;
}

static int
intcon(Fn *fn, Ref r)
{
	return rtype(r) == RCon && fn->con[r.val].type == CBits
		&& !fn->con[r.val].flt;
}

/* Trace the finite set of values fed back to the header through phis.
 * Any value produced by general arithmetic leaves the loop alone. */
static int
track(Thread *t, Ref r)
{
	Phi *p;
	uint n, bid;

	if (intcon(t->fn, r) || stateid(t, r) >= 0)
		return 1;
	if (rtype(r) != RTmp || r.val < Tmp0 || t->nstate == MaxStates)
		return 0;
	bid = t->fn->tmp[r.val].bid;
	if (bid >= t->fn->nblk || !t->region[bid])
		return 0;
	for (p=t->fn->rpo[bid]->phi; p && !req(p->to, r); p=p->link)
		;
	if (!p || (p->cls != Kw && p->cls != Kl))
		return 0;
	t->state[t->nstate++] = p;
	for (n=0; n<p->narg; n++)
		if (!track(t, p->arg[n]))
			return 0;
	return 1;
}

static Ref
known(Thread *t, Version *v, Ref r, int depth)
{
	Ins *i;
	Con c;
	Ref a, b;
	int n, k, cmp;

	if (intcon(t->fn, r))
		return r;
	if (!v || !depth || rtype(r) != RTmp || r.val < Tmp0)
		return R;
	n = stateid(t, r);
	if (n >= 0)
		return v->val[n];
	i = t->fn->tmp[r.val].def;
	if (!i)
		return R;
	if (i->op == Ocopy && i->cls == Kw) {
		a = known(t, v, i->arg[0], depth-1);
		return req(a, R) ? R : getcon((int32_t)t->fn->con[a.val].bits.i, t->fn);
	}
	if (!iscmp(i->op, &k, &cmp) || (k != Kw && k != Kl))
		return R;
	a = known(t, v, i->arg[0], depth-1);
	b = known(t, v, i->arg[1], depth-1);
	if (req(a, R) || req(b, R))
		return R;
	if (foldint(&c, i->op, 0, &t->fn->con[a.val], &t->fn->con[b.val]))
		return R;
	return getcon(c.bits.i, t->fn);
}

static Version *
version(Thread *t, Version *v, Blk *from, Blk *to)
{
	Version *w;
	Ref val[MaxStates] = {R};
	Ref r;
	Phi *p;
	int n, k;

	for (n=0; n<t->nstate; n++) {
		p = t->state[n];
		if (t->fn->tmp[p->to.val].bid == to->id) {
			r = known(t, v, phiarg(p, from), 4);
			if (req(r, R))
				return 0;
			if (p->cls == Kw)
				r = getcon((int32_t)t->fn->con[r.val].bits.i, t->fn);
			val[n] = r;
		} else if (v && bshas(to->in, p->to.val))
			val[n] = v->val[n];
	}
	for (n=0; n<t->nversion; n++) {
		w = &t->version[n];
		if (w->src != to)
			continue;
		for (k=0; k<t->nstate && req(w->val[k], val[k]); k++)
			;
		if (k == t->nstate)
			return w;
	}
	if (t->nversion == t->limit)
		return 0;
	w = &t->version[t->nversion++];
	w->src = to;
	memcpy(w->val, val, sizeof val);
	return w;
}

static int
natural(Thread *t, Blk *b)
{
	uint n;

	if (t->region[b->id])
		return 1;
	if (!dom(t->head, b))
		return 0;
	t->region[b->id] = 1;
	t->nregion++;
	for (n=0; n<b->npred; n++)
		if (!natural(t, b->pred[n]))
			return 0;
	return 1;
}

static int
plan(Thread *t, Phi *p)
{
	Version *v;
	Blk *h, *b, *s[2];
	Ins *i;
	Ref r;
	int n, k, nback;
	uint cost;

	h = t->head;
	t->region[h->id] = 1;
	t->nregion = 1;
	for (n=0; n<(int)h->npred; n++) {
		b = h->pred[n];
		if (dom(h, b) && !natural(t, b))
			return 0;
	}
	if (t->nregion < 2 || !track(t, p->to))
		return 0;
	t->limit = 2*t->nregion + 12;
	if (t->limit > MaxVersions)
		t->limit = MaxVersions;
	for (n=0; n<(int)h->npred; n++) {
		b = h->pred[n];
		if (!t->region[b->id]) {
			t->entry[b->id] = version(t, 0, b, h);
			if (!t->entry[b->id])
				return 0;
		}
	}
	nback = 0;
	cost = 0;
	for (n=0; n<t->nversion; n++) {
		v = &t->version[n];
		b = v->src;
		cost += b->nins;
		if (cost > 2048)
			return 0;
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (isalloc(i->op))
				return 0;
		s[0] = b->s1;
		s[1] = b->s2;
		v->first = 0;
		v->last = s[1] ? 2 : !!s[0];
		if (b->jmp.type == Jjnz) {
			r = known(t, v, b->jmp.arg, 4);
			if (!req(r, R)) {
				v->first = !(uint32_t)t->fn->con[r.val].bits.i;
				v->last = v->first + 1;
				t->folded++;
			}
		}
		for (k=v->first; k<v->last; k++)
			if (t->region[s[k]->id]) {
				v->edge[k] = version(t, v, b, s[k]);
				if (!v->edge[k])
					return 0;
				nback += s[k] == h;
			}
	}
	return t->folded && nback;
}

/* A copied edge performs the same parallel phi assignments as the
 * original edge. Rename the resulting multi-definition IL with SSA
 * construction, including live values used after the loop. */
static void
edgecopy(Fn *fn, Blk **edge, Blk *from, Blk *to)
{
	Blk *b;
	Phi *p;
	Ins *i;
	uint n;

	if (!to || !to->phi)
		return;
	b = newblk();
	b->name = strf(PFn, "%s.edge", from->name);
	b->s1 = *edge;
	b->jmp.type = Jjmp;
	b->link = fn->start->link;
	fn->start->link = b;
	*edge = b;
	for (p=to->phi; p; p=p->link) {
		Ins save = {.op = Ocopy, .cls = p->cls, .arg = {phiarg(p, from), R}};
		save.to = newtmp("edge", p->cls, fn);
		addins(&b->ins, &b->nins, &save);
	}
	n = 0;
	for (p=to->phi; p; p=p->link) {
		i = &b->ins[n++];
		Ins set = {.op = Ocopy, .cls = p->cls, .to = p->to, .arg = {i->to, R}};
		addins(&b->ins, &b->nins, &set);
	}
}

static void
materialise(Thread *t)
{
	Fn *fn;
	Blk *b, *s, **edge, *src;
	Version *v;
	int n, k;

	fn = t->fn;
	/* Save the original CFG for exact phi-edge lookup. */
	for (n=0; n<t->nversion; n++) {
		v = &t->version[n];
		b = newblk();
		b->name = strf(PFn, "%s.state.%d", v->src->name, n);
		b->jmp = v->src->jmp;
		idup(b, v->src->ins, v->src->nins);
		v->out = b;
	}
	for (n=0; n<(int)fn->nblk; n++) {
		b = fn->rpo[n];
		if (t->region[n])
			continue;
		for (k=0; k<2; k++) {
			edge = k ? &b->s2 : &b->s1;
			s = *edge;
			if (s == t->head)
				*edge = t->entry[n]->out;
			edgecopy(fn, edge, b, s);
		}
	}
	for (n=0; n<t->nversion; n++) {
		v = &t->version[n];
		src = v->src;
		b = v->out;
		b->link = fn->start->link;
		fn->start->link = b;
		if (src->jmp.type == Jjnz && v->last-v->first == 1) {
			b->jmp.type = Jjmp;
			b->jmp.arg = R;
		}
		for (k=v->first; k<v->last; k++) {
			s = k ? src->s2 : src->s1;
			edge = k == v->first ? &b->s1 : &b->s2;
			*edge = v->edge[k] ? v->edge[k]->out : s;
			edgecopy(fn, edge, src, s);
		}
	}
	for (b=fn->start; b; b=b->link)
		b->phi = 0;
	fillcfg(fn);
	filluse(fn);
	ssa(fn);
	filluse(fn);
	ssacheck(fn);
}

int
ie64_threadstate(Fn *fn)
{
	Blk *b;
	Phi *p;
	Thread *t;
	uint a, n;

	for (n=fn->nblk; n--;) {
		b = fn->rpo[n];
		while (mergearms(fn, b))
			;
	}
	if (fn->nblk > MaxBlocks || fn->ntmp > 2048)
		return 0;
	t = 0;
	for (b=fn->start->link; b; b=b->link)
		for (p=b->phi; p; p=p->link) {
			if (p->cls != Kw && p->cls != Kl)
				continue;
			for (a=0; a<p->narg; a++)
				if (!dom(b, p->blk[a]) && intcon(fn, p->arg[a]))
					break;
			if (a == p->narg)
				continue;
			for (a=0; a<p->narg; a++)
				if (dom(b, p->blk[a]))
					break;
			if (a == p->narg)
				continue;
			if (!t) {
				t = emalloc(sizeof *t);
				filllive(fn);
			} else
				memset(t, 0, sizeof *t);
			t->fn = fn;
			t->head = b;
			if (plan(t, p)) {
				materialise(t);
				free(t);
				return 1;
			}
		}
	free(t);
	return 0;
}
