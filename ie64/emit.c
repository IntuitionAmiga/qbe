#include "all.h"

typedef struct SavedDat SavedDat;
typedef struct FloatCon FloatCon;
struct SavedDat {
	Dat d;
	Lnk lnk;
	SavedDat *next;
};

static SavedDat *datahead, **datatail = &datahead;
struct FloatCon {
	unsigned id, flt;
	uint64_t bits;
	FloatCon *next;
};
static FloatCon *floathead, **floattail = &floathead;

static int
rawfloatbits(int cls, uint64_t bits)
{
	if (cls == Ks)
		return (bits & UINT64_C(0x7f800000)) != 0;
	return (bits & UINT64_C(0x7ff0000000000000)) != 0;
}
static unsigned floatid;
static int emitframe, emitdynalloc;

static char *
copystr(char *s)
{
	char *p;

	if (!s)
		return 0;
	p = malloc(strlen(s) + 1);
	if (!p)
		die("out of memory");
	return strcpy(p, s);
}

void
ie64_emitdat(Dat *d, FILE *f)
{
	SavedDat *s;

	(void)f;
	s = malloc(sizeof *s);
	if (!s)
		die("out of memory");
	s->d = *d;
	if (d->type == DStart || d->type == DEnd) {
		s->d.isstr = 0;
		s->d.isref = 0;
		memset(&s->d.u, 0, sizeof s->d.u);
	}
	if (d->lnk)
		s->lnk = *d->lnk;
	else
		s->lnk = (Lnk){0};
	/* The section names live in the parser pool, which func() frees before
	 * ie64_emitfin runs.  Copy them like the other retained strings. */
	s->lnk.sec = copystr(s->lnk.sec);
	s->lnk.secf = copystr(s->lnk.secf);
	s->d.name = d->type == DStart ? copystr(d->name) : 0;
	s->d.lnk = &s->lnk;
	/* Test the retained flags, not the incoming ones: DStart and DEnd clear
	 * them above, and the parser leaves them stale from the previous record.
	 * Copying on the raw flags allocates strings the free below never sees. */
	if (s->d.isstr)
		s->d.u.str = copystr(d->u.str);
	if (s->d.isref)
		s->d.u.ref.name = copystr(d->u.ref.name);
	s->next = 0;
	*datatail = s;
	datatail = &s->next;
}

static void
emitlabel(SavedDat *s, FILE *f)
{
	fprintf(f, "%s %s\n", s->lnk.weak ? ".weak" : s->lnk.export ? ".global" : ".local", s->d.name);
	if (s->lnk.hidden)
		fprintf(f, ".hidden %s\n", s->d.name);
	fprintf(f, ".type %s,@object\n", s->d.name);
	if (s->lnk.align > 1)
		fprintf(f, ".align %d\n", s->lnk.align);
	fprintf(f, "%s:\n", s->d.name);
}

static void
emitdata(SavedDat *s, FILE *f)
{
	Dat *d = &s->d;
	char size;

	switch (d->type) {
	case DStart:
		emitlabel(s, f);
		break;
	case DEnd:
		break;
	case DZ:
		fprintf(f, "\tds.b %"PRId64"\n", d->u.num);
		break;
	case DB: size = 'b'; goto Data;
	case DH: size = 'w'; goto Data;
	case DW: size = 'l'; goto Data;
	case DL: size = 'q'; goto Data;
	}
	return;

Data:
	if (d->isstr)
		fprintf(f, "\tdc.b %s\n", d->u.str);
	else if (d->isref)
		fprintf(f, "\tdc.%c %s%+"PRId64"\n", size,
			d->u.ref.name, d->u.ref.off);
	else
		fprintf(f, "\tdc.%c %"PRId64"\n", size, d->u.num);
}

static char
suffix(int cls)
{
	if (cls == Kw)
		return 'l';
	if (cls == Kl)
		return 'q';
	die("IE64 integer emitter received non-integer class");
	return 0;
}

static void
emitint(int64_t n, FILE *f)
{
	if (n == INT64_MIN)
		fputs("0x8000000000000000", f);
	else
		fprintf(f, "%"PRId64, n);
}

static void emitreg(Ref, FILE *);

static void
emitintload(Ref dst, int cls, int64_t n, FILE *f)
{
	uint64_t bits = (uint64_t)n;

	/* One instruction where one will do.  moveq sign-extends a 32-bit
	 * immediate; move.l zero-extends one.  A w constant is always the
	 * zero-extended 32-bit value, which is the form the facts in ext.c
	 * and the 32-bit shifts and divisions rely on. */
	if (cls == Kw)
		bits = (uint32_t)bits;
	if (cls == Kw ? bits <= INT32_MAX : n >= INT32_MIN && n <= INT32_MAX) {
		fputs("moveq ", f);
		emitreg(dst, f);
		fprintf(f, ", #%"PRId32"\n", (int32_t)bits);
		return;
	}
	if (bits <= UINT32_MAX) {
		fputs("move.l ", f);
		emitreg(dst, f);
		fprintf(f, ", #0x%08"PRIx32"\n", (uint32_t)bits);
		return;
	}
	fputs("move.l ", f);
	emitreg(dst, f);
	fprintf(f, ", #0x%08"PRIx32"\n\tmovt ", (uint32_t)bits);
	emitreg(dst, f);
	fprintf(f, ", #0x%08"PRIx32"\n", (uint32_t)(bits >> 32));
}

static void
emitlbl(int n, FILE *f)
{
	fprintf(f, "%s%d", T.asloc, n);
}

static void
emitreg(Ref r, FILE *f)
{
	if (!isreg(r))
		die("IE64 instruction selection left a non-register operand");
	if (r.val >= F0)
		fprintf(f, "f%u", r.val - F0);
	else
		fprintf(f, "r%u", r.val);
}

/* A compare operand: a register, or the constant zero as r0. */
static int
iszeroref(Ref r, Fn *fn)
{
	Con *c;

	return rtype(r) == RCon && (c = &fn->con[r.val])->type == CBits
		&& !c->flt && c->bits.i == 0;
}

static void
emitcmpreg(Ref r, Fn *fn, FILE *f)
{
	if (iszeroref(r, fn))
		fputs("r0", f);
	else
		emitreg(r, f);
}

static void
emitaddr(Ref r, char *name, int64_t addend, FILE *f)
{
	fputs("move.l ", f);
	emitreg(r, f);
	fprintf(f, ", #lo32(%s%+"PRId64")\n\tmovt ", name, addend);
	emitreg(r, f);
	fprintf(f, ", #hi32(%s%+"PRId64")\n", name, addend);
}

static char *
floatlabel(unsigned id)
{
	return strf(PFn, ".L__ie64_fconst_%u", id);
}

static char *
stashfloat(Con *c)
{
	FloatCon *fc;
	union { float f; double d; uint64_t u; } bits;

	assert(c->flt == 1 || c->flt == 2);
	bits.u = 0;
	if (c->flt == 1)
		bits.f = c->bits.s;
	else
		bits.d = c->bits.d;
	for (fc=floathead; fc; fc=fc->next)
		if (fc->flt == (unsigned char)c->flt && fc->bits == bits.u)
			return floatlabel(fc->id);
	fc = emalloc(sizeof *fc);
	fc->id = floatid++;
	fc->flt = c->flt;
	fc->bits = bits.u;
	fc->next = 0;
	*floattail = fc;
	floattail = &fc->next;
	return floatlabel(fc->id);
}

static void
emitvalue(Ref r, Fn *fn, FILE *f)
{
	Con *c;

	if (isreg(r)) {
		emitreg(r, f);
		return;
	}
	if (rtype(r) != RCon)
		die("IE64 instruction selection left an invalid operand");
	c = &fn->con[r.val];
	if (c->type != CBits)
		die("IE64 address materialisation is not selected");
	fputc('#', f);
	emitint(c->bits.i, f);
}

static int slotoffset(Ref);
static int slotbase(Ref);

/* Displacement folded into the memory instruction being emitted by
 * foldaddrs(); zero when there is none. */
static int64_t emitoff;

static void
emitmem(Ref r, FILE *f)
{
	if (emitoff)
		fprintf(f, "%"PRId64, emitoff);
	fputc('(', f);
	emitreg(r, f);
	fputc(')', f);
}

