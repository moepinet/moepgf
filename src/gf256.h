/*
 * This is a library providing arithmetic functions on GF(2^1) and GF(2^8).
 * Copyright (C) 2013  Alexander Kurtz <alexander@kurtz.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _GF256_H_
#define _GF256_H_

#include <stdint.h>

uint8_t
ffinv256(uint8_t element);

void
ffadd256_region_c_gpr(uint8_t *region1, const uint8_t *region2, int length);

void
ffmadd256_region_c_gpr(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);

void
ffmadd256_region_c_slow(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);

void
ffmul256_region_c_gpr(uint8_t *region, uint8_t constant, int length);

void
ffmul256_region_c_slow(uint8_t *region, uint8_t constant, int length);

#ifdef __x86_64__
void
ffadd256_region_sse2(uint8_t *region1, const uint8_t *region2, int length);

void
ffadd256_region_avx2(uint8_t *region1, const uint8_t *region2, int length);

void
ffdiv256_region_c_sse2(uint8_t *region, uint8_t constant, int length);

void
ffdiv256_region_c_avx2(uint8_t *region, uint8_t constant, int length);

void
ffmadd256_region_c_sse2(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);

void
ffmadd256_region_c_sse41(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);

void
ffmadd256_region_c_avx2(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);

void
ffmul256_region_c_sse2(uint8_t *region, uint8_t constant, int length);

void
ffmul256_region_c_sse41(uint8_t *region, uint8_t constant, int length);

void
ffmul256_region_c_avx2(uint8_t *region, uint8_t constant, int length);
#endif

#ifdef __arm__
void
ffadd256_region_neon(uint8_t *region1, const uint8_t *region2, int length);

void
ffdiv256_region_c_neon(uint8_t *region, uint8_t constant, int length);

void
ffmadd256_region_c_neon(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);

void
ffmul256_region_c_neon(uint8_t *region, uint8_t constant, int length);
#endif

#endif
