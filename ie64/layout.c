#include "all.h"
#include <errno.h>

/*
 * Profile-guided block layout (-fprofile-use=FILE).
 *
 * The profile holds, per function, how often each emitted block ran and,
 * for a conditional branch, how often it was taken and where it went.
 * Blocks are chained so that the likely successor of a block follows it.
 * The IE64 JIT runs a taken conditional branch more slowly than a
 * not-taken one or a bra, so the pass minimises
 *
 *	CT * (taken conditional branches) + CB * (executed bra)
 *
 * A block without a profile entry has weight 0 and keeps its place.
 *
 * File format, one record per line, '#' starts a comment:
 *
 *	B <function> <label> <executed> <taken> <target>
 *
 * <label> and <target> are the numbers of the emitted .L<number> labels,
 * which are the same in the compile that produced the profile and in the
 * compile that reads it.  A <label> of -1 is the entry block of the
 * function when it has no label.  <taken> is -1 for a block that does not end in
 * a conditional branch.
 */

typedef struct Prec Prec;
struct Prec {
	char *fn;
	long lbl;
	long long exec;
	long long taken;
	long target;
	int bad;
};

static Prec *prof;
static size_t nprof, capprof;

/* swapped[b->id] is set when neither successor of conditional block b follows
 * it and the first successor is the likelier one. A bra is cheaper than a
 * taken conditional branch, so the block then reaches the likelier first
 * successor by a bra and the second by an inverted conditional branch. */
static char *swapped;
static uint nswapped;

int
ie64_layout_swap(Blk *b)
{
	return swapped && b->id < nswapped && swapped[b->id];
}

/* Relative costs of the three ways a block can leave, from the IE64 JIT
 * microbenchmark: a taken conditional branch, an executed bra. */
#define CT 19
#define CB 8

/* Counts are clamped so that the cost arithmetic cannot overflow. */
#define MAXCOUNT (1LL << 50)

static int
reccmp(const void *a, const void *b)
{
	const Prec *x = a, *y = b;
	int c;

	c = strcmp(x->fn, y->fn);
	if (c)
		return c;
	return x->lbl < y->lbl ? -1 : x->lbl > y->lbl;
}

/* Two functions of the same name in different units (file-scope statics)
 * can have the same label numbers. A profile that holds one label twice for
 * a function name cannot say which function it describes, so the pass
 * ignores every record of that name. */
static void
markambiguous(void)
{
	Prec *s;
	size_t i, j;

	if (nprof < 2)
		return;
	s = emalloc(nprof * sizeof s[0]);
	memcpy(s, prof, nprof * sizeof s[0]);
	qsort(s, nprof, sizeof s[0], reccmp);
	for (i=1; i<nprof; i++)
		if (reccmp(&s[i-1], &s[i]) == 0)
			for (j=0; j<nprof; j++)
				if (strcmp(prof[j].fn, s[i].fn) == 0)
					prof[j].bad = 1;
	free(s);
}

/* A count is a decimal number, or -1 for "not a conditional branch". A
 * number beyond MAXCOUNT is clamped, so that the cost arithmetic cannot
 * overflow. */
static long long
parsecount(char *tok, char *path, char *line)
{
	char *end;
	long long v;

	errno = 0;
	v = strtoll(tok, &end, 10);
	if (end == tok || *end || (errno == ERANGE && v < 0) || v < -1) {
		fprintf(stderr, "malformed profile line in '%s': %s", path, line);
		exit(1);
	}
	if (errno == ERANGE || v > MAXCOUNT)
		v = MAXCOUNT;
	return v;
}

void
ie64_profileload(char *path)
{
	FILE *f;
	char line[1024], name[512], ex[64], tk[64];
	Prec r;

	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "cannot open profile '%s'\n", path);
		exit(1);
	}
	while (fgets(line, sizeof line, f)) {
		if (line[0] == '#' || line[0] == '\n')
			continue;
		if (sscanf(line, "B %511s %ld %63s %63s %ld", name, &r.lbl,
		    ex, tk, &r.target) != 5) {
			fprintf(stderr, "malformed profile line in '%s': %s", path, line);
			exit(1);
		}
		r.exec = parsecount(ex, path, line);
		r.taken = parsecount(tk, path, line);
		if (r.exec < 0)
			r.exec = 0;
		if (r.taken > r.exec)
			r.taken = r.exec;
		if (nprof == capprof) {
			capprof = capprof ? 2 * capprof : 1024;
			prof = realloc(prof, capprof * sizeof prof[0]);
			if (!prof) {
				fputs("out of memory\n", stderr);
				exit(1);
			}
		}
		r.bad = 0;
		r.fn = emalloc(strlen(name) + 1);
		strcpy(r.fn, name);
		prof[nprof++] = r;
	}
	if (ferror(f)) {
		fprintf(stderr, "cannot read profile '%s'\n", path);
		exit(1);
	}
	fclose(f);
	markambiguous();
}

typedef struct Edge Edge;
struct Edge {
	long long gain;
	uint from, to;	/* indexes in the original order */
	uint ord;
};

static int
edgecmp(const void *a, const void *b)
{
	const Edge *x = a, *y = b;

	if (x->gain != y->gain)
		return x->gain < y->gain ? 1 : -1;
	return x->ord < y->ord ? -1 : x->ord > y->ord;
}

static long long
condcost(long long wt, long long wf, int next)
{
	long long k, i;

	if (next == 2)		/* the not-taken successor follows */
		return CT * wt;
	if (next == 1)		/* the taken successor follows, branch inverted */
		return CT * wf;
	k = CT * wt + CB * wf;
	i = CT * wf + CB * wt;
	return k < i ? k : i;
}