static void
emitcopy(Ins *i, Fn *fn, FILE *f)
{
	Con *c;
	Ref src;
	int off;
	char s;

	if (rtype(i->to) == RSlot) {
		off = slotoffset(i->to);
		src = i->arg[0];
		if (rtype(src) == RSlot) {
			fprintf(f, "\tload.%c r30, %d(r%d)\n", i->cls == Ks || i->cls == Kw ? 'l' : 'q',
				slotoffset(src), slotbase(src));
			fprintf(f, "\tstore.%c r30, %d(r%d)\n", i->cls == Ks || i->cls == Kw ? 'l' : 'q', off,
				slotbase(i->to));
			return;
		}
		if (isreg(src)) {
			if (src.val >= F0)
				fprintf(f, "\t%sstore ", i->cls == Kd ? "d" : "f");
			else
				fprintf(f, "\tstore.%c ", suffix(i->cls));
			emitreg(src, f);
			fprintf(f, ", %d(r%d)\n", off, slotbase(i->to));
			return;
		}
		if (rtype(src) != RCon)
			die("IE64 spill copy source is invalid in '%s'", fn->name);
		c = &fn->con[src.val];
		if (c->type == CAddr) {
			fputc('\t', f);
			emitaddr(TMP(R30), str(c->sym.id), c->bits.i, f);
		} else if (c->type == CBits) {
			fputc('\t', f);
			emitintload(TMP(R30), i->cls, c->bits.i, f);
		} else
			die("IE64 spill constant is invalid in '%s'", fn->name);
		s = i->cls == Kw || i->cls == Ks ? 'l' : 'q';
		fprintf(f, "\tstore.%c r30, %d(r%d)\n", s, off, slotbase(i->to));
		return;
	}
	if (rtype(i->arg[0]) == RSlot) {
		if (!isreg(i->to))
			die("IE64 spill copy destination is invalid in '%s'", fn->name);
		if (i->to.val >= F0)
			fprintf(f, "\t%sload ", i->cls == Kd ? "d" : "f");
		else
			fprintf(f, "\tload.%c ", suffix(i->cls));
		emitreg(i->to, f);
		fprintf(f, ", %d(r%d)\n", slotoffset(i->arg[0]),
			slotbase(i->arg[0]));
		return;
	}

	if (!isreg(i->to))
		die("IE64 copy destination is not a register");
	if (i->cls == Kl && isreg(i->arg[0]) && req(i->to, i->arg[0]))
		return;
	fputc('\t', f);
	if (isreg(i->arg[0])) {
		if (i->cls == Kd)
			fputs("dmov ", f);
		else if (KBASE(i->cls) == 1)
			fputs("fmov ", f);
		else
			fprintf(f, "move.%c ", suffix(i->cls));
		emitreg(i->to, f);
		fputs(", ", f);
		emitreg(i->arg[0], f);
		fputc('\n', f);
		return;
	}
	if (rtype(i->arg[0]) != RCon)
		die("IE64 copy source is invalid");
	c = &fn->con[i->arg[0].val];
	if ((i->cls == Ks || i->cls == Kd) && c->type == CBits) {
		Con zero;

		if (!c->flt) {
			/* C union/memcpy lowering uses integer constants as raw IEEE
			 * bits.  Accept normal, infinity, and NaN encodings while
			 * retaining the diagnostic for accidental subnormal values such
			 * as `copy 1`. */
			if (!req(i->arg[0], UNDEF) && c->bits.i != 0
			&& !rawfloatbits(i->cls, c->bits.i))
				die("IE64 floating copy has an untyped constant");
			zero = *c;
			if (req(i->arg[0], UNDEF))
				zero.bits.i = 0;
			zero.flt = i->cls == Ks ? 1 : 2;
			c = &zero;
		}
		if ((i->cls == Ks && c->flt != 1)
		|| (i->cls == Kd && c->flt != 2))
			die("IE64 floating copy has the wrong constant width");
		emitaddr(TMP(R30), stashfloat(c), 0, f);
		fputs(i->cls == Kd ? "\tdload " : "\tfload ", f);
		emitreg(i->to, f);
		fputs(", (r30)\n", f);
		return;
	}
	if (KBASE(i->cls) == 0 && c->type == CAddr) {
		/* A `w` copy narrows a symbol address to its low 32 bits.  The full
		 * address is materialised and the surplus high bits are discarded by
		 * the word-sized uses, exactly as for any other `w` value. */
		emitaddr(i->to, str(c->sym.id), c->bits.i, f);
		return;
	}
	if (c->type != CBits)
		die("IE64 address materialisation is not selected");
	emitintload(i->to, i->cls, c->bits.i, f);
}

static Ref
materializeaddr(Ref r, Fn *fn, FILE *f)
{
	Con *c;

	if (isreg(r))
		return r;
	if (rtype(r) == RSlot) {
		int s = rsval(r);
		fprintf(f, "\tlea r30, %d(r%d)\n",
			s < 0 ? emitframe - 4 * s : 4 * s,
			emitdynalloc ? 25 : 31);
		return TMP(R30);
	}
	if (rtype(r) != RCon)
		die("IE64 memory address is invalid");
	c = &fn->con[r.val];
	if (c->type == CBits) {
		fputc('\t', f);
		emitintload(TMP(R30), Kl, c->bits.i, f);
		return TMP(R30);
	}
	if (c->type != CAddr)
		die("IE64 memory address is not an address constant");
	fputc('\t', f);
	emitaddr(TMP(R30), str(c->sym.id), c->bits.i, f);
	return TMP(R30);
}

static int
slotoffset(Ref r)
{
	int s = rsval(r);
	return s < 0 ? emitframe - 4 * s : 4 * s;
}

static int
slotbase(Ref r)
{
	(void)r;
	return emitdynalloc ? 25 : 31;
}

