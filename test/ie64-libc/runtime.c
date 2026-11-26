/* SPDX-License-Identifier: MIT */

#include <errno.h>
#include <fenv.h>
#include <intuitionengine.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static const unsigned lengths[] = {0, 1, 7, 8, 9, 31, 32, 33, 63, 64, 65, 127, 128, 129};
static _Alignas(8) unsigned char source[144], dest[144];

static int
memory(void)
{
	unsigned i, s, d, j, n;

	for (j = 0; j < sizeof source; j++)
		source[j] = (unsigned char)(0x80u + j);
	for (s = 0; s < 8; s++) {
		for (d = 0; d < 8; d++) {
			for (i = 0; i < sizeof lengths / sizeof lengths[0]; i++) {
				n = lengths[i];
				for (j = 0; j < sizeof dest; j++)
					dest[j] = 0x41;
				if (memcpy(dest + d, source + s, n) != dest + d)
					return 1;
				for (j = 0; j < sizeof dest; j++)
					if (dest[j] != (j >= d && j < d+n ? source[s+j-d] : 0x41))
						return 2;
				if (memset(dest + d, -129, n) != dest + d)
					return 3;
				for (j = 0; j < sizeof dest; j++)
					if (dest[j] != (j >= d && j < d+n ? 127 : 0x41))
						return 4;
				dest[d+n] = 0;
				dest[d+n+1] = 1;
				if (strlen((const char *)dest + d) != n)
					return 5;
			}
		}
	}
	return 0;
}

static int
page_boundaries(void)
{
	const uintptr_t page_table = 0x83000;
	volatile unsigned char *src = (volatile unsigned char *)0x61ffb;
	volatile unsigned char *dst = (volatile unsigned char *)0x65ffe;
	volatile uint64_t *table;
	unsigned level, page, j;
	int status = 0;

	for (level = 0; level < 5; level++) {
		table = (volatile uint64_t *)(page_table + level * 4096);
		table[0] = (page_table + (level+1)*4096) | 1;
	}
	table = (volatile uint64_t *)(page_table + 5*4096);
	for (page = 0; page < 0x9f; page++)
		table[page] = (uint64_t)page * 4096 | 0x1f;

	/* Each virtual range spans two non-contiguous physical pages. */
	table[0x61] = 0x7101f;
	table[0x62] = 0x7401f;
	table[0x65] = 0x7501f;
	table[0x66] = 0x7c01f;
	__builtin_ie64_mtcr(IE64_CR_PTBR, page_table);
	__builtin_ie64_mtcr(IE64_CR_MMU_CTRL, 1);
	for (j = 0; j < 16; j++) {
		src[j] = 0xe0 + j;
		dst[j] = 0x41;
	}
	memcpy((void *)dst, (const void *)src, 13);
	for (j = 0; j < 16; j++)
		if (dst[j] != (j < 13 ? 0xe0 + j : 0x41))
			status = 6;
	memset((void *)dst, 0xab, 12);
	dst[12] = 0;
	for (j = 0; j < 12; j++)
		if (dst[j] != 0xab)
			status = 7;
	if (strlen((const char *)dst) != 12)
		status = 8;
	__builtin_ie64_mtcr(IE64_CR_MMU_CTRL, 0);
	return status;
}

static int
roots(void)
{
	volatile union { double f; uint64_t u; } d;
	volatile union { float f; uint32_t u; } s;

	if (fegetround() != FE_TONEAREST)
		return 9;
	errno = 17;
	d.u = 0x8000000000000000ull;
	s.u = 0x80000000u;
	d.f = sqrt(d.f);
	s.f = sqrtf(s.f);
	if (d.u != 0x8000000000000000ull || s.u != 0x80000000u || errno != 17)
		return 10;
	d.u = 0x4000000000000000ull;
	s.u = 0x40000000u;
	d.f = sqrt(d.f);
	s.f = sqrtf(s.f);
	if (d.u != 0x3ff6a09e667f3bcdull || s.u != 0x3fb504f3u || errno != 17)
		return 11;
	d.u = 1;
	s.u = 1;
	d.f = sqrt(d.f);
	s.f = sqrtf(s.f);
	if (d.u != 0x1e60000000000000ull || s.u != 0x1a3504f3u || errno != 17)
		return 12;
	d.u = 0x7ff0000000000000ull;
	s.u = 0x7f800000u;
	d.f = sqrt(d.f);
	s.f = sqrtf(s.f);
	if (d.u != 0x7ff0000000000000ull || s.u != 0x7f800000u || errno != 17)
		return 13;
	d.u = 0xfff800000000042aull;
	s.u = 0x7fc0042au;
	d.f = sqrt(d.f);
	s.f = sqrtf(s.f);
	if (d.u != 0xfff800000000042aull || s.u != 0x7fc0042au || errno != 17)
		return 14;
	d.u = 0x7ff000000000042aull;
	s.u = 0xff80042au;
	d.f = sqrt(d.f);
	s.f = sqrtf(s.f);
	if (!isnan(d.f) || !isnan(s.f) || errno != 17)
		return 15;
	d.u = 0xfff0000000000000ull;
	if (!isnan(sqrt(d.f)) || errno != ((math_errhandling & MATH_ERRNO) ? EDOM : 17))
		return 16;
	errno = 0;
	s.u = 0xc0800000u;
	if (!isnan(sqrtf(s.f)) || errno != ((math_errhandling & MATH_ERRNO) ? EDOM : 0))
		return 17;
	return 0;
}

int
main(void)
{
	int result;

	result = memory();
	if (!result)
		result = page_boundaries();
	if (!result)
		result = roots();
	if (!result)
		*(volatile uint32_t *)IE_INPUT_TERM_OUT = 'K';
	*(volatile uint32_t *)IE_SYSINFO_EXIT = result;
	__builtin_ie64_halt();
	return result;
}
