// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbsse.c

    SSE optimized RGB utilities.

    WARNING: This code assumes SSE2 or greater capability.

***************************************************************************/

#include "emu.h"
#include <emmintrin.h>
#include "rgbutil.h"

/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

void rgbaint_t::blend(const rgbaint_t& other, UINT8 factor)
{
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, *(__m128i *)&rgbsse_statics.scale_table[factor][0]);
	m_value = _mm_srli_epi32(m_value, 8);
}

void rgbaint_t::scale_and_clamp(const rgbaint_t& scale)
{
	mul(scale);
	shr(8);
	min(255);
}

void rgbaint_t::scale_imm_and_clamp(const INT32 scale)
{
	mul_imm(scale);
	shr(8);
	min(255);
}

void rgbaint_t::scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2)
{
	mul(scale);
	rgbaint_t color2(other);
	color2.mul(scale2);

	mul(scale);
	add(color2);
	shr(8);
	min(255);
}

void rgbaint_t::scale_imm_add_and_clamp(const INT32 scale, const rgbaint_t& other)
{
	mul_imm(scale);
	add(other);
	shr(8);
	min(255);
}

void rgbaint_t::scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other)
{
	mul(scale);
	add(other);
	shr(8);
	min(255);
}

UINT32 rgbaint_t::bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	__m128i color00 = _mm_cvtsi32_si128(rgb00);
	__m128i color01 = _mm_cvtsi32_si128(rgb01);
	__m128i color10 = _mm_cvtsi32_si128(rgb10);
	__m128i color11 = _mm_cvtsi32_si128(rgb11);

	/* interleave color01 and color00 at the byte level */
	color01 = _mm_unpacklo_epi8(color01, color00);
	color11 = _mm_unpacklo_epi8(color11, color10);
	color01 = _mm_unpacklo_epi8(color01, _mm_setzero_si128());
	color11 = _mm_unpacklo_epi8(color11, _mm_setzero_si128());
	color01 = _mm_madd_epi16(color01, *(__m128i *)&rgbsse_statics.scale_table[u][0]);
	color11 = _mm_madd_epi16(color11, *(__m128i *)&rgbsse_statics.scale_table[u][0]);
	color01 = _mm_slli_epi32(color01, 15);
	color11 = _mm_srli_epi32(color11, 1);
	color01 = _mm_max_epi16(color01, color11);
	color01 = _mm_madd_epi16(color01, *(__m128i *)&rgbsse_statics.scale_table[v][0]);
	color01 = _mm_srli_epi32(color01, 15);
	color01 = _mm_packs_epi32(color01, color01);
	color01 = _mm_packus_epi16(color01, color01);
	return _mm_cvtsi128_si32(color01);
}
