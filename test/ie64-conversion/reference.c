#include <stdint.h>
#ifdef HOST_REFERENCE
#include <stdio.h>
#else
#include <intuitionengine.h>
#endif

#ifdef HOST_REFERENCE
#define CAST(name, from, to) to name(from x) { return (to)x; }
#else
#define CAST(name, from, to) to name(from);
#endif
CAST(u64_f32, uint64_t, float)
CAST(f32_u64, float, uint64_t)
CAST(u64_f64, uint64_t, double)
CAST(f64_u64, double, uint64_t)
CAST(s64_f32, int64_t, float)
CAST(f32_s64, float, int64_t)
CAST(s64_f64, int64_t, double)
CAST(f64_s64, double, int64_t)
CAST(u32_f32, uint32_t, float)
CAST(f32_u32, float, uint32_t)
CAST(u32_f64, uint32_t, double)
CAST(f64_u32, double, uint32_t)
CAST(s32_f32, int32_t, float)
CAST(f32_s32, float, int32_t)
CAST(s32_f64, int32_t, double)
CAST(f64_s32, double, int32_t)
CAST(narrow, double, float)
CAST(extend, float, double)
#undef CAST

static uint64_t single(float f) { return ((union { float f; uint32_t u; }){.f = f}).u; }
static uint64_t wide(double f) { return ((union { double f; uint64_t u; }){.f = f}).u; }

static uint64_t hash[19];

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
check_exceptional(void)
{
	/* Native FCVTFI and DCVTFI define these exceptional results. A C
	 * floating-to-integer cast cannot provide an oracle here. */
#ifdef HOST_REFERENCE
	hex(INT32_MAX);
	hex((uint32_t)INT32_MIN);
	hex(0);
	hex(INT64_MAX);
	hex((uint64_t)INT64_MIN);
	hex(0);
#else
	float inf = ((union { uint32_t u; float f; }){.u = UINT32_C(0x7f800000)}).f;
	float nan = ((union { uint32_t u; float f; }){.u = UINT32_C(0x7fc01234)}).f;
	double dinf = ((union { uint64_t u; double f; }){.u = UINT64_C(0x7ff0000000000000)}).f;
	double dnan = ((union { uint64_t u; double f; }){.u = UINT64_C(0x7ff8000000001234)}).f;
	hex((uint32_t)f32_s32(inf));
	hex((uint32_t)f32_s32(-inf));
	hex((uint32_t)f32_s32(nan));
	hex((uint64_t)f64_s64(dinf));
	hex((uint64_t)f64_s64(-dinf));
	hex((uint64_t)f64_s64(dnan));
#endif
}

static void
mix(unsigned index, uint64_t bits)
{
	hash[index] = (hash[index] ^ bits) * UINT64_C(1099511628211);
}

static void
check(uint64_t x)
{
	uint64_t a = single(u64_f32(x));
	uint64_t b = wide(u64_f64(x));
	float f = ((union { uint32_t u; float f; }){.u = (uint32_t)x}).f;
	double d = ((union { uint64_t u; double f; }){.u = x}).f;

	mix(0, x);
	mix(1, a);
	mix(2, b);
	mix(3, single(s64_f32((int64_t)x)));
	mix(4, wide(s64_f64((int64_t)x)));
	mix(5, single(u32_f32((uint32_t)x)));
	mix(6, wide(u32_f64((uint32_t)x)));
	mix(7, single(s32_f32((int32_t)x)));
	mix(8, wide(s32_f64((int32_t)x)));
	/* Only an integer result in range has C semantics. The lower unsigned
	 * bound admits negative fractions whose truncation is zero. */
	if (f >= -0x1p31f && f < 0x1p31f) mix(9, (uint32_t)f32_s32(f));
	if (d > -0x1p31 - 1 && d < 0x1p31) mix(10, (uint32_t)f64_s32(d));
	if (f >= -0x1p63f && f < 0x1p63f) mix(11, (uint64_t)f32_s64(f));
	if (d >= -0x1p63 && d < 0x1p63) mix(12, (uint64_t)f64_s64(d));
	if (f > -1.0f && f < 0x1p32f) mix(13, f32_u32(f));
	if (d > -1.0 && d < 0x1p32) mix(14, f64_u32(d));
	if (f > -1.0f && f < 0x1p64f) mix(15, f32_u64(f));
	if (d > -1.0 && d < 0x1p64) mix(16, f64_u64(d));
	/* NaN comparison and payload preservation are separate from C casts. */
	if (f == f) mix(17, wide(extend(f)));
	if (d == d && d >= -0x1.fffffep127 && d <= 0x1.fffffep127)
		mix(18, single(narrow(d)));
}

int
main(void)
{
	uint64_t x;
	uint64_t base, step, midpoint;
	int bit, precision, offset, i;

	check_exceptional();
	/* The first binary32 halfway value above 2^62, plus one. */
	x = UINT64_C(0x4000004000000001);
	hex(single(s64_f32((int64_t)x)));
	hex(single(s64_f32(-(int64_t)x)));
	x = UINT64_C(0x5f8eb39aa2f547b1);
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
	for (i=0; i<19; i++)
		hex(hash[i]);
#ifndef HOST_REFERENCE
	*(volatile uint32_t *)IE_SYSINFO_EXIT = 0;
	__builtin_ie64_halt();
#endif
	return 0;
}
