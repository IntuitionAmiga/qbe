#include "all.h"

void
ie64_extsb(Fn *fn)
{
	Blk *b;
	Ins *i;
	Ref r;
	int op;

	for (b=fn->start; b; b=b->link) {
		curi = &insb[NIns];
		if (isretbh(b->jmp.type)) {
			r = newtmp("abi", Kl, fn);
			op = Oextsb + (b->jmp.type - Jretsb);
			emit(op, Kl, r, b->jmp.arg, R);
			b->jmp.arg = r;
			b->jmp.type = Jretl;
		}
		for (i=&b->ins[b->nins]; i>b->ins;) {
			emiti(*--i);
			if (isargbh(i->op)) {
				r = newtmp("abi", Kl, fn);
				curi->op = Oarg;
				curi->cls = Kl;
				curi->arg[0] = r;
				emit(Oextsb + (i->op - Oargsb), Kl, r, i->arg[0], R);
			} else if (isparbh(i->op))
				curi->op = Opar;
		}
		idup(b, curi, &insb[NIns]-curi);
	}
}

/*
 * The call descriptor format is shared with the other native targets.  IE64
 * uses six integer/pointer and eight scalar floating argument registers.
 * Full ABI lowering is added in this file rather than leaking IE64 rules into
 * QBE's target-independent passes.
 */
static void
emitaggchunk(Ref src, Ref dst, int off, int sz, Fn *fn)
{
	Ref s = src, d = dst, v;
	int op;

	if (off) {
		s = newtmp("abi", Kl, fn);
		d = newtmp("abi", Kl, fn);
	}
	switch (sz) {
	case 8: op = Oload; break;
	case 4: op = Oload; break;
	case 2: op = Oloaduh; break;
	case 1: op = Oloadub; break;
	default: die("invalid IE64 aggregate copy size");
	}
	v = newtmp("abi", sz == 8 ? Kl : Kw, fn);
	emit(sz == 8 ? Ostorel : sz == 4 ? Ostorew
		: sz == 2 ? Ostoreh : Ostoreb, Kw, R, v, d);
	emit(op, sz == 8 ? Kl : Kw, v, s, R);
	if (off) {
		emit(Oadd, Kl, d, dst, getcon(off, fn));
		emit(Oadd, Kl, s, src, getcon(off, fn));
	}
}

static void
emitaggmove(Ref src, Ref dst, int sz, Fn *fn)
{
	int off, n;

	for (off=sz; off;) {
		n = off >= 8 ? 8 : off >= 4 ? 4 : off >= 2 ? 2 : 1;
		off -= n;
		emitaggchunk(src, dst, off, n, fn);
	}
}

static void
emitaggcopy(Ref src, Ref dst, int sz, int align, Fn *fn)
{
	emitaggmove(src, dst, sz, fn);
	emit(align >= 4 ? Oalloc16 : Oalloc8, Kl, dst, getcon(sz, fn), R);
}

