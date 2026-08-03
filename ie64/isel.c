#include "all.h"

static int
memarg(Ref *r, Ins *i)
{
	return (isload(i->op) && r == &i->arg[0])
		|| (isstore(i->op) && r == &i->arg[1]);
}

static void
fixarg(Ref *r, int k, Ins *i, Fn *fn)
{
	Ref t;
	int ck, cc;

	if (rtype(*r) == RTmp && !isreg(*r)
	&& fn->tmp[r->val].slot != -1) {
		t = newtmp("isel", Kl, fn);
		emit(Oaddr, Kl, t, SLOT(fn->tmp[r->val].slot), R);
		*r = t;
		return;
	}
	if (rtype(*r) != RCon || memarg(r, i))
		return;
	if (isie64immop(i->op) && r == &i->arg[0])
		return;
	if (i->op == Osalloc)
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

void
ie64_isel(Fn *fn)
{
	Blk *b;
	Ins *i, *ii;
	Ref t;
	int64_t n;
	int a, ck, cc;
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
				i->op = Onop;
			}

	for (b=fn->start; b; b=b->link) {
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
				Ref left = i->arg[0], right = i->arg[1];
				Ref leftsrc = left, rightsrc = right;
				int extop = cc >= Cisge && cc <= Cislt ? Oextsw : Oextuw;

				i->arg[0] = newtmp("ie64cmp", Kl, fn);
				i->arg[1] = newtmp("ie64cmp", Kl, fn);
				i->op += Ocmpl - Ocmpw;
				emiti(*i);
				if (rtype(right) == RCon)
					rightsrc = newtmp("ie64cmp", Kw, fn);
				emit(extop, Kl, i->arg[1], rightsrc, R);
				if (!req(rightsrc, right))
					emit(Ocopy, Kw, rightsrc, right, R);
				if (rtype(left) == RCon)
					leftsrc = newtmp("ie64cmp", Kw, fn);
				emit(extop, Kl, i->arg[0], leftsrc, R);
				if (!req(leftsrc, left))
					emit(Ocopy, Kw, leftsrc, left, R);
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
			emiti(*i);
			ii = curi;
			if (ii->op == Ocopy
			&& rtype(ii->arg[0]) == RTmp
			&& !isreg(ii->arg[0])
			&& fn->tmp[ii->arg[0].val].slot != -1) {
				t = newtmp("isel", Kl, fn);
				emit(Oaddr, Kl, t, SLOT(fn->tmp[ii->arg[0].val].slot), R);
				ii->arg[0] = t;
			} else if (ii->op != Onop && ii->op != Ocopy) {
				fixarg(&ii->arg[0], argcls(ii, 0), ii, fn);
				fixarg(&ii->arg[1], argcls(ii, 1), ii, fn);
			}
		}
		idup(b, curi, &insb[NIns]-curi);
	}
	if (debug['I']) {
		fprintf(stderr, "\n> After instruction selection:\n");
		printfn(fn, stderr);
	}
}
