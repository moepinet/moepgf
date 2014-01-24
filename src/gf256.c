/*
 * This file is part of moep80211gf.
 * 
 * Copyright (C) 2014 	Stephan M. Guenther <moepi@moepi.net>
 * Copyright (C) 2014 	Maximilian Riemensberger <riemensberger@tum.de>
 * Copyright (C) 2013 	Alexander Kurtz <alexander@kurtz.be>
 * 
 * moep80211gf is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 of the License.
 * 
 * moep80211gf is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License * along
 * with moep80211gf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gf256.h"
#include "gf.h"

#if GF256_POLYNOMIAL == 285
#include "gf256tables285.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t inverses[GF256_SIZE] = GF256_INV_TABLE;
static const uint8_t pt[GF256_SIZE][GF256_EXPONENT] = GF256_POLYNOMIAL_DIV_TABLE;
static const uint8_t tl[GF256_SIZE][16] = GF256_SHUFFLE_LOW_TABLE;
static const uint8_t th[GF256_SIZE][16] = GF256_SHUFFLE_HIGH_TABLE;
static const uint8_t alogt[2*GF256_SIZE-1] = GF256_ALOG_TABLE;
static const uint8_t logt[GF256_SIZE] = GF256_LOG_TABLE;
static const uint8_t mult[GF256_SIZE][GF256_SIZE] = GF256_MUL_TABLE;

inline uint8_t
ffinv256(uint8_t element)
{
	return inverses[element];
}

inline void
ffadd256_region_gpr(uint8_t *region1, const uint8_t *region2, int length)
{
	ffxor_region_gpr(region1, region2, length);
}

void
ffmadd256_region_c_slow(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[8];

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_gpr(region1, region2, length);
		return ;
	}

	for (; length; region1++, region2++, length--) {
		r[0] = (*region2 &   1) ? p[0] : 0;
		r[1] = (*region2 &   2) ? p[1] : 0;
		r[2] = (*region2 &   4) ? p[2] : 0;
		r[3] = (*region2 &   8) ? p[3] : 0;
		r[4] = (*region2 &  16) ? p[4] : 0;
		r[5] = (*region2 &  32) ? p[5] : 0;
		r[6] = (*region2 &  64) ? p[6] : 0;
		r[7] = (*region2 & 128) ? p[7] : 0;
		*region1 ^= r[0]^r[1]^r[2]^r[3]^r[4]^r[5]^r[6]^r[7];
	}
}

void
ffmadd256_region_c_gpr(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[8];
	uint64_t r64[8];
	
	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_gpr(region1, region2, length);
		return;
	}

	for (; length & 0xfffffff8; region1+=8, region2+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region2 & 0x0101010101010101)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region2 & 0x0202020202020202)>>1)*p[1];
		r64[2] = ((*(uint64_t *)region2 & 0x0404040404040404)>>2)*p[2];
		r64[3] = ((*(uint64_t *)region2 & 0x0808080808080808)>>3)*p[3];
		r64[4] = ((*(uint64_t *)region2 & 0x1010101010101010)>>4)*p[4];
		r64[5] = ((*(uint64_t *)region2 & 0x2020202020202020)>>5)*p[5];
		r64[6] = ((*(uint64_t *)region2 & 0x4040404040404040)>>6)*p[6];
		r64[7] = ((*(uint64_t *)region2 & 0x8080808080808080)>>7)*p[7];
		*(uint64_t *)region1 ^= r64[0]^r64[1]^r64[2]^r64[3]
					^r64[4]^r64[5]^r64[6]^r64[7];
	}

	for (; length; region1++, region2++, length--) {
		r[0] = ((*region2 & 0x01) >> 0) * p[0];
		r[1] = ((*region2 & 0x02) >> 1) * p[1];
		r[2] = ((*region2 & 0x04) >> 2) * p[2];
		r[3] = ((*region2 & 0x08) >> 3) * p[3];
		r[4] = ((*region2 & 0x10) >> 4) * p[4];
		r[5] = ((*region2 & 0x20) >> 5) * p[5];
		r[6] = ((*region2 & 0x40) >> 6) * p[6];
		r[7] = ((*region2 & 0x80) >> 7) * p[7];
		*region1 ^= r[0]^r[1]^r[2]^r[3]^r[4]^r[5]^r[6]^r[7];
	}
}

void
ffmadd256_region_c_gpr32(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[8];
	uint32_t r32[8];
	
	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_gpr(region1, region2, length);
		return;
	}

	for (; length & 0xfffffffc; region1+=4, region2+=4, length-=4) {
		r32[0] = ((*(uint32_t *)region2 & 0x01010101)>>0)*p[0];
		r32[1] = ((*(uint32_t *)region2 & 0x02020202)>>1)*p[1];
		r32[2] = ((*(uint32_t *)region2 & 0x04040404)>>2)*p[2];
		r32[3] = ((*(uint32_t *)region2 & 0x08080808)>>3)*p[3];
		r32[4] = ((*(uint32_t *)region2 & 0x10101010)>>4)*p[4];
		r32[5] = ((*(uint32_t *)region2 & 0x20202020)>>5)*p[5];
		r32[6] = ((*(uint32_t *)region2 & 0x40404040)>>6)*p[6];
		r32[7] = ((*(uint32_t *)region2 & 0x80808080)>>7)*p[7];
		*(uint32_t *)region1 ^= r32[0]^r32[1]^r32[2]^r32[3]
					^r32[4]^r32[5]^r32[6]^r32[7];
	}

	for (; length; region1++, region2++, length--) {
		r[0] = ((*region2 & 0x01) >> 0) * p[0];
		r[1] = ((*region2 & 0x02) >> 1) * p[1];
		r[2] = ((*region2 & 0x04) >> 2) * p[2];
		r[3] = ((*region2 & 0x08) >> 3) * p[3];
		r[4] = ((*region2 & 0x10) >> 4) * p[4];
		r[5] = ((*region2 & 0x20) >> 5) * p[5];
		r[6] = ((*region2 & 0x40) >> 6) * p[6];
		r[7] = ((*region2 & 0x80) >> 7) * p[7];
		*region1 ^= r[0]^r[1]^r[2]^r[3]^r[4]^r[5]^r[6]^r[7];
	}
}

void
ffmadd256_region_c_log(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	uint8_t l;
	int x;

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_gpr(region1, region2, length);
		return ;
	}
	
	l = logt[constant];

	for (; length; region1++, region2++, length--) {
		if (*region2 == 0)
			continue;
		x = l + logt[*region2];
		*region1 ^= alogt[x];
	}
}

void
ffmadd256_region_c_table(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_gpr(region1, region2, length);
		return ;
	}
	
	for (; length; region1++, region2++, length--) {
		*region1 ^= mult[constant][*region2];
	}
}

void ffmul256_region_c_slow(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[8];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	for (; length; region++, length--) {
		r[0] = (*region &   1) ? p[0] : 0;
		r[1] = (*region &   2) ? p[1] : 0;
		r[2] = (*region &   4) ? p[2] : 0;
		r[3] = (*region &   8) ? p[3] : 0;
		r[4] = (*region &  16) ? p[4] : 0;
		r[5] = (*region &  32) ? p[5] : 0;
		r[6] = (*region &  64) ? p[6] : 0;
		r[7] = (*region & 128) ? p[7] : 0;
		*region = r[0]^r[1]^r[2]^r[3]^r[4]^r[5]^r[6]^r[7];
	}
}

void
ffmul256_region_c_gpr(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[8];
	uint64_t r64[8];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	for (; length & 0xfffffff8; region+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region & 0x0101010101010101)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region & 0x0202020202020202)>>1)*p[1];
		r64[2] = ((*(uint64_t *)region & 0x0404040404040404)>>2)*p[2];
		r64[3] = ((*(uint64_t *)region & 0x0808080808080808)>>3)*p[3];
		r64[4] = ((*(uint64_t *)region & 0x1010101010101010)>>4)*p[4];
		r64[5] = ((*(uint64_t *)region & 0x2020202020202020)>>5)*p[5];
		r64[6] = ((*(uint64_t *)region & 0x4040404040404040)>>6)*p[6];
		r64[7] = ((*(uint64_t *)region & 0x8080808080808080)>>7)*p[7];
		*(uint64_t *)region =
			r64[0]^r64[1]^r64[2]^r64[3]^r64[4]^r64[5]^r64[6]^r64[7];
	}

	for (; length; region++, length--) {
		r[0] = ((*region & 0x01) >> 0) * p[0];
		r[1] = ((*region & 0x02) >> 1) * p[1];
		r[2] = ((*region & 0x04) >> 2) * p[2];
		r[3] = ((*region & 0x08) >> 3) * p[3];
		r[4] = ((*region & 0x10) >> 4) * p[4];
		r[5] = ((*region & 0x20) >> 5) * p[5];
		r[6] = ((*region & 0x40) >> 6) * p[6];
		r[7] = ((*region & 0x80) >> 7) * p[7];
		*region = r[0]^r[1]^r[2]^r[3]^r[4]^r[5]^r[6]^r[7];
	}
}

