#include "all.h"

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

/* Is an extension a no-op for what is known about its operand? */
static int
extisnop(Ins *i, Fn *fn)
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
				emit(Osltof, Kd, t, i->arg[0], R);
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
			if (i->op == Oultof) {
				Ref bit = newtmp("ie64conv", Kl, fn);
				Ref half = newtmp("ie64conv", Kl, fn);
				Ref rounded = newtmp("ie64conv", Kl, fn);
				Ref d = newtmp("ie64conv", Kd, fn);
				Ref result = i->cls == Kd ? i->to
					: newtmp("ie64conv", Kd, fn);

				if (i->cls == Ks)
					emit(Oie64fcvtds, Ks, i->to, result, R);
				emit(Oadd, Kd, result, d, d);
				emit(Osltof, Kd, d, rounded, R);
				emit(Oor, Kl, rounded, half, bit);
				emit(Oshr, Kl, half, i->arg[0], getcon(1, fn));
				emit(Oand, Kl, bit, i->arg[0], getcon(1, fn));
				continue;
			}
			if (iscmp(i->op, &ck, &cc) && ck == Kw) {
				Cmparg ca[2];
				int sign = cc >= Cisge && cc <= Cislt;

				/* The compare runs on 64 bits: bring both w operands to
				 * the same extension first. */
				cmpplan(i->arg[0], sign, 1, &ca[0], fn);
				cmpplan(i->arg[1], sign, 1, &ca[1], fn);
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
					Ref thresholdcon = getcon(UINT64_C(0x43e0000000000000), fn);
					Ref highw = newtmp("ie64conv", Kw, fn);
					Ref high = newtmp("ie64conv", Kl, fn);
					Ref highfp = newtmp("ie64conv", Kd, fn);
					Ref offset = newtmp("ie64conv", Kd, fn);
					Ref adjusted = newtmp("ie64conv", Kd, fn);
					Ref low = newtmp("ie64conv", Kl, fn);
					Ref highbit = newtmp("ie64conv", Kl, fn);

					emit(Oor, Kl, i->to, low, highbit);
					emit(Oshl, Kl, highbit, high, getcon(63, fn));
					emit(Odtosi, Kl, low, adjusted, R);
					emit(Osub, Kd, adjusted, x, offset);
					emit(Omul, Kd, offset, highfp, threshold);
					emit(Osltof, Kd, highfp, high, R);
					emit(Oextuw, Kl, high, highw, R);
					emit(Ocmpd + Cfge, Kw, highw, x, threshold);
					fn->con[thresholdcon.val].flt = 2;
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
				fixarg(&ii->arg[0], argcls(ii, 0), ii, fn);
				fixarg(&ii->arg[1], argcls(ii, 1), ii, fn);
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
		}
		idup(b, curi, &insb[NIns]-curi);
	}
	if (debug['I']) {
		fprintf(stderr, "\n> After instruction selection:\n");
		printfn(fn, stderr);
	}
}
