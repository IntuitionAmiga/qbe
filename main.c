#include "all.h"
#include "config.h"
#include <ctype.h>
#include <getopt.h>

Target T;

char debug['Z'+1] = {
	['P'] = 0, /* parsing */
	['M'] = 0, /* memory optimization */
	['N'] = 0, /* ssa construction */
	['C'] = 0, /* copy elimination */
	['G'] = 0, /* gvn/gcm */
	['K'] = 0, /* if-conversion */
	['A'] = 0, /* abi lowering */
	['I'] = 0, /* instruction selection */
	['L'] = 0, /* liveness */
	['S'] = 0, /* spilling */
	['R'] = 0, /* reg. allocation */
};

extern Target T_amd64_sysv;
extern Target T_amd64_apple;
extern Target T_amd64_win;
extern Target T_arm64;
extern Target T_arm64_apple;
extern Target T_rv64;
extern Target T_ie64;

static Target *tlist[] = {
	&T_amd64_sysv,
	&T_amd64_apple,
	&T_amd64_win,
	&T_arm64,
	&T_arm64_apple,
	&T_rv64,
	&T_ie64,
	0
};
static FILE *outf;
static int dbg;
int optlevel = 2;
extern void ie64_bitopt(Fn *);
extern void ie64_narrow(Fn *);
extern void ie64_lower_fpext(Fn *);
static int ie64_fpext, ie64_fma;
extern void ie64_searchloops(Fn *);
extern int ie64_threadstate(Fn *);

static void
lowervolatile(Fn *fn)
{
	Blk *b;
	Ins *i;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (isvstore(i->op))
				i->op = Ostoreb + i->op - Ostorevb;
			else if (isvload(i->op))
				i->op = Oloadsb + i->op - Oloadvsb;
		}
}

static void
checktargetops(Fn *fn)
{
	Blk *b;
	Ins *i;

	for (b=fn->start; b; b=b->link)
		for (i=b->ins; i<&b->ins[b->nins]; i++) {
			if (isie64op(i->op) && strcmp(T.name, "ie64") != 0)
				die("private IE64 operation '%s' used with target '%s'",
					optab[i->op].name, T.name);
			if (isie64fpext(i->op) && (isie64fused(i->op)
			    ? !ie64_fma : !ie64_fpext)) {
				fprintf(stderr, "qbe: operation '%s' needs %s\n", optab[i->op].name,
					isie64fused(i->op) ? "-mfma" : "-mfpext");
				exit(1);
			}
		}
}

static void
data(Dat *d)
{
	if (dbg)
		return;
	if (T.emitdat)
		T.emitdat(d, outf);
	else
		emitdat(d, outf);
	if (d->type == DEnd && strcmp(T.name, "ie64") != 0) {
		fputs("/* end data */\n\n", outf);
		freeall();
	}
}

