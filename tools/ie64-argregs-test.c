#include "../ie64/all.h"

int
main(void)
{
	bits b;
	int p[2];

	/* A float in F0 followed by an aligned double in F2:F3. */
	b = ie64_argregs(CALL((4 << 8) | (13 << 12)), p);
	assert(p[0] == 0);
	assert(p[1] == 3);
	assert(b == (BIT(F0) | BIT(F2) | BIT(F3)));

	/* The env pseudo-argument rides in its own register and counts towards
	 * the integer argument total without occupying an argument register. */
	b = ie64_argregs(CALL(1 << 20), p);
	assert(p[0] == 1);
	assert(p[1] == 0);
	assert(b == BIT(RENV));

	/* It does not displace the ordinary sequence either. */
	b = ie64_argregs(CALL((2 << 4) | (1 << 20)), p);
	assert(p[0] == 3);
	assert(b == (BIT(R1) | BIT(R2) | BIT(RENV)));
	return 0;
}