static void
emitins(Ins *i, Fn *fn, FILE *f)
{
	static char *bin[] = {
		[Oadd] = "add", [Osub] = "sub", [Omul] = "mulu",
		[Oand] = "and", [Oor] = "or", [Oxor] = "eor",
		[Oshl] = "lsl", [Oshr] = "lsr", [Osar] = "asr",
		[Oudiv] = "divu", [Ourem] = "mod",
		[Odiv] = "divs", [Orem] = "mods",
	};

	if (i->op == Onop)
		return;
	if (isie64op(i->op)) {
		static char *simple[] = {
			[Oie64nop] = "nop", [Oie64halt] = "halt",
			[Oie64sei] = "sei", [Oie64cli] = "cli",
			[Oie64tlbflush] = "tlbflush", [Oie64suaen] = "suaen",
			[Oie64suadis] = "suadis", [Oie64eret] = "eret",
			[Oie64rti] = "rti",
		};
		static char *funary[] = {
			[Oie64fabs] = "fabs", [Oie64dabs] = "dabs",
			[Oie64fint] = "fint", [Oie64dint] = "dint",
			[Oie64fcvtsd] = "fcvtsd", [Oie64fcvtds] = "fcvtds",
			[Oie64fsin] = "fsin", [Oie64fcos] = "fcos",
			[Oie64ftan] = "ftan", [Oie64fatan] = "fatan",
			[Oie64flog] = "flog", [Oie64fexp] = "fexp",
			[Oie64fsqrt] = "fsqrt", [Oie64dsin] = "dsin",
			[Oie64dcos] = "dcos", [Oie64dtan] = "dtan",
			[Oie64datan] = "datan", [Oie64dlog] = "dlog",
			[Oie64dexp] = "dexp", [Oie64dsqrt] = "dsqrt",
		};
		static char *atomic[] = {
			[Oie64xchg] = "xchg", [Oie64faa] = "faa",
			[Oie64fand] = "fand", [Oie64for] = "for",
			[Oie64fxor] = "fxor",
		};
		if (i->op <= Oie64rti) {
			fprintf(f, "\t%s\n", simple[i->op]);
			return;
		}
		if (i->op == Oie64smode || i->op == Oie64fmovsr
		|| i->op == Oie64fmovcr) {
			fprintf(f, "\t%s ", i->op == Oie64smode ? "smode"
				: i->op == Oie64fmovsr ? "fmovsr" : "fmovcr");
			emitreg(i->to, f);
			fputc('\n', f);
			return;
		}
		if (i->op == Oie64tlbinval) {
			fputs("\ttlbinval ", f); emitreg(i->arg[0], f); fputc('\n', f);
			return;
		}
		if (i->op == Oie64wait) {
			fputs("\twait ", f); emitvalue(i->arg[0], fn, f); fputc('\n', f);
			return;
		}
		if (i->op == Oie64syscall) {
			fputs("\tsyscall ", f); emitvalue(i->arg[0], fn, f); fputc('\n', f);
			if (!req(i->to, R) && !req(i->to, TMP(R1))) {
				fputs("\tmove.q ", f); emitreg(i->to, f); fputs(", r1\n", f);
			}
			return;
		}
		if (i->op == Oie64mfcr) {
			fputs("\tmfcr ", f); emitreg(i->to, f); fputs(", cr", f);
			emitint(fn->con[i->arg[0].val].bits.i, f); fputc('\n', f);
			return;
		}
		if (i->op == Oie64fmovecr || i->op == Oie64dmovecr) {
			fputs("\tfmovecr ", f); emitreg(i->to, f); fputs(", ", f);
			emitvalue(i->arg[0], fn, f); fputc('\n', f);
			if (i->op == Oie64dmovecr) {
				fputs("\tfcvtsd ", f); emitreg(i->to, f); fputs(", ", f);
				emitreg(i->to, f); fputc('\n', f);
			}
			return;
		}
		if (i->op == Oie64fmovsc || i->op == Oie64fmovcc) {
			fprintf(f, "\t%s ", i->op == Oie64fmovsc ? "fmovsc" : "fmovcc");
			emitreg(i->arg[0], f); fputc('\n', f);
			return;
		}
		if (i->op == Oie64mtcr) {
			fputs("\tmtcr cr", f); emitint(fn->con[i->arg[0].val].bits.i, f);
			fputs(", ", f); emitreg(i->arg[1], f); fputc('\n', f);
			return;
		}
		if (i->op == Oie64fmod || i->op == Oie64dmod) {
			fprintf(f, "\t%s ", i->op == Oie64fmod ? "fmod" : "dmod");
			emitreg(i->to, f); fputs(", ", f); emitreg(i->arg[0], f);
			fputs(", ", f); emitreg(i->arg[1], f); fputc('\n', f);
			return;
		}
		if (i->op == Oie64fpow || i->op == Oie64dpow) {
			fprintf(f, "\t%s ", i->op == Oie64fpow ? "fpow" : "dpow");
			emitreg(i->to, f); fputs(", ", f); emitreg(i->arg[0], f);
			fputs(", ", f); emitreg(i->arg[1], f); fputc('\n', f);
			return;
		}
		if (INRANGE(i->op, Oie64fabs, Oie64dsqrt) && funary[i->op]) {
			fprintf(f, "\t%s ", funary[i->op]); emitreg(i->to, f);
			fputs(", ", f); emitreg(i->arg[0], f); fputc('\n', f);
			return;
		}
		if (INRANGE(i->op, Oie64xchg, Oie64fxor) && atomic[i->op]) {
			fprintf(f, "\t%s ", atomic[i->op]); emitreg(i->to, f);
			fputs(", (", f); emitreg(i->arg[0], f); fputs("), ", f);
			emitreg(i->arg[1], f); fputc('\n', f);
			return;
		}
		if (i->op == Oie64casd) {
			fputs("\tmove.q r29, ", f); emitreg(i->arg[0], f); fputc('\n', f);
			return;
		}
		if (i->op == Oie64cas) {
			fputs("\tmove.q r30, ", f); emitreg(i->arg[0], f);
			fputs("\n\tmove.q ", f); emitreg(i->to, f); fputs(", ", f);
			emitreg(i->arg[1], f); fputs("\n\tcas ", f); emitreg(i->to, f);
			fputs(", (r30), r29\n", f);
			return;
		}
		die("unimplemented private IE64 operation");
	}
	if (i->op == Ocopy) {
		emitcopy(i, fn, f);
		return;
	}
	if (i->op == Oswap) {
		if (KBASE(i->cls) == 1) {
			fputs("\tsub.q r31, r31, #16\n", f);
			fprintf(f, "\t%sstore ", i->cls == Kd ? "d" : "f");
			emitreg(i->arg[0], f);
			fputs(", (r31)\n", f);
			fprintf(f, "\t%smov ", i->cls == Kd ? "d" : "f");
			emitreg(i->arg[0], f);
			fputs(", ", f);
			emitreg(i->arg[1], f);
			fprintf(f, "\n\t%sload ", i->cls == Kd ? "d" : "f");
			emitreg(i->arg[1], f);
			fputs(", (r31)\n\tadd.q r31, r31, #16\n", f);
			return;
		}
		fputs("\tmove.", f);
		fputc(suffix(i->cls), f);
		fputs(" r30, ", f);
		emitreg(i->arg[0], f);
		fputs("\n\tmove.", f);
		fputc(suffix(i->cls), f);
		fputc(' ', f);
		emitreg(i->arg[0], f);
		fputs(", ", f);
		emitreg(i->arg[1], f);
		fputs("\n\tmove.", f);
		fputc(suffix(i->cls), f);
		fputc(' ', f);
		emitreg(i->arg[1], f);
		fputs(", r30\n", f);
		return;
	}
	if ((i->cls == Ks || i->cls == Kd) && (i->op == Oadd || i->op == Osub
	|| i->op == Omul || i->op == Odiv)) {
		static char *fbin[] = {
			[Oadd] = "fadd", [Osub] = "fsub",
			[Omul] = "fmul", [Odiv] = "fdiv",
		};
		if (i->cls == Kd)
			fprintf(f, "\td%s ", fbin[i->op] + 1);
		else
			fprintf(f, "\t%s ", fbin[i->op]);
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputs(", ", f);
		emitvalue(i->arg[1], fn, f);
		fputc('\n', f);
		return;
	}
	if (i->op == Ostosi && i->cls == Kw) {
		fputs("\tfcvtfi ", f);
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputc('\n', f);
		return;
	}
	if (i->op == Odtosi) {
		fputs("\tdcvtfi ", f);
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputc('\n', f);
		return;
	}
	if (i->op == Oswtof && i->cls == Ks) {
		fputs("\tfcvtif ", f);
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputc('\n', f);
		return;
	}
	if ((i->op == Oswtof || i->op == Osltof) && i->cls == Kd) {
		fputs("\tdcvtif ", f);
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputc('\n', f);
		return;
	}
	if (i->op == Ocast && i->cls == Ks) {
		fputs("\tfmovi ", f);
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputc('\n', f);
		return;
	}
	if (i->op == Ocast && i->cls == Kw) {
		fputs("\tfmovo ", f);
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputc('\n', f);
		return;
	}
	if (i->op == Ocast && i->cls == Kd) {
		fputs("\tsub.q r31, r31, #16\n\tstore.q ", f);
		emitvalue(i->arg[0], fn, f);
		fputs(", (r31)\n\tdload ", f);
		emitreg(i->to, f);
		fputs(", (r31)\n\tadd.q r31, r31, #16\n", f);
		return;
	}
	if (i->op == Ocast && i->cls == Kl) {
		fputs("\tsub.q r31, r31, #16\n\tdstore ", f);
		emitvalue(i->arg[0], fn, f);
		fputs(", (r31)\n\tload.q ", f);
		emitreg(i->to, f);
		fputs(", (r31)\n\tadd.q r31, r31, #16\n", f);
		return;
	}
	if (i->op == Osalloc) {
		Con *c;
		if (rtype(i->arg[0]) == RCon
		&& (c = &fn->con[i->arg[0].val])->type == CBits) {
			if (c->bits.i < 0)
				fprintf(f, "\tadd.q r31, r31, #%"PRId64"\n", -c->bits.i);
			else
				fprintf(f, "\tsub.q r31, r31, #%"PRId64"\n", c->bits.i);
		} else if (isreg(i->arg[0])) {
			fputs("\tsub.q r31, r31, ", f);
			emitreg(i->arg[0], f);
			fputc('\n', f);
		} else
			die("IE64 dynamic stack allocation is not selected");
		if (!req(i->to, R)) {
			fputs("\tmove.q ", f);
			emitreg(i->to, f);
			fputs(", r31\n", f);
		}
		return;
	}
	if (i->op == Oaddr) {
		int s;
		if (!isreg(i->to) || rtype(i->arg[0]) != RSlot)
			die("IE64 local address is not selected");
		s = rsval(i->arg[0]);
		fprintf(f, "\tlea ");
		emitreg(i->to, f);
		fprintf(f, ", %d(r%d)\n", s < 0 ? emitframe - 4 * s : 4 * s,
			emitdynalloc ? 25 : 31);
		return;
	}
	if (isload(i->op)) {
		Con *c;
		char s;
		if (i->cls == Ks || i->cls == Kd) {
			if (rtype(i->arg[0]) == RSlot) {
				fprintf(f, "\t%sload ", i->cls == Kd ? "d" : "f");
				emitreg(i->to, f);
				fprintf(f, ", %d(r%d)\n", slotoffset(i->arg[0]),
					slotbase(i->arg[0]));
				return;
			}
			if (rtype(i->arg[0]) == RCon
			&& (c = &fn->con[i->arg[0].val])->type == CBits
			&& isreg(i->arg[1])) {
				fprintf(f, "\t%sload ", i->cls == Kd ? "d" : "f");
				emitreg(i->to, f);
				fprintf(f, ", %"PRId64"(", c->bits.i);
				emitreg(i->arg[1], f);
				fputs(")\n", f);
				return;
			}
			i->arg[0] = materializeaddr(i->arg[0], fn, f);
			fputs(i->cls == Kd ? "\tdload " : "\tfload ", f);
			emitreg(i->to, f);
			fputs(", ", f);
			emitmem(i->arg[0], f);
			fputc('\n', f);
			return;
		}
		s = suffix(i->cls);
		if (i->op == Oloadsb || i->op == Oloadub)
			s = 'b';
		else if (i->op == Oloadsh || i->op == Oloaduh)
			s = 'w';
		else if (i->op == Oloadsw || i->op == Oloaduw)
			s = 'l';
		if (rtype(i->arg[0]) == RSlot) {
			fprintf(f, "\tload.%c ", s);
			emitreg(i->to, f);
			fprintf(f, ", %d(r%d)\n", slotoffset(i->arg[0]),
				slotbase(i->arg[0]));
			if (i->op == Oloadsb || i->op == Oloadsh
			|| (i->op == Oloadsw && i->cls == Kl)) {
				fprintf(f, "\tsext.%c ", s);
				emitreg(i->to, f);
				fprintf(f, ", r%u\n", i->to.val);
			}
			return;
		}
		if (rtype(i->arg[0]) == RCon
		&& (c = &fn->con[i->arg[0].val])->type == CBits
		&& isreg(i->arg[1])) {
			fprintf(f, "\tload.%c ", s);
			emitreg(i->to, f);
			fprintf(f, ", %"PRId64"(", c->bits.i);
			emitreg(i->arg[1], f);
			fputs(")\n", f);
			return;
		}
		i->arg[0] = materializeaddr(i->arg[0], fn, f);
		fputs("\t", f);
		fprintf(f, "load.%c ", s);
		emitreg(i->to, f);
		fputs(", ", f);
		emitmem(i->arg[0], f);
		fputc('\n', f);
		if (i->op == Oloadsb || i->op == Oloadsh
			|| (i->op == Oloadsw && i->cls == Kl)) {
			fprintf(f, "\tsext.%c ", s);
			emitreg(i->to, f);
			fprintf(f, ", r%u\n", i->to.val);
		}
		return;
	}
	if (isstore(i->op)) {
		char s;
		int stackoff = -1, stackreg = 31;
		if (rtype(i->arg[1]) == RSlot) {
			stackoff = slotoffset(i->arg[1]);
			stackreg = slotbase(i->arg[1]);
		} else
			i->arg[1] = materializeaddr(i->arg[1], fn, f);
		if (isreg(i->arg[0]) && i->arg[0].val >= F0) {
			fputs(i->op == Ostored || i->cls == Kd ? "\tdstore " : "\tfstore ", f);
			emitvalue(i->arg[0], fn, f);
			if (stackoff >= 0)
				fprintf(f, ", %d(r%d)\n", stackoff, stackreg);
			else {
				fputs(", ", f);
				emitmem(i->arg[1], f);
				fputc('\n', f);
			}
			return;
		}
		s = suffix(i->cls);
		if (stackoff >= 0 && !isreg(i->arg[0])) {
			Con *sc;

			if (rtype(i->arg[0]) != RCon
			|| (sc = &fn->con[i->arg[0].val])->type != CBits)
				die("IE64 store value is not selected in '%s'", fn->name);
			fputc('\t', f);
			emitintload(TMP(R30), i->op == Ostorel ? Kl : Kw,
				sc->bits.i, f);
			i->arg[0] = TMP(R30);
		}
		if (i->op == Ostoreb)
			s = 'b';
		else if (i->op == Ostoreh)
			s = 'w';
		else if (i->op == Ostorew)
			s = 'l';
		else if (i->op == Ostorel)
			s = 'q';
		fprintf(f, "\tstore.%c ", s);
		emitvalue(i->arg[0], fn, f);
		if (stackoff >= 0)
			fprintf(f, ", %d(r%d)\n", stackoff, stackreg);
		else {
			fputs(", ", f);
			emitmem(i->arg[1], f);
			fputc('\n', f);
		}
		return;
	}
	if (i->op == Ocall) {
		Con *c;
		if (isreg(i->arg[0])) {
			fputs("\tjsr (", f);
			emitreg(i->arg[0], f);
			fputs(")\n", f);
		} else {
			if (rtype(i->arg[0]) != RCon)
				die("IE64 call target is invalid");
			c = &fn->con[i->arg[0].val];
			if (c->type != CAddr || c->bits.i)
				die("IE64 call target is invalid");
			fprintf(f, "\tjsr %s\n", str(c->sym.id));
		}
		if (!req(i->to, R)) {
			if (i->cls == Ks)
				fputs("\tfmov ", f);
			else if (i->cls == Kd)
				fputs("\tdmov ", f);
			else {
				fputs("\tmove.", f);
				fputc(suffix(i->cls), f);
				fputs(" ", f);
			}
			emitreg(i->to, f);
			fputs(i->cls == Ks || i->cls == Kd ? ", f0\n" : ", r1\n", f);
		}
		return;
	}
	if (isext(i->op)) {
		char s;
		uint64_t mask;

		switch (i->op) {
		case Oextsb: s = 'b'; goto Sign;
		case Oextsh: s = 'w'; goto Sign;
		case Oextsw: s = 'l'; goto Sign;
		case Oextub: mask = 0xff; break;
		case Oextuh: mask = 0xffff; break;
		case Oextuw:
			fputs("\tmove.l ", f);
			emitreg(i->to, f);
			fputs(", ", f);
			emitvalue(i->arg[0], fn, f);
			fputc('\n', f);
			return;
		default: die("invalid IE64 extension");
		}
		fprintf(f, "\tand.q ");
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fprintf(f, ", #0x%"PRIx64"\n", mask);
		return;
	Sign:
		fprintf(f, "\tsext.%c ", s);
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputc('\n', f);
		return;
	}
	if (i->op == Oneg) {
		if (i->cls == Ks || i->cls == Kd) {
			fprintf(f, "\t%s ", i->cls == Kd ? "dneg" : "fneg");
			emitreg(i->to, f);
			fputs(", ", f);
			emitvalue(i->arg[0], fn, f);
			fputc('\n', f);
			return;
		}
		fprintf(f, "\tneg.%c ", suffix(i->cls));
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputc('\n', f);
		return;
	}
	{
		int k, cc;
		static unsigned cmpid;
		char *br;
		unsigned id;

		if (iscmp(i->op, &k, &cc) && KBASE(k) == 0) {
			if ((!isreg(i->arg[0]) && !iszeroref(i->arg[0], fn))
			|| (!isreg(i->arg[1]) && !iszeroref(i->arg[1], fn)))
				die("IE64 comparison operands are not selected");
			switch (cc) {
			case Cieq: br = "bne"; break;
			case Cine: br = "beq"; break;
			case Cisge: br = "blt"; break;
			case Cisgt: br = "ble"; break;
			case Cisle: br = "bgt"; break;
			case Cislt: br = "bge"; break;
			case Ciuge: br = "bhi"; break;
			case Ciugt: br = "bls"; break;
			case Ciule: br = "bhi"; break;
			case Ciult: br = "bls"; break;
			default: die("IE64 comparison value is not implemented");
			}
			id = cmpid++;
			/* The branch reads the operands before the result is written,
			 * so the result may share a register with an operand. */
			if (req(i->to, i->arg[0]) || req(i->to, i->arg[1])) {
				fputc('\t', f);
				fputs(br, f);
				fputc(' ', f);
				if (cc == Ciuge || cc == Ciult)
					emitcmpreg(i->arg[1], fn, f), fputs(", ", f), emitcmpreg(i->arg[0], fn, f);
				else
					emitcmpreg(i->arg[0], fn, f), fputs(", ", f), emitcmpreg(i->arg[1], fn, f);
				fprintf(f, ", .Lcmp%u\n\tmoveq ", id);
				emitreg(i->to, f);
				fprintf(f, ", #1\n\tbra .Lcmpe%u\n.Lcmp%u:\n\tmoveq ", id, id);
				emitreg(i->to, f);
				fprintf(f, ", #0\n.Lcmpe%u:\n", id);
				return;
			}
			fputs("\tmoveq ", f);
			emitreg(i->to, f);
			fputs(", #0\n\t", f);
			fputs(br, f);
			fputc(' ', f);
			if (cc == Ciuge || cc == Ciult)
				emitcmpreg(i->arg[1], fn, f), fputs(", ", f), emitcmpreg(i->arg[0], fn, f);
			else
				emitcmpreg(i->arg[0], fn, f), fputs(", ", f), emitcmpreg(i->arg[1], fn, f);
			fprintf(f, ", .Lcmp%u\n\tmoveq ", id);
			emitreg(i->to, f);
			fprintf(f, ", #1\n.Lcmp%u:\n", id);
			return;
		}
		if (iscmp(i->op, &k, &cc) && (k == Ks || k == Kd)) {
			char *yes;

			if (!isreg(i->arg[0]) || !isreg(i->arg[1]))
				die("IE64 floating comparison operands are not selected");
			id = cmpid++;
			fprintf(f, "\t%s ", k == Kd ? "dcmp" : "fcmp");
			fputs("r29", f);
			fputs(", ", f);
			emitreg(i->arg[0], f);
			fputs(", ", f);
			emitreg(i->arg[1], f);
			fputs("\n\tfmovsr r30\n\tand.q r30, r30, #0x1000000\n", f);
			fputs("\tmoveq ", f);
			emitreg(i->to, f);
			fputs(", #0\n", f);
			yes = 0;
			switch (cc) {
			case NCmpI + Cfeq:
				fprintf(f, "\tbne r30, r0, .Lcmp%s%u\n", "", id);
				yes = "beq";
				break;
			case NCmpI + Cfge: yes = "bge"; break;
			case NCmpI + Cfgt: yes = "bgt"; break;
			case NCmpI + Cfle: yes = "ble"; break;
			case NCmpI + Cflt: yes = "blt"; break;
			case NCmpI + Cfne:
				fprintf(f, "\tbne r30, r0, .Lcmptrue%s%u\n", "", id);
				yes = "bne";
				break;
			case NCmpI + Cfo:
				fprintf(f, "\tbeq r30, r0, .Lcmptrue%s%u\n", "", id);
				break;
			case NCmpI + Cfuo:
				fprintf(f, "\tbne r30, r0, .Lcmptrue%s%u\n", "", id);
				break;
			default: die("invalid IE64 floating comparison");
			}
			if (yes) {
				if (cc != NCmpI + Cfeq && cc != NCmpI + Cfne)
					fprintf(f, "\tbne r30, r0, .Lcmp%s%u\n", "", id);
				fprintf(f, "\t%s ", yes);
				fputs("r29", f);
				fprintf(f, ", r0, .Lcmptrue%s%u\n", "", id);
			}
			fprintf(f, "\tbra .Lcmp%s%u\n.Lcmptrue%s%u:\n\tmoveq ",
				"", id,
				"", id);
			emitreg(i->to, f);
			fprintf(f, ", #1\n.Lcmp%s%u:\n",
				"", id);
			return;
		}
	}
	if (bin[i->op]) {
		fprintf(f, "\t%s.%c ", bin[i->op], suffix(i->cls));
		emitreg(i->to, f);
		fputs(", ", f);
		emitvalue(i->arg[0], fn, f);
		fputs(", ", f);
		emitvalue(i->arg[1], fn, f);
		fputc('\n', f);
		return;
	}
	die("IE64 instruction selection missing %s in '%s'", optab[i->op].name,
		fn->name);
}

