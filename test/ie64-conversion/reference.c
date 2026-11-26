#include <stdint.h>
#ifdef HOST_REFERENCE
#include <stdio.h>
#else
#include <intuitionengine.h>
#endif

#ifdef HOST_REFERENCE
float u64_f32(uint64_t x) { return (float)x; }
double u64_f64(uint64_t x) { return (double)x; }
#else
float u64_f32(uint64_t);
double u64_f64(uint64_t);
#endif

static uint64_t single(float f) { return ((union { float f; uint32_t u; }){.f = f}).u; }
static uint64_t wide(double f) { return ((union { double f; uint64_t u; }){.f = f}).u; }

static uint64_t hash;

static void
hex(uint64_t x)
{
	static const char digits[] = "0123456789abcdef";
	unsigned i;

#ifdef HOST_REFERENCE
	for (i=0; i<16; i++)
		putchar(digits[(x >> (60 - 4*i)) & 15]);
	putchar(10);
#else
	volatile uint32_t *term = (volatile uint32_t *)IE_INPUT_TERM_OUT;
	for (i=0; i<16; i++)
		*term = digits[(x >> (60 - 4*i)) & 15];
	*term = 10;
#endif
}

static void
check(uint64_t x)
{
	uint64_t a = single(u64_f32(x));
	uint64_t b = wide(u64_f64(x));

	if (x < 4 || x == UINT64_C(0x8000008000000001)) {
		hex(x);
		hex(a);
		hex(b);
	}
	hash = ((hash ^ x) * UINT64_C(1099511628211) ^ a) * UINT64_C(1099511628211) ^ b;
}

int
main(void)
{
	uint64_t x = UINT64_C(0x5f8eb39aa2f547b1);
	uint64_t base, step, midpoint;
	int bit, precision, offset, i;

	for (i=0; i<4096; i++) {
		check(i);
		check(UINT64_MAX - i);
	}
	for (bit=0; bit<64; bit++) {
		base = UINT64_C(1) << bit;
		for (offset=-64; offset<=64; offset++)
			check(base + offset);
		for (precision=24; precision<=53; precision+=29) {
			if (bit < precision)
				continue;
			step = UINT64_C(1) << (bit - precision + 1);
			/* Both parities of the shortened significand. */
			for (i=0; i<16; i++) {
				midpoint = base + i*step + step/2;
				for (offset=-8; offset<=8; offset++)
					check(midpoint + offset);
			}
		}
	}
	for (i=0; i<32768; i++) {
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		check(x);
		check(x >> (i & 63));
	}
	hex(hash);
#ifndef HOST_REFERENCE
	*(volatile uint32_t *)IE_SYSINFO_EXIT = 0;
	__builtin_ie64_halt();
#endif
	return 0;
}
