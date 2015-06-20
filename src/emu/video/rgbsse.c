// license:BSD-3-Clause
// copyright-holders:Vas Crabb,Ryan Holtz
/***************************************************************************

    rgbsse.c

    SSE optimized RGB utilities.

    WARNING: This code assumes SSE2 or greater capability.

***************************************************************************/

#include "emu.h"
#include <emmintrin.h>
#include "rgbutil.h"

void rgbint_t::print()
{
	printf("%04x ", _mm_extract_epi16(m_value, 7));
	printf("%04x ", _mm_extract_epi16(m_value, 6));
	printf("%04x ", _mm_extract_epi16(m_value, 5));
	printf("%04x ", _mm_extract_epi16(m_value, 4));
	printf("%04x ", _mm_extract_epi16(m_value, 3));
	printf("%04x ", _mm_extract_epi16(m_value, 2));
	printf("%04x ", _mm_extract_epi16(m_value, 1));
	printf("%04x\n", _mm_extract_epi16(m_value, 0));
}

/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

void rgbint_t::blend(const rgbint_t& other, UINT8 factor)
{
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, *(__m128i *)&rgbsse_statics.scale_table[factor][0]);
	m_value = _mm_srli_epi32(m_value, 8);
}

void rgbint_t::scale_and_clamp(const rgbint_t& scale)
{
	__m128i mscale = _mm_unpacklo_epi16(scale.m_value, _mm_setzero_si128());
	m_value = _mm_unpacklo_epi16(m_value, _mm_setzero_si128());
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_min_epi16(m_value, *(__m128i *)&rgbsse_statics.maxbyte);
}

void rgbint_t::scale_imm_and_clamp(const INT16 scale)
{
	__m128i mscale = _mm_set1_epi16(scale);
	m_value = _mm_unpacklo_epi16(m_value, _mm_setzero_si128());
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_min_epi16(m_value, *(__m128i *)&rgbsse_statics.maxbyte);
}

void rgbint_t::scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other, const rgbint_t& scale2)
{
	__m128i mscale = _mm_unpacklo_epi16(scale.m_value, scale2.m_value);
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_min_epi16(m_value, *(__m128i *)&rgbsse_statics.maxbyte);
}

void rgbint_t::scale_imm_add_and_clamp(const INT16 scale, const rgbint_t& other)
{
	// color2 will get mutiplied by 2^8 (256) and then divided by 2^8 by the shift by 8
	__m128i mscale = _mm_unpacklo_epi16(_mm_set1_epi16(scale), _mm_set_epi16(0, 0, 0, 0, 256, 256, 256, 256));
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_min_epi16(m_value, *(__m128i *)&rgbsse_statics.maxbyte);
}

void rgbint_t::scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other)
{
	// color2 will get mutiplied by 2^8 (256) and then divided by 2^8 by the shift by 8
	__m128i mscale = _mm_unpacklo_epi16(scale.m_value, _mm_set_epi16(0, 0, 0, 0, 256, 256, 256, 256));
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_min_epi16(m_value, *(__m128i *)&rgbsse_statics.maxbyte);
}

UINT32 rgbint_t::bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
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