static void
emitbranch(int cmp, int invert, Ref a, Ref b, Blk *to, int id0, Fn *fn, FILE *f)
{
	static char *name[] = {
		[Cieq] = "beq", [Cine] = "bne",
		[Cisge] = "bge", [Cisgt] = "bgt", [Cisle] = "ble", [Cislt] = "blt",
		[Ciuge] = "bls", [Ciugt] = "bhi", [Ciule] = "bls", [Ciult] = "bhi",
	};
	char *n;
	Ref t;

	if (cmp == Cieq || cmp == Cine) {
		if (invert)
			cmp = cmp == Cieq ? Cine : Cieq;
	} else if (cmp >= Cisge && cmp <= Cislt) {
		static int inv[] = {Cislt, Cisle, Cisgt, Cisge};
		if (invert)
			cmp = inv[cmp-Cisge];
	} else if (cmp >= Ciuge && cmp <= Ciult) {
		static int inv[] = {Ciult, Ciule, Ciugt, Ciuge};
		if (invert)
			cmp = inv[cmp-Ciuge];
	}
	if (cmp == Ciuge || cmp == Ciult) {
		t = a;
		a = b;
		b = t;
	}
	/* HOOK(branch-immediate): a constant operand reaches here only if
	 * isel kept one.  Zero names r0.  Any other constant is loaded into
	 * r30 today; the immediate form of the branch, once it exists,
	 * replaces these two loads. */
	if (!isreg(a) && !iszeroref(a, fn) && rtype(a) == RCon) {
		fputc('\t', f);
		emitintload(TMP(R30), Kl, fn->con[a.val].bits.i, f);
		a = TMP(R30);
	}
	if (!isreg(b) && !iszeroref(b, fn) && rtype(b) == RCon) {
		fputc('\t', f);
		emitintload(TMP(R30), Kl, fn->con[b.val].bits.i, f);
		b = TMP(R30);
	}
	if ((!isreg(a) && !iszeroref(a, fn)) || (!isreg(b) && !iszeroref(b, fn)))
		die("IE64 branch comparison is not selected");
	n = name[cmp];
	fprintf(f, "\t%s ", n);
	emitcmpreg(a, fn, f);
	fputs(", ", f);
	emitcmpreg(b, fn, f);
	fputs(", ", f);
	emitlbl(id0+to->id, f);
	fputc('\n', f);
}