static void
funcdo(Fn *fn)
{
	uint n;
	int reduced;

	if (dbg)
		fprintf(stderr, "**** Function %s ****", fn->name);
	if (debug['P']) {
		fprintf(stderr, "\n> After parsing:\n");
		printfn(fn, stderr);
	}
	T.abi0(fn);
	fillcfg(fn);
	filluse(fn);
	promote(fn);
	filluse(fn);
	ssa(fn);
	filluse(fn);
	ssacheck(fn);
	fillalias(fn);
	if (optlevel >= 1) {
		loadopt(fn);
		filluse(fn);
		fillalias(fn);
	}
	coalesce(fn);
	filluse(fn);
	filldom(fn);
	ssacheck(fn);
	gvn(fn);
	fillcfg(fn);
	simplcfg(fn);
	filluse(fn);
	filldom(fn);
	if (T.preopt && T.sinkfuse) {
		T.preopt(fn);
		filluse(fn);
	}
	if (optlevel >= 2 && strcmp(T.name, "ie64") == 0) {
		ie64_narrow(fn);
		filluse(fn);
	}
	gcm(fn);
	filluse(fn);
	ssacheck(fn);
	if (optlevel >= 2 && strcmp(T.name, "ie64") == 0) {
		if (ie64_threadstate(fn)) {
			gvn(fn);
			fillcfg(fn);
			simplcfg(fn);
			filluse(fn);
			filldom(fn);
			gcm(fn);
			filluse(fn);
			ssacheck(fn);
		}
		ie64_bitopt(fn);
	}
	if (optlevel >= 2 && T.cansel) {
		ifconvert(fn);
		fillcfg(fn);
		filluse(fn);
		filldom(fn);
		ssacheck(fn);
	}
	if (strictalias && optlevel >= 2)
		lpromote(fn);
	if (optlevel >= 2 && strcmp(T.name, "ie64") == 0) {
		ie64_searchloops(fn);
		reduced = lreduce(fn);
		if (lrowreduce(fn) || reduced) {
			gcm(fn);
			filluse(fn);
			ssacheck(fn);
		}
	}
	if (strcmp(T.name, "ie64") == 0)
		ie64_lower_fpext(fn);
	T.abi1(fn);
	simpl(fn);
	fillcfg(fn);
	filluse(fn);
	T.isel(fn);
	fillcfg(fn);
	filllive(fn);
	fillloop(fn);
	fillcost(fn);
	spill(fn);
	rega(fn);
	lowervolatile(fn);
	fillcfg(fn);
	simpljmp(fn);
	fillcfg(fn);
	assert(fn->rpo[0] == fn->start);
	for (n=0;; n++)
		if (n == fn->nblk-1) {
			fn->rpo[n]->link = 0;
			break;
		} else
			fn->rpo[n]->link = fn->rpo[n+1];
	if (!dbg) {
		T.emitfn(fn, outf);
		if (strcmp(T.name, "ie64") != 0)
			fprintf(outf, "/* end function %s */\n\n", fn->name);
	} else
		fprintf(stderr, "\n");
}

/* With inlining the functions of a unit are held until the unit has been
 * parsed.  Only the IE64 target does so, at -O2 and above. */
static Fn **fnq;
static int nfnq, capfnq;
static int inlining, noinline, nosinkfuse;
extern int ie64_branch_imm;
extern int ie64_signed_loads;
extern void ie64_profileload(char *);
static char *profpath;

static void flushfuncs(void);

static void
func(Fn *fn)
{
	if (!fn) {
		flushfuncs();
		return;
	}
	checktargetops(fn);
	if (!inlining) {
		funcdo(fn);
		freeall();
		return;
	}
	if (nfnq == capfnq) {
		capfnq = capfnq ? 2*capfnq : 64;
		fnq = realloc(fnq, capfnq * sizeof fnq[0]);
		if (!fnq)
			die("out of memory");
	}
	fnq[nfnq++] = fn;
}

static void
flushfuncs(void)
{
	int n;

	if (!nfnq)
		return;
	inlinefns(fnq, nfnq);
	for (n=0; n<nfnq; n++)
		funcdo(fnq[n]);
	nfnq = 0;
	freeall();
}

static void
dbgfile(char *fn)
{
	emitdbgfile(fn, outf);
}

