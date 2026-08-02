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
	return 0;
}