static int
emitsaves(Fn *fn, int off, int restore, FILE *f)
{
	int r, n = 0;

	for (r=R18; r<=R25; r++)
		if (fn->reg & BIT(r) && (r != R25 || fn->dynalloc)) {
			if (f && restore)
				fprintf(f, "\tload.q r%d, %d(r31)\n", r, off + 8*n);
			else if (f)
				fprintf(f, "\tstore.q r%d, %d(r31)\n", r, off + 8*n);
			n++;
		}
	for (r=F8; r<=F15; r++)
		if (fn->reg & BIT(r)) {
			if (f && restore)
				fprintf(f, "\tfload f%d, %d(r31)\n", r-F0, off + 8*n);
			else if (f)
				fprintf(f, "\tfstore f%d, %d(r31)\n", r-F0, off + 8*n);
			n++;
		}
	return n;
}

/* Registers an allocated instruction reads, with calls modelled as in
 * filllive(): they read their argument registers. */
static bits
insuse(Ins *i)
{
	bits u;
	int n;

	u = 0;
	if (iscallop(i->op) && rtype(i->arg[1]) == RCall)
		u |= T.argregs(i->arg[1], 0);
	for (n=0; n<2; n++)
		if (isreg(i->arg[n]))
			u |= BIT(i->arg[n].val);
	return u;
}

/* Registers live on entry to each block of the final, allocated CFG.  Folding
 * a comparison into its branch is only sound while nothing reads the
 * comparison's result register afterwards, and the temporary-level liveness
 * filled before allocation no longer describes registers. */
static bits *
fillreglive(Fn *fn)
{
	Blk *b;
	Ins *i;
	bits *in, live;
	int chg;

	in = alloc(fn->nblk * sizeof in[0]);
	do {
		chg = 0;
		for (b=fn->start; b; b=b->link) {
			live = 0;
			if (b->s1)
				live |= in[b->s1->id];
			if (b->s2)
				live |= in[b->s2->id];
			if (rtype(b->jmp.arg) == RCall)
				live |= T.retregs(b->jmp.arg, 0);
			else if (isreg(b->jmp.arg))
				live |= BIT(b->jmp.arg.val);
			for (i=&b->ins[b->nins]; i!=b->ins;) {
				--i;
				if (iscallop(i->op) && rtype(i->arg[1]) == RCall)
					live &= ~T.retregs(i->arg[1], 0);
				if (isreg(i->to))
					live &= ~BIT(i->to.val);
				live |= insuse(i);
			}
			if (live != in[b->id]) {
				in[b->id] = live;
				chg = 1;
			}
		}
	} while (chg);
	return in;
}

/* Is the instruction a definition `t = base + k` that a memory operand can
 * absorb?  That is an add or sub of a small constant, or the address of a
 * stack slot. */
static int
addrdef(Ins *i, Fn *fn, Ref *base, int64_t *k)
{
	Con *c;
	int64_t v;
	int s;

	if (!isreg(i->to) || i->to.val >= F0 || req(i->to, R))
		return 0;
	if (i->op == Oaddr) {
		if (rtype(i->arg[0]) != RSlot)
			return 0;
		s = rsval(i->arg[0]);
		*k = s < 0 ? emitframe - 4 * s : 4 * s;
		*base = TMP(emitdynalloc ? R25 : R31);
		return 1;
	}
	if ((i->op != Oadd && i->op != Osub) || i->cls != Kl
	|| !isreg(i->arg[0]) || i->arg[0].val >= F0 || req(i->arg[0], R)
	|| rtype(i->arg[1]) != RCon)
		return 0;
	c = &fn->con[i->arg[1].val];
	if (c->type != CBits || c->flt)
		return 0;
	v = c->bits.i;
	if (i->op == Osub)
		v = -v;
	if (c->bits.i < INT32_MIN || c->bits.i > INT32_MAX
	|| v < INT32_MIN || v > INT32_MAX)
		return 0;
	*k = v;
	*base = i->arg[0];
	return 1;
}

static int
callersave(Ref r)
{
	int *p;

	for (p=ie64_rsave; *p>=0; p++)
		if (*p == (int)r.val)
			return 1;
	return 0;
}

/* Fold `t = base + k` into the loads and stores that use t as their address,
 * when t is dead afterwards.  Runs on allocated code.  off[] receives the
 * displacement for each folded memory instruction; the definition becomes a
 * nop.  The scan is deliberately conservative: it gives up at calls and at
 * instructions with implicit register effects, at any other read of t, and at
 * a use of t after the base register was overwritten. */
