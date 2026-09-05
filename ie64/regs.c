#include "all.h"

bits
ie64_retregs(Ref r, int p[2])
{
	bits b = 0;
	int ngp, nfp;

	assert(rtype(r) == RCall);
	ngp = r.val & 3;
	nfp = (r.val >> 2) & 3;
	if (p) {
		p[0] = ngp;
		p[1] = nfp;
	}
	while (ngp--)
		b |= BIT(R1 + ngp);
	while (nfp--)
		b |= BIT(F0 + nfp);
	return b;
}

bits
ie64_argregs(Ref r, int p[2])
{
	bits b = 0;
	int ngp, nfp, fmask, envc, j;

	assert(rtype(r) == RCall);
	ngp = (r.val >> 4) & 15;
	nfp = (r.val >> 8) & 15;
	fmask = (r.val >> 12) & 255;
	envc = (r.val >> 20) & 1;
	if (p)
		p[0] = ngp + envc;
	if (envc)
		b |= BIT(RENV);
	while (ngp--)
		b |= BIT(R1 + ngp);
	if (fmask) {
		for (j=0; j<8; j++)
			if (fmask & (1 << j))
				b |= BIT(F0 + j);
		nfp = 0;
		for (j=0; j<8; j++)
			if (fmask & (1 << j))
				nfp++;
	} else {
		if (p)
			p[1] = nfp;
		while (nfp--)
			b |= BIT(F0 + nfp);
		return b;
	}
	if (p)
		p[1] = nfp;
	return b;
}