void
ie64_abi(Fn *fn)
{
	Blk *b;
	Ins *i, *arg0;
	int n, gp, fp, namedstack;

	/* The return-address slot at [R31] makes incoming stack arguments start at
	 * eight bytes.  Integer and floating arguments retain independent cursors. */
	b = fn->start;
	fn->reg = 0;
	n = gp = fp = 0;
	int sp = 0;
	if (fn->retty >= 0) {
		fn->retr = newtmp("abi", Kl, fn);
		fn->reg |= BIT(R1);
		gp = 1;
	}
	for (i=b->ins; i<&b->ins[b->nins] && ispar(i->op); i++) {
		if (i->op == Oparc) {
			i->cls = Kl;
			if (gp < 6) {
				i->op = Ocopy;
				i->arg[0] = TMP(R1 + gp++);
				i->arg[1] = R;
				fn->reg |= BIT(R1 + gp - 1);
			} else {
				i->op = Oload;
				i->arg[0] = SLOT(-(2 + 2 * sp++));
				i->arg[1] = R;
			}
		} else if (i->op == Opare) {
			i->op = Ocopy;
			i->cls = Kl;
			i->arg[0] = TMP(RENV);
			i->arg[1] = R;
			fn->reg |= BIT(RENV);
		} else if (i->op != Opar)
			die("IE64 parameter lowering is not implemented");
		else if (i->cls == Kd) {
			int d = (fp + 1) & -2;
			if (d + 1 < 8) {
				i->op = Ocopy;
				i->arg[0] = TMP(F0 + d);
				i->arg[1] = R;
				fn->reg |= BIT(F0 + d) | BIT(F0 + d + 1);
				fp = d + 2;
			} else {
				i->op = Oload;
				i->arg[0] = SLOT(-(2 + 2 * sp++));
				i->arg[1] = R;
			}
		} else if (i->cls == Ks) {
			if (fp < 8) {
				i->op = Ocopy;
				i->arg[0] = TMP(F0 + fp++);
				i->arg[1] = R;
				fn->reg |= BIT(F0 + fp - 1);
			} else {
				i->op = Oload;
				i->arg[0] = SLOT(-(2 + 2 * sp++));
				i->arg[1] = R;
			}
		} else if (gp < 6) {
			i->op = Ocopy;
			i->arg[0] = TMP(R1 + gp++);
			i->arg[1] = R;
			fn->reg |= BIT(R1 + gp - 1);
		} else {
			i->op = Oload;
			i->arg[0] = SLOT(-(2 + 2 * sp++));
			i->arg[1] = R;
		}
		n++;
	}
	namedstack = sp;
	if (!req(fn->retr, R)) {
		vgrow(&b->ins, b->nins + 1);
		memmove(&b->ins[1], b->ins, b->nins * sizeof b->ins[0]);
		b->ins[0] = (Ins){
			.op = Ocopy, .cls = Kl, .to = fn->retr,
			.arg = {TMP(R1), R},
		};
		b->nins++;
	}

	/* Build call sequences backwards, as QBE's other native ABIs do. */
	for (b=fn->start; b; b=b->link) {
		curi = &insb[NIns];
		for (i=&b->ins[b->nins]; i!=b->ins;) {
			Ins call;

			--i;
			if (i->op == Ovastart) {
				Ref p = newtmp("abi", Kl, fn);
				emit(Ostorel, Kw, R, p, i->arg[0]);
				emit(Oaddr, Kl, p, SLOT(-(2 + 2 * namedstack)), R);
				continue;
			}
			if (i->op == Ovaarg) {
				Ref p = newtmp("abi", Kl, fn);
				Ref next = newtmp("abi", Kl, fn);
				emit(Ostorel, Kw, R, next, i->arg[0]);
				emit(Oadd, Kl, next, p, getcon(8, fn));
				emit(Oload, i->cls, i->to, p, R);
				emit(Oload, Kl, p, i->arg[0], R);
				continue;
			}
			if (i->op == Oie64vaarg) {
				Ref slot = newtmp("abi", Kl, fn);
				Ref src = newtmp("abi", Kl, fn);
				Ref next = newtmp("abi", Kl, fn);
				Con *meta;
				uint64_t packed;
				int size, align, op;

				if (rtype(i->arg[1]) != RCon
				|| (meta = &fn->con[i->arg[1].val])->type != CBits)
					die("invalid IE64 aggregate va_arg metadata");
				packed = meta->bits.i;
				size = packed >> 8;
				align = packed & 0xff;
				if (size <= 0 || (align != 1 && align != 2
				&& align != 4 && align != 8 && align != 16))
					die("invalid IE64 aggregate va_arg layout");
				op = align == 16 ? Oalloc16 : align == 8 ? Oalloc8 : Oalloc4;
				emitaggmove(src, i->to, size, fn);
				emit(Ostorel, Kw, R, next, i->arg[0]);
				emit(Oadd, Kl, next, slot, getcon(8, fn));
				emit(Oload, Kl, src, slot, R);
				emit(Oload, Kl, slot, i->arg[0], R);
				emit(op, Kl, i->to, getcon(size, fn), R);
				continue;
			}
			if (i->op == Oie64syscall) {
				Ins call = *i;
				if (!req(call.to, R))
					emit(Ocopy, Kl, call.to, TMP(R1), R);
				call.to = R;
				call.arg[1] = CALL(1);
				emiti(call);
				continue;
			}
			if (i->op != Ocall) {
				emiti(*i);
				continue;
			}
			for (arg0=i; arg0>b->ins && isarg((arg0-1)->op); arg0--)
				;
			n = i - arg0;
			int ngp = 0, nfp = 0, nstk = 0, stackbytes, fmask = 0;
			int vararg = 0, envc = 0;
			Ref stackbase = R;
			Ref aggret = R;
			int aggretlen = 0, aggretalign = 0;
			int *stack = emalloc((n ? n : 1) * sizeof stack[0]);
			int *fpreg = emalloc((n ? n : 1) * sizeof fpreg[0]);
			Ref *aggsrc = emalloc((n ? n : 1) * sizeof aggsrc[0]);
			Ref *aggcopy = emalloc((n ? n : 1) * sizeof aggcopy[0]);
			int *aggsize = emalloc((n ? n : 1) * sizeof aggsize[0]);
			int *aggalign = emalloc((n ? n : 1) * sizeof aggalign[0]);
			if (rtype(i->arg[1]) == RType) {
				aggretlen = typ[i->arg[1].val].size;
				aggretalign = typ[i->arg[1].val].align;
				aggret = i->to;
				ngp = 1;
			}
			for (Ins *a=arg0; a<i; a++) {
				stack[a-arg0] = -1;
				fpreg[a-arg0] = -1;
				aggsize[a-arg0] = 0;
				aggalign[a-arg0] = 0;
				if (a->op == Oargv) {
					vararg = 1;
					continue;
				}
				if (a->op == Oarge) {
					envc = 1;
					continue;
				}
				if (a->op == Oargc) {
					aggsrc[a-arg0] = a->arg[1];
					aggcopy[a-arg0] = newtmp("abi", Kl, fn);
					aggsize[a-arg0] = typ[a->arg[0].val].size;
					aggalign[a-arg0] = typ[a->arg[0].val].align;
					a->op = Oarg;
					a->cls = Kl;
					a->arg[0] = aggcopy[a-arg0];
				} else if (a->op != Oarg)
					die("IE64 call argument lowering is not implemented");
				if (vararg) {
					stack[a-arg0] = nstk++;
				} else if (a->cls == Kd) {
					int d = (nfp + 1) & -2;
					if (d + 1 < 8) {
						fpreg[a-arg0] = d;
						fmask |= 3 << d;
						nfp = d + 2;
					} else
						stack[a-arg0] = nstk++;
				} else if (a->cls == Ks) {
					if (nfp < 8)
						fpreg[a-arg0] = nfp++, fmask |= 1 << fpreg[a-arg0];
					else
						stack[a-arg0] = nstk++;
				} else if (KBASE(a->cls) == 0) {
					if (ngp < 6)
						ngp++;
					else
						stack[a-arg0] = nstk++;
				}
			}
			stackbytes = (8*nstk + 15) & -16;
			if (stackbytes)
				stackbase = newtmp("abi", Kl, fn);
			call = *i;
			if (aggretlen)
				call.to = R;
			if (stackbytes)
				emit(Osalloc, Kl, R, getcon(-stackbytes, fn), R);
			if (!req(call.to, R))
				emit(Ocopy, call.cls, call.to,
					TMP(KBASE(call.cls) == 1 ? F0 : R1), R);
			else
				emit(Ocopy, Kw, R, TMP(R1), R);
			call.to = R;
			call.arg[1] = CALL((ngp << 4) | (nfp << 8) | (fmask << 12)
				| (envc << 20)
				| (req(i->to, R) ? 1
					: KBASE(call.cls) == 1
						? (call.cls == Kd ? 2 : 1) << 2 : 1));
			emiti(call);
			for (Ins *a=i-1; a>=arg0; a--) {
				if (a->op == Oargv)
					continue;
				if (a->op == Oarge) {
					emit(Ocopy, Kl, TMP(RENV), a->arg[0], R);
					continue;
				}
				if (stack[a-arg0] >= 0) {
					Ref p = stackbase;
					if (stack[a-arg0]) {
						p = newtmp("abi", Kl, fn);
					}
					emit(a->cls == Ks ? Ostores
						: a->cls == Kd ? Ostored : Ostorel,
						Kw, R,
						a->arg[0], p);
					if (stack[a-arg0])
						emit(Oadd, Kl, p, stackbase,
							getcon(8*stack[a-arg0], fn));
				} else if (fpreg[a-arg0] >= 0)
					emit(Ocopy, a->cls, TMP(F0 + fpreg[a-arg0]), a->arg[0], R);
				else
					emit(Ocopy, a->cls, TMP(R1 + --ngp), a->arg[0], R);
				if (aggsize[a-arg0])
					emitaggcopy(aggsrc[a-arg0], aggcopy[a-arg0],
						aggsize[a-arg0], aggalign[a-arg0], fn);
			}
			if (aggretlen) {
				emit(Ocopy, Kl, TMP(R1), aggret, R);
				emit(aggretalign >= 4 ? Oalloc16 : Oalloc8, Kl,
					aggret, getcon(aggretlen, fn), R);
			}
			if (stackbytes)
				emit(Osalloc, Kl, stackbase, getcon(stackbytes, fn), R);
			free(stack);
			free(fpreg);
			free(aggsrc);
			free(aggcopy);
			free(aggsize);
			free(aggalign);
			i = arg0;
		}
		idup(b, curi, &insb[NIns]-curi);
	}

	/* QBE's spill pass requires floating returns to be register copies. */
	for (b=fn->start; b; b=b->link)
		if (b->jmp.type == Jrets || b->jmp.type == Jretd) {
			int nfp = b->jmp.type == Jretd ? 2 : 1;

			vgrow(&b->ins, b->nins + 1);
			b->ins[b->nins++] = (Ins){
				.op = Ocopy, .cls = b->jmp.type == Jretd ? Kd : Ks, .to = TMP(F0),
				.arg = {b->jmp.arg, R},
			};
			b->jmp.type = Jret0;
			b->jmp.arg = CALL(nfp << 2);
		}

	for (b=fn->start; b; b=b->link)
		if (b->jmp.type == Jretc) {
			curi = &insb[NIns];
			emitaggmove(b->jmp.arg, fn->retr, typ[fn->retty].size, fn);
			vgrow(&b->ins, b->nins + (&insb[NIns]-curi));
			icpy(&b->ins[b->nins], curi, &insb[NIns]-curi);
			b->nins += &insb[NIns]-curi;
			b->jmp.type = Jret0;
			b->jmp.arg = R;
		}

	elimsb(fn);
}