static void
foldaddrs(Fn *fn, Blk *b, bits *livein, int64_t *off)
{
	enum { MaxUses = 16 };
	Ins *d, *j;
	Ref t, base;
	int64_t k;
	uint use[MaxUses], p, q;
	int n, dead, baseset;
	bits out;

	/* Last definition first: when the base of one folded address is the
	 * result of another definition, the earlier one is folded into the
	 * same access in turn. */
	for (p=b->nins; p-- > 0;) {
		d = &b->ins[p];
		if (!addrdef(d, fn, &base, &k))
			continue;
		t = d->to;
		n = 0;
		dead = 0;
		baseset = 0;
		for (q=p+1; q<b->nins; q++) {
			int addruse = 0;

			j = &b->ins[q];
			if (j->op == Onop)
				continue;
			if (isie64op(j->op) || j->op == Osalloc || j->op == Oswap)
				goto Skip;
			if (iscallop(j->op)) {
				/* A call reads its argument registers and clobbers the
				 * caller-save set.  A register in that set that the call
				 * does not read is dead at the call. */
				if (insuse(j) & BIT(t.val))
					goto Skip;
				if (callersave(t)) {
					dead = 1;
					break;
				}
				if (callersave(base))
					baseset = 1;
			}
			if (isload(j->op) && req(j->arg[0], t) && req(j->arg[1], R))
				addruse = 1;
			else if (isstore(j->op) && req(j->arg[1], t)
			&& !req(j->arg[0], t))
				addruse = 1;
			if ((insuse(j) & BIT(t.val)) && !addruse)
				goto Skip;
			if (addruse) {
				if (baseset || n == MaxUses
				|| off[q] + k < INT32_MIN || off[q] + k > INT32_MAX)
					goto Skip;
				use[n++] = q;
			}
			if (isreg(j->to) && !req(j->to, R)) {
				if (req(j->to, t)) {
					dead = 1;
					break;
				}
				if (req(j->to, base))
					baseset = 1;
			}
		}
		if (!dead) {
			/* Falling off the end of the block: t must not be live out. */
			out = 0;
			if (b->s1)
				out |= livein[b->s1->id];
			if (b->s2)
				out |= livein[b->s2->id];
			if (rtype(b->jmp.arg) == RCall)
				out |= T.retregs(b->jmp.arg, 0);
			else if (isreg(b->jmp.arg))
				out |= BIT(b->jmp.arg.val);
			if (out & BIT(t.val))
				continue;
		}
		if (!n)
			continue;
		for (q=0; (int)q<n; q++) {
			j = &b->ins[use[q]];
			if (isload(j->op))
				j->arg[0] = base;
			else
				j->arg[1] = base;
			off[use[q]] += k;
		}
		*d = (Ins){.op = Onop};
	Skip:;
	}
}

/* Registers live at the end of a block. */
static bits
liveout(Blk *b, bits *livein)
{
	bits out;

	out = 0;
	if (b->s1)
		out |= livein[b->s1->id];
	if (b->s2)
		out |= livein[b->s2->id];
	if (rtype(b->jmp.arg) == RCall)
		out |= T.retregs(b->jmp.arg, 0);
	else if (isreg(b->jmp.arg))
		out |= BIT(b->jmp.arg.val);
	return out;
}

/* Can the destination of this instruction be another register without
 * changing the value it leaves?  Only a plain operation that writes one
 * integer register, with the whole register as its result. */
static int
retargetable(Ins *x, int cpycls)
{
	if (!isreg(x->to) || x->to.val >= F0 || KBASE(x->cls) != 0)
		return 0;
	if (cpycls == Kw) {
		/* move.l clears the upper half: the result must be clean. */
		if (x->cls != Kw)
			return 0;
		switch (x->op) {
		case Oextsb: case Oextsh: case Oextsw:
		case Oloadsb: case Oloadsh: case Oloadsw:
			return 0;
		}
	}
	if (isload(x->op))
		return 1;
	switch (x->op) {
	case Oadd: case Osub: case Omul: case Oand: case Oor: case Oxor:
	case Oshl: case Oshr: case Osar: case Oudiv: case Ourem:
	case Odiv: case Orem: case Oneg: case Ocopy: case Oaddr:
	case Oextsb: case Oextub: case Oextsh: case Oextuh:
	case Oextsw: case Oextuw:
		return 1;
	}
	return 0;
}

static int
plainreg(Ref r)
{
	return isreg(r) && r.val >= R1 && r.val <= R28 && r.val != R25;
}

/* `x = op ...; move d, x` where x is dead after the move: the operation
 * can write d.  Phi copies and parameter shuffles leave many of these. */
static void
copyfwd(Fn *fn, Blk *b, bits *livein)
{
	Ins *c, *j, *x;
	Ref rs, rd;
	bits out;
	int dead, abort_;

	for (c=b->ins; c<&b->ins[b->nins]; c++) {
		if (c->op != Ocopy || !plainreg(c->to) || !plainreg(c->arg[0])
		|| req(c->to, c->arg[0]) || KBASE(c->cls) != 0)
			continue;
		rd = c->to;
		rs = c->arg[0];
		x = 0;
		abort_ = 0;
		for (j=c; j>b->ins;) {
			--j;
			if (j->op == Onop)
				continue;
			if (isie64op(j->op) || iscallop(j->op) || j->op == Oswap
			|| j->op == Osalloc || j->op == Oie64nop)
				break;
			if (req(j->to, rs)) {
				x = j;
				break;
			}
			if ((insuse(j) & (BIT(rs.val) | BIT(rd.val)))
			|| req(j->to, rd))
				break;
		}
		if (!x || !retargetable(x, c->cls))
			continue;
		/* The source must be dead after the copy. */
		dead = 0;
		for (j=c+1; j<&b->ins[b->nins]; j++) {
			if (j->op == Onop)
				continue;
			if (insuse(j) & BIT(rs.val))
				break;
			if (iscallop(j->op) && callersave(rs)) {
				dead = 1;
				break;
			}
			if (isie64op(j->op) || j->op == Oswap || j->op == Osalloc)
				break;
			if (req(j->to, rs)) {
				dead = 1;
				break;
			}
		}
		if (j == &b->ins[b->nins]) {
			out = liveout(b, livein);
			dead = !(out & BIT(rs.val));
		}
		if (!dead)
			continue;
		x->to = rd;
		*c = (Ins){.op = Onop};
	}
	(void)fn;
	(void)abort_;
}

/* Registers an instruction or the end of a block mentions. */
static bits
insmention(Ins *i)
{
	bits m;

	m = insuse(i);
	if (isreg(i->to))
		m |= BIT(i->to.val);
	return m;
}

typedef struct Cand Cand;
struct Cand {
	Blk *b;
	uint at;
	Ref reg;
	int64_t v;
	int nuse;
	uint use[8];
	int usejmp;
	int grp;
};

/* The value a constant load leaves in its register. */
static int
constload(Fn *fn, Ins *i, int64_t *v)
{
	Con *c;

	if (i->op != Ocopy || !plainreg(i->to) || KBASE(i->cls) != 0
	|| rtype(i->arg[0]) != RCon)
		return 0;
	c = &fn->con[i->arg[0].val];
	if (c->type != CBits || c->flt)
		return 0;
	*v = i->cls == Kw ? (int64_t)(uint32_t)c->bits.i : c->bits.i;
	return 1;
}

/* Hoist the constants a loop loads into registers that the loop leaves
 * alone, out of the loop.  A loop without calls leaves most of the
 * caller-save registers free. */