int
main(int ac, char *av[])
{
	Target **t;
	FILE *inf, *hf;
	char *f, *sep;
	int c;

	T = Deftgt;
	outf = stdout;
	while ((c = getopt(ac, av, "hd:f:m:O:o:t:")) != -1)
		switch (c) {
		case 'd':
			for (; *optarg; optarg++)
				if (isalpha(*optarg)) {
					debug[toupper(*optarg)] = 1;
					dbg = 1;
				}
			break;
		case 'f':
			if (strcmp(optarg, "no-inline") == 0) {
				noinline = 1;
				break;
			}
			if (strcmp(optarg, "strict-aliasing") == 0) {
				strictalias = 1;
				break;
			}
			if (strcmp(optarg, "no-sink-fuse") == 0) {
				nosinkfuse = 1;
				break;
			}
			if (strncmp(optarg, "profile-use=", sizeof "profile-use=" - 1) == 0) {
				profpath = optarg + sizeof "profile-use=" - 1;
				break;
			}
			if (strncmp(optarg, "ie64-unit-prefix=",
			    sizeof "ie64-unit-prefix=" - 1) != 0) {
				fprintf(stderr, "invalid option '-f%s'\n", optarg);
				exit(1);
			}
			break;
		case 'm':
			if (strcmp(optarg, "branch-imm") == 0)
				ie64_branch_imm = 1;
			else if (strcmp(optarg, "signed-loads") == 0)
				ie64_signed_loads = 1;
			else if (strcmp(optarg, "fpext") == 0 || strcmp(optarg, "no-fpext") == 0)
				ie64_fpext = optarg[0] == 'f';
			else if (strcmp(optarg, "fma") == 0 || strcmp(optarg, "no-fma") == 0)
				ie64_fma = optarg[0] == 'f';
			else {
				fprintf(stderr, "invalid option '-m%s'\n", optarg);
				exit(1);
			}
			break;
		case 'o':
			if (strcmp(optarg, "-") != 0) {
				outf = fopen(optarg, "w");
				if (!outf) {
					fprintf(stderr, "cannot open '%s'\n", optarg);
					exit(1);
				}
			}
			break;
		case 'O':
			if (optarg[0] < '0' || optarg[0] > '3' || optarg[1]) {
				fprintf(stderr, "invalid optimization level '%s'\n", optarg);
				exit(1);
			}
			optlevel = optarg[0] - '0';
			break;
		case 't':
			if (strcmp(optarg, "?") == 0) {
				puts(T.name);
				exit(0);
			}
			for (t=tlist;; t++) {
				if (!*t) {
					fprintf(stderr, "unknown target '%s'\n", optarg);
					exit(1);
				}
				if (strcmp(optarg, (*t)->name) == 0) {
					T = **t;
					break;
				}
			}
			break;
		case 'h':
		default:
			hf = c != 'h' ? stderr : stdout;
			fprintf(hf, "%s [OPTIONS] {file.ssa, -}\n", av[0]);
			fprintf(hf, "\t%-11s prints this help\n", "-h");
			fprintf(hf, "\t%-11s output to file\n", "-o file");
			fprintf(hf, "\t%-11s set optimization level (default: 2)\n", "-O <0-3>");
			fprintf(hf, "\t%-11s generate for a target among:\n", "-t <target>");
			fprintf(hf, "\t%-11s ", "");
			for (t=tlist, sep=""; *t; t++, sep=", ") {
				fprintf(hf, "%s%s", sep, (*t)->name);
				if (*t == &Deftgt)
					fputs(" (default)", hf);
			}
			fprintf(hf, "\n");
			fprintf(hf, "\t%-11s dump debug information\n", "-d <flags>");
			fprintf(hf, "\t%-11s do not inline functions (ie64, -O2 and up)\n", "-fno-inline");
			fprintf(hf, "\t%-11s keep loop accumulators in registers when other accesses are disjoint\n", "-fstrict-aliasing");
			fprintf(hf, "\t%-11s do not copy shared compares and addresses to their uses (ie64)\n", "-fno-sink-fuse");
			fprintf(hf, "\t%-11s lay out blocks by an execution profile (ie64)\n", "-fprofile-use=FILE");
			fprintf(hf, "\t%-11s compare-and-branch with an immediate (ie64, needs engine support)\n", "-mbranch-imm");
			fprintf(hf, "\t%-11s signed narrow loads (ie64, needs engine support)\n", "-msigned-loads");
			exit(c != 'h');
		}
	if (ie64_branch_imm && strcmp(T.name, "ie64") != 0) {
		fprintf(stderr, "-mbranch-imm needs the ie64 target\n");
		exit(1);
	}
	if (ie64_signed_loads && strcmp(T.name, "ie64") != 0) {
		fprintf(stderr, "-msigned-loads needs the ie64 target\n");
		exit(1);
	}
	if ((ie64_fpext || ie64_fma) && strcmp(T.name, "ie64") != 0) {
		fprintf(stderr, "-mfpext and -mfma need the ie64 target\n");
		exit(1);
	}
	if (profpath) {
		if (strcmp(T.name, "ie64") != 0) {
			fprintf(stderr, "-fprofile-use needs the ie64 target\n");
			exit(1);
		}
		ie64_profileload(profpath);
	}
	if (nosinkfuse)
		T.sinkfuse = 0;
	inlining = strcmp(T.name, "ie64") == 0 && optlevel >= 2 && !noinline && !dbg;
	do {
		f = av[optind];
		if (!f || strcmp(f, "-") == 0) {
			inf = stdin;
			f = "-";
		} else {
			inf = fopen(f, "r");
			if (!inf) {
				fprintf(stderr, "cannot open '%s'\n", f);
				exit(1);
			}
		}
		parse(inf, f, dbgfile, data, func);
		fclose(inf);
	} while (++optind < ac);

	if (!dbg)
		T.emitfin(outf);

	exit(0);
}