/* Reorder the blocks of fn.  id0 is the number of the first label of fn. */
void
ie64_layout(Fn *fn, int id0)
{
	Blk **ord, **out, *b;
	long long *exec, *taken, *wsucc1, *wsucc2;
	char *known;
	uint n, nid, i, k, nedge, *head, *tail, *next, *chain, nout;
	Edge *es;
	size_t p;
	int any;

	swapped = 0;
	nswapped = 0;
	/* Blocks that jump merging removed are no longer linked, but their
	 * numbers are still reserved. */
	nid = fn->nblk;
	for (n=0, b=fn->start; b; b=b->link)
		n++;
	if (!nprof || n < 3)
		return;
	ord = alloc(n * sizeof ord[0]);
	for (i=0, b=fn->start; b; b=b->link)
		ord[i++] = b;
	exec = alloc(n * sizeof exec[0]);
	taken = alloc(n * sizeof taken[0]);
	wsucc1 = alloc(n * sizeof wsucc1[0]);
	wsucc2 = alloc(n * sizeof wsucc2[0]);
	known = alloc(n);
	memset(known, 0, n);
	any = 0;
	for (i=0; i<n; i++)
		taken[i] = -1;
	for (p=0; p<nprof; p++) {
		if (prof[p].bad || strcmp(prof[p].fn, fn->name) != 0)
			continue;
		if (prof[p].lbl != -1 && (prof[p].lbl < id0 || prof[p].lbl >= id0 + (long)nid))
			continue;
		/* -1 names the entry block, which has no label unless a
		 * branch comes back to it. */
		for (i=0; i<n; i++)
			if (prof[p].lbl == -1 ? i == 0 : (long)(id0 + ord[i]->id) == prof[p].lbl)
				break;
		if (i == n)
			continue;
		exec[i] = prof[p].exec;
		taken[i] = prof[p].taken;
		known[i] = 1;
		any = 1;
		if (prof[p].taken >= 0 && ord[i]->jmp.type == Jjnz) {
			if (ord[i]->s1 && (long)(id0 + ord[i]->s1->id) == prof[p].target) {
				wsucc1[i] = prof[p].taken;
				wsucc2[i] = prof[p].exec - prof[p].taken;
			} else if (ord[i]->s2 && (long)(id0 + ord[i]->s2->id) == prof[p].target) {
				wsucc2[i] = prof[p].taken;
				wsucc1[i] = prof[p].exec - prof[p].taken;
			} else
				known[i] = 0;
		}
	}
	if (!any)
		return;
	swapped = alloc(nid);
	memset(swapped, 0, nid);
	nswapped = nid;
	for (i=0; i<n; i++)
		if (known[i] && ord[i]->jmp.type == Jjnz && taken[i] >= 0)
			swapped[ord[i]->id] = wsucc1[i] > wsucc2[i];
	/* index of a block in the original order, by block id */
	head = alloc(nid * sizeof head[0]);	/* head[id] = index */
	for (i=0; i<n; i++)
		head[ord[i]->id] = i;
	es = alloc(2 * n * sizeof es[0]);
	nedge = 0;
	for (i=0; i<n; i++) {
		b = ord[i];
		if (!known[i])
			continue;
		if (b->jmp.type == Jjmp && b->s1) {
			es[nedge].gain = CB * exec[i];
			es[nedge].from = i;
			es[nedge].to = head[b->s1->id];
			es[nedge].ord = nedge;
			nedge++;
		} else if (b->jmp.type == Jjnz && taken[i] >= 0) {
			long long w1 = wsucc1[i], w2 = wsucc2[i];
			long long base = condcost(w1, w2, 0);

			/* s1 follows: s2 is the other way out.  Same gain
			 * as the model above with the roles swapped. */
			es[nedge].gain = base - condcost(w2, w1, 2);
			es[nedge].from = i;
			es[nedge].to = head[b->s1->id];
			es[nedge].ord = nedge;
			nedge++;
			es[nedge].gain = base - condcost(w1, w2, 2);
			es[nedge].from = i;
			es[nedge].to = head[b->s2->id];
			es[nedge].ord = nedge;
			nedge++;
		}
	}
	qsort(es, nedge, sizeof es[0], edgecmp);
	/* chains as singly linked lists */
	next = alloc(n * sizeof next[0]);
	tail = alloc(n * sizeof tail[0]);	/* tail[i] valid for a chain head */
	chain = alloc(n * sizeof chain[0]);	/* chain[i] = head of i's chain */
	for (i=0; i<n; i++) {
		next[i] = n;
		tail[i] = i;
		chain[i] = i;
	}
	for (k=0; k<nedge; k++) {
		uint a = es[k].from, c = es[k].to;
		uint ha, hc;

		if (es[k].gain <= 0 || c == 0 || a == c)
			continue;
		ha = chain[a];
		hc = chain[c];
		if (ha == hc || tail[ha] != a || hc != c)
			continue;
		next[a] = c;
		tail[ha] = tail[hc];
		for (i=c; i!=n; i=next[i])
			chain[i] = ha;
	}
	/* The entry chain first, then the others by their first block. */
	out = alloc(n * sizeof out[0]);
	nout = 0;
	for (i=chain[0]; i!=n; i=next[i])
		out[nout++] = ord[i];
	for (k=1; k<n; k++) {
		if (chain[k] != k)
			continue;
		for (i=k; i!=n; i=next[i])
			out[nout++] = ord[i];
	}
	if (nout != n)
		return;
	for (i=0; i<n; i++)
		out[i]->link = i+1 < n ? out[i+1] : 0;
	fn->start = out[0];
}