static void
hoistloop(Fn *fn, Blk *h, Blk *l, bits *livein)
{
	enum { MaxCand = 64 };
	Blk *b, *p, *pre, *other, *x;
	Ins *i, *j, *ni;
	Cand *cs;
	int64_t gv[32];
	int gcnt[32], greg[32], ng, nc, k, q, r, n, ok, *rs;
	bits mention, free_, lout;
	uint npre, t;
	int hascall = 0;

	pre = 0;
	npre = 0;
	mention = 0;
	for (b=fn->start; b; b=b->link) {
		if (b->id < h->id || b->id > l->id) {
			if (b->s1 == h || b->s2 == h) {
				pre = b;
				npre++;
			}
			if (b->s1 && b->s1 != h && b->s1->id >= h->id
			&& b->s1->id <= l->id)
				return;
			if (b->s2 && b->s2 != h && b->s2->id >= h->id
			&& b->s2->id <= l->id)
				return;
			continue;
		}
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (i->op == Onop)
				continue;
			if (isie64op(i->op) || i->op == Oswap
			|| i->op == Osalloc || i->op == Ovastart)
				return;
			if (iscallop(i->op))
				hascall = 1;
			mention |= insmention(i);
		}
		if (isreg(b->jmp.arg))
			mention |= BIT(b->jmp.arg.val);
		if (rtype(b->jmp.arg) == RCall)
			return;
	}
	if (npre != 1 || !pre || (pre->jmp.type != Jjmp && pre->jmp.type != Jjnz))
		return;
	/* The registers free for a constant: caller-save, untouched by the
	 * loop and the preheader, and dead on entry to the loop and on the
	 * other way out of the preheader. */
	free_ = 0;
	/* A callee-save register that the function saves anyway costs
	 * nothing; a caller-save one does not survive a call. */
	if (!hascall)
		for (rs=ie64_rsave; *rs>=0; rs++)
			if (*rs < F0)
				free_ |= BIT(*rs);
	for (t=R18; t<=R24; t++)
		if (fn->reg & BIT(t))
			free_ |= BIT(t);
	free_ &= ~mention & ~livein[h->id];
	for (i=pre->ins; i<&pre->ins[pre->nins]; i++)
		free_ &= ~insmention(i);
	if (isreg(pre->jmp.arg))
		free_ &= ~BIT(pre->jmp.arg.val);
	other = pre->s1 == h ? pre->s2 : pre->s1;
	if (pre->jmp.type == Jjnz && other)
		free_ &= ~livein[other->id];
	if (pre->jmp.type == Jjnz && other == h)
		return;
	if (!free_)
		return;
	cs = alloc(MaxCand * sizeof cs[0]);
	nc = 0;
	for (x=h; x && x->id <= l->id; x=x->link) {
		lout = 0;
		if (x->s1)
			lout |= livein[x->s1->id];
		if (x->s2)
			lout |= livein[x->s2->id];
		if (isreg(x->jmp.arg))
			lout |= BIT(x->jmp.arg.val);
		for (q=0; q<(int)x->nins; q++) {
			Cand *c;
			int64_t v;

			i = &x->ins[q];
			if (nc == MaxCand || !constload(fn, i, &v))
				continue;
			c = &cs[nc];
			c->b = x;
			c->at = q;
			c->reg = i->to;
			c->v = v;
			c->nuse = 0;
			c->usejmp = 0;
			ok = 1;
			for (j=i+1;; j++) {
				if (j == &x->ins[x->nins]) {
					if (isreg(x->jmp.arg) && req(x->jmp.arg, c->reg))
						c->usejmp = 1;
					else if (lout & BIT(c->reg.val))
						ok = 0;
					break;
				}
				if (j->op == Onop)
					continue;
				if (insuse(j) & BIT(c->reg.val)) {
					if (c->nuse == 8) {
						ok = 0;
						break;
					}
					c->use[c->nuse++] = j - x->ins;
				}
				if (req(j->to, c->reg))
					break;
			}
			/* A value that flows into a fused comparison's branch is
			 * read through the comparison's operands: they are among
			 * the uses above. */
			if (ok && (c->nuse || c->usejmp))
				nc++;
		}
	}
	if (!nc)
		return;
	/* Group by value and take the most frequent first. */
	ng = 0;
	for (k=0; k<nc; k++) {
		for (n=0; n<ng; n++)
			if (gv[n] == cs[k].v)
				break;
		if (n == ng) {
			if (ng == 32)
				continue;
			gv[ng] = cs[k].v;
			gcnt[ng] = 0;
			greg[ng] = -1;
			ng++;
		}
		cs[k].grp = n;
		gcnt[n]++;
	}
	for (n=0; n<ng; n++) {
		r = -1;
		for (k=0; k<ng; k++)
			if (greg[k] < 0 && (r < 0 || gcnt[k] > gcnt[r]))
				r = k;
		if (r < 0)
			break;
		for (t=R1; t<=R30 && !(free_ & BIT(t)); t++)
			;
		if (t > R30)
			break;
		free_ &= ~BIT(t);
		greg[r] = t;
	}
	nc = nc;
	/* Rewrite the uses, drop the loads, and load once in the preheader. */
	npre = 0;
	for (n=0; n<ng; n++)
		if (greg[n] >= 0)
			npre++;
	if (!npre)
		return;
	ni = vnew(pre->nins + npre, sizeof ni[0], PFn);
	for (t=0; t<pre->nins; t++)
		ni[t] = pre->ins[t];
	for (n=0; n<ng; n++)
		if (greg[n] >= 0)
			ni[t++] = (Ins){.op = Ocopy, .cls = Kl, .to = TMP(greg[n]),
				.arg = {getcon(gv[n], fn), R}};
	pre->ins = ni;
	pre->nins = t;
	for (k=0; k<nc; k++) {
		Cand *c = &cs[k];
		Ref f;

		if (greg[c->grp] < 0)
			continue;
		f = TMP(greg[c->grp]);
		for (n=0; n<c->nuse; n++) {
			j = &c->b->ins[c->use[n]];
			if (req(j->arg[0], c->reg))
				j->arg[0] = f;
			if (req(j->arg[1], c->reg))
				j->arg[1] = f;
		}
		if (c->usejmp)
			c->b->jmp.arg = f;
		c->b->ins[c->at] = (Ins){.op = Onop};
	}
	(void)p;
}

static void
hoistconsts(Fn *fn)
{
	enum { MaxLoops = 32 };
	Blk *hd[MaxLoops], *lt[MaxLoops], *b, *s, *t;
	bits *livein;
	int n, i, j, k;

	n = 0;
	for (b=fn->start; b; b=b->link)
		for (k=0; k<2; k++) {
			s = k ? b->s2 : b->s1;
			if (!s || s->id > b->id)
				continue;
			for (i=0; i<n; i++)
				if (hd[i] == s)
					break;
			if (i == n) {
				if (n == MaxLoops)
					return;
				hd[n] = s;
				lt[n++] = b;
			} else if (lt[i]->id < b->id)
				lt[i] = b;
		}
	/* Inner loops first, so that a constant moves out of several. */
	for (i=0; i<n; i++)
		for (j=i+1; j<n; j++)
			if (lt[j]->id - hd[j]->id < lt[i]->id - hd[i]->id) {
				s = hd[i]; hd[i] = hd[j]; hd[j] = s;
				t = lt[i]; lt[i] = lt[j]; lt[j] = t;
			}
	/* Overlapping loops may take a second round. */
	for (k=0; k<2; k++)
		for (i=0; i<n; i++) {
			livein = fillreglive(fn);
			hoistloop(fn, hd[i], lt[i], livein);
		}
}

void
ie64_emitfn(Fn *fn, FILE *f)
{
	static int id0;
	Blk *b;
	Con *c;
	Ins *i, *cmp;
	bits *livein, out;
	int64_t *foff, **foffs;
	Ins **fused;
	uint nb;
	int first, j, ck, cc, frame, local, saveoff, nsaves;

	/* IE64 JSR pushes an eight-byte return address.  A non-leaf frame therefore
	 * has an eight-byte residue so the body stack pointer remains 16-byte aligned. */
	if (fn->dynalloc)
		fn->reg |= BIT(R25);
	if (fn->lnk.sec)
		fprintf(f, ".section %s,%s\n", fn->lnk.sec,
			fn->lnk.secf ? fn->lnk.secf : "\"ax\"");
	else
		fputs(".section .text,\"ax\"\n", f);
	fprintf(f, "%s %s\n", fn->lnk.weak ? ".weak" : fn->lnk.export ? ".global" : ".local", fn->name);
	if (fn->lnk.hidden)
		fprintf(f, ".hidden %s\n", fn->name);
	fprintf(f, ".type %s,@function\n", fn->name);
	if (fn->lnk.align > 1)
		fprintf(f, ".align %d\n", fn->lnk.align);
	fprintf(f, "%s:\n", fn->name);
	local = 4 * fn->slot;
	saveoff = (local + 7) & -8;
	nsaves = emitsaves(fn, saveoff, 0, 0);
	frame = saveoff + 8*nsaves;
	if (frame || !fn->leaf)
		frame = ((frame + 8 + 15) & -16) - 8;
	emitframe = frame;
	emitdynalloc = fn->dynalloc;
	if (frame)
		fprintf(f, "\tsub.q r31, r31, #%d\n", frame);
	if (nsaves)
		emitsaves(fn, saveoff, 0, f);
	if (fn->dynalloc)
		fputs("\tmove.q r25, r31\n", f);
	ie64_dupjumps(fn);
	ie64_mergeblocks(fn);
	livein = fillreglive(fn);
	nb = fn->nblk;
	foffs = alloc(nb * sizeof foffs[0]);
	fused = alloc(nb * sizeof fused[0]);
	for (b=fn->start; b; b=b->link)
		copyfwd(fn, b, livein);
	hoistconsts(fn);
	livein = fillreglive(fn);
	for (b=fn->start; b; b=b->link) {
		foff = alloc((b->nins + 1) * sizeof foff[0]);
		foffs[b->id] = foff;
		foldaddrs(fn, b, livein, foff);
		cmp = 0;
		if (b->jmp.type == Jjnz)
			for (i=b->ins; i<&b->ins[b->nins]; i++)
				if (req(i->to, b->jmp.arg) && iscmp(i->op, &ck, &cc)
				&& KBASE(ck) == 0)
					cmp = i;
		/* Fusing drops the comparison and re-reads its operand registers at
		 * the branch, which register allocation considers dead from the
		 * comparison onwards.  Anything below that redefines an operand or the
		 * result, or a call clobbering the caller-save set, would be read
		 * through.  Materialise the comparison instead in those cases.  The
		 * same holds when the result itself is read again, below in this
		 * block or in a successor, since nothing ever writes it. */
		if (cmp)
			for (i=cmp+1; i<&b->ins[b->nins]; i++)
				if (iscallop(i->op)
				|| (insuse(i) & BIT(cmp->to.val))
				|| (!req(i->to, R)
				&& (req(i->to, cmp->to) || req(i->to, cmp->arg[0])
				|| req(i->to, cmp->arg[1])))) {
					cmp = 0;
					break;
				}
		if (cmp) {
			out = livein[b->s1->id];
			if (b->s2)
				out |= livein[b->s2->id];
			if (out & BIT(cmp->to.val))
				cmp = 0;
		}
		fused[b->id] = cmp;
	}
	for (first=1, b=fn->start; b; b=b->link) {
		foff = foffs[b->id];
		cmp = fused[b->id];
		if (cmp)
			iscmp(cmp->op, &ck, &cc);
		if (!first || b->npred > 1) {
			emitlbl(id0+b->id, f);
			fputs(":\n", f);
		}
		first = 0;
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (i != cmp) {
				emitoff = foff[i - b->ins];
				emitins(i, fn, f);
				emitoff = 0;
			}
		switch (b->jmp.type) {
		case Jret0:
			if (fn->dynalloc)
				fputs("\tmove.q r31, r25\n", f);
			if (nsaves)
				emitsaves(fn, saveoff, 1, f);
			if (frame)
				fprintf(f, "\tadd.q r31, r31, #%d\n", frame);
			fputs("\trts\n", f);
			break;
		case Jretw:
		case Jretl:
			j = b->jmp.type - Jretw;
			if (isreg(b->jmp.arg)) {
				if (j == 0)
					fputs("\tsext.l r1, ", f), emitreg(b->jmp.arg, f), fputc('\n', f);
				else if (!req(b->jmp.arg, TMP(R1)))
					fputs("\tmove.q r1, ", f), emitreg(b->jmp.arg, f), fputc('\n', f);
			} else if (rtype(b->jmp.arg) == RCon) {
				c = &fn->con[b->jmp.arg.val];
				if (c->type == CAddr) {
					fputc('\t', f);
					emitaddr(TMP(R1), str(c->sym.id),
						c->bits.i, f);
					if (j == 0)
						fputs("\tsext.l r1, r1\n", f);
				} else if (c->type == CBits) {
					/* A w result is returned sign-extended: load the
					 * sign-extended constant directly. */
					fputc('\t', f);
					emitintload(TMP(R1), Kl,
						j == 0 ? (int64_t)(int32_t)c->bits.i : c->bits.i, f);
				} else
					die("IE64 return constant is invalid in '%s'",
						fn->name);
			} else
				die("IE64 return selection missing in '%s'", fn->name);
			if (fn->dynalloc)
				fputs("\tmove.q r31, r25\n", f);
			if (nsaves)
				emitsaves(fn, saveoff, 1, f);
			if (frame)
				fprintf(f, "\tadd.q r31, r31, #%d\n", frame);
			fputs("\trts\n", f);
			break;
		case Jjmp:
			if (b->s1 != b->link)
				fputs("\tbra ", f), emitlbl(id0+b->s1->id, f), fputc('\n', f);
			break;
		case Jjnz:
			if (!cmp) {
				if (!isreg(b->jmp.arg))
					die("IE64 conditional value is not selected in '%s'", fn->name);
				if (b->link == b->s2) {
					fputs("\tbne ", f);
					emitreg(b->jmp.arg, f);
					fputs(", r0, ", f);
					emitlbl(id0+b->s1->id, f);
					fputc('\n', f);
				} else if (b->link == b->s1) {
					fputs("\tbeq ", f);
					emitreg(b->jmp.arg, f);
					fputs(", r0, ", f);
					emitlbl(id0+b->s2->id, f);
					fputc('\n', f);
				} else {
					fputs("\tbne ", f);
					emitreg(b->jmp.arg, f);
					fputs(", r0, ", f);
					emitlbl(id0+b->s1->id, f);
					fputc('\n', f);
					fputs("\tbra ", f);
					emitlbl(id0+b->s2->id, f);
					fputc('\n', f);
				}
			} else if (b->link == b->s2)
				emitbranch(cc, 0, cmp->arg[0], cmp->arg[1], b->s1, id0, fn, f);
			else if (b->link == b->s1)
				emitbranch(cc, 1, cmp->arg[0], cmp->arg[1], b->s2, id0, fn, f);
			else {
				emitbranch(cc, 0, cmp->arg[0], cmp->arg[1], b->s1, id0, fn, f);
				fputs("\tbra ", f), emitlbl(id0+b->s2->id, f), fputc('\n', f);
			}
			break;
		case Jhlt:
			fputs("\thalt\n", f);
			break;
		default:
			die("IE64 jump selection missing in '%s'", fn->name);
		}
	}
	id0 += fn->nblk;
	fprintf(f, ".size %s,.-%s\n", fn->name, fn->name);
}

static uint64_t
stringdatasize(const char *s)
{
	uint64_t n = 0;
	int digits;

	if (*s++ != '"')
		die("malformed data string");
	while (*s && *s != '"') {
		if (*s++ == '\\') {
			if (!*s)
				die("malformed data string escape");
			if (*s >= '0' && *s <= '7') {
				for (digits=0; digits<3 && *s >= '0' && *s <= '7'; digits++)
					s++;
			} else
				s++;
		}
		n++;
	}
	if (*s != '"' || s[1])
		die("malformed data string");
	return n;
}

static uint64_t
datasize(SavedDat *first, SavedDat **end, int *hasinit)
{
	SavedDat *s;
	uint64_t n = 0;

	*hasinit = 0;
	for (s=first->next; s && s->d.type != DEnd; s=s->next) {
		switch (s->d.type) {
		case DZ: n += s->d.u.num; break;
		case DB: n += s->d.isstr ? stringdatasize(s->d.u.str) : 1; *hasinit = 1; break;
		case DH: n += 2; *hasinit = 1; break;
		case DW: n += 4; *hasinit = 1; break;
		case DL: n += 8; *hasinit = 1; break;
		default: break;
		}
	}
	*end = s;
	return n;
}

void
ie64_emitfin(FILE *f)
{
	SavedDat *s, *next, *end, *start;
	uint64_t size;
	int hasinit;

	for (s=datahead; s;) {
		start = s;
		if (start->d.type != DStart)
			die("malformed IE64 data stream");
		size = datasize(start, &end, &hasinit);
		if (!end)
			die("unterminated IE64 data definition");
		if (start->lnk.common) {
			if (hasinit)
				die("invalid common data definition");
			if (start->lnk.weak)
				fprintf(f, ".weak %s\n", start->d.name);
			if (start->lnk.hidden)
				fprintf(f, ".hidden %s\n", start->d.name);
			if (!start->lnk.export && !start->lnk.weak)
				fprintf(f, ".local %s\n", start->d.name);
			fprintf(f, ".comm %s,%"PRIu64",%d\n", start->d.name, size,
				start->lnk.align > 1 ? start->lnk.align : 1);
		} else {
			if (start->lnk.sec)
				fprintf(f, ".section %s,%s\n", start->lnk.sec,
					start->lnk.secf ? start->lnk.secf : "\"aw\"");
			else
				fprintf(f, ".section %s,\"aw\"\n", hasinit ? ".data" : ".bss");
			for (s=start; s != end; s=s->next)
				emitdata(s, f);
			fprintf(f, ".size %s,%"PRIu64"\n", start->d.name, size);
		}
		s = end->next;
	}
	for (s=datahead; s; s=next) {
		next = s->next;
		free(s->d.name);
		free(s->lnk.sec);
		free(s->lnk.secf);
		if (s->d.isstr)
			free(s->d.u.str);
		if (s->d.isref)
			free(s->d.u.ref.name);
		free(s);
	}
	for (FloatCon *fc=floathead, *nextfc; fc; fc=nextfc) {
		nextfc = fc->next;
		fputs(".section .rodata,\"a\"\n", f);
		fprintf(f, ".local %s\n", floatlabel(fc->id));
		if (fc->flt == 1)
			fprintf(f, ".align 4\n%s:\n\tdc.l 0x%08"PRIx64"\n",
				floatlabel(fc->id), fc->bits);
		else
			fprintf(f, ".align 8\n%s:\n\tdc.q 0x%016"PRIx64"\n",
				floatlabel(fc->id), fc->bits);
		free(fc);
	}
	floathead = 0;
	floattail = &floathead;
	datahead = 0;
	datatail = &datahead;
}
