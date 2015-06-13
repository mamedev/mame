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

rgbint_t::rgbint_t()
{
	m_value = _mm_setzero_si128();
}

rgbint_t::rgbint_t(UINT32 rgb)
{
	set_rgb(rgb);
}

rgbint_t::rgbint_t(INT16 r, INT16 g, INT16 b)
{
	set_rgb(r, g, b);
}

rgbint_t::rgbint_t(rgb_t& rgb)
{
	m_value = _mm_unpacklo_epi8(_mm_cvtsi32_si128(rgb), _mm_setzero_si128());
}

rgbaint_t::rgbaint_t()
{
	set_rgba(0, 0, 0, 0);
}

rgbaint_t::rgbaint_t(UINT32 argb)
{
	m_value = _mm_set_epi16(0, 0, 0, 0, (argb >> 24) & 0xff, (argb >> 16) & 0xff, (argb >> 8) & 0xff, argb & 0xff);
}

rgbaint_t::rgbaint_t(INT16 a, INT16 r, INT16 g, INT16 b)
{
	set_rgba(a, r, g, b);
}

rgbaint_t::rgbaint_t(rgb_t& rgba)
{
	m_value = _mm_unpacklo_epi8(_mm_cvtsi32_si128(rgba), _mm_setzero_si128());
}

void rgbint_t::set(__m128i value)
{
	m_value = value;
}

void rgbint_t::set_rgb(UINT32 rgb)
{
	m_value = _mm_set_epi16(0, 0, 0, 0, 0, (rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
}

void rgbint_t::set_rgb(rgb_t& rgb)
{
	m_value = _mm_unpacklo_epi8(_mm_cvtsi32_si128(rgb), _mm_setzero_si128());
}

void rgbint_t::set_rgb(INT16 r, INT16 g, INT16 b)
{
	m_value = _mm_set_epi16(0, 0, 0, 0, 0, r, g, b);
}

void rgbaint_t::set_rgba(INT16 a, INT16 r, INT16 g, INT16 b)
{
	m_value = _mm_set_epi16(0, 0, 0, 0, a, r, g, b);
}

/***************************************************************************
    BASIC CONVERSIONS
***************************************************************************/

rgb_t rgbint_t::to_rgb()
{
	return _mm_cvtsi128_si32(_mm_packus_epi16(m_value, m_value));
}

rgb_t rgbint_t::to_rgb_clamp()
{
	return _mm_cvtsi128_si32(_mm_packus_epi16(m_value, m_value));
}

rgb_t rgbint_t::to_rgba()
{
	return to_rgb();
}

rgb_t rgbint_t::to_rgba_clamp()
{
	return to_rgb_clamp();
}

/***************************************************************************
    CORE MATH
***************************************************************************/

void rgbint_t::add(const rgbint_t& color2)
{
	m_value = _mm_add_epi16(m_value, color2.m_value);
}

void rgbint_t::add_imm(const INT16 imm)
{
	__m128i temp = _mm_set_epi16(0, 0, 0, 0, imm, imm, imm, imm);
	m_value = _mm_add_epi16(m_value, temp);
}

void rgbint_t::add_imm_rgb(const INT16 r, const INT16 g, const INT16 b)
{
	__m128i temp = _mm_set_epi16(0, 0, 0, 0, 0, r, g, b);
	m_value = _mm_add_epi16(m_value, temp);
}

void rgbaint_t::add_imm_rgba(const INT16 a, const INT16 r, const INT16 g, const INT16 b)
{
	__m128i temp = _mm_set_epi16(0, 0, 0, 0, a, r, g, b);
	m_value = _mm_add_epi16(m_value, temp);
}

void rgbint_t::sub(const rgbint_t& color2)
{
	m_value = _mm_sub_epi16(m_value, color2.m_value);
}

void rgbint_t::sub_imm(const INT16 imm)
{
	__m128i temp = _mm_set_epi16(0, 0, 0, 0, imm, imm, imm, imm);
	m_value = _mm_sub_epi16(m_value, temp);
}

void rgbint_t::sub_imm_rgb(const INT16 r, const INT16 g, const INT16 b)
{
	__m128i temp = _mm_set_epi16(0, 0, 0, 0, 0, r, g, b);
	m_value = _mm_sub_epi16(m_value, temp);
}

void rgbaint_t::sub_imm_rgba(const INT16 a, const INT16 r, const INT16 g, const INT16 b)
{
	__m128i temp = _mm_set_epi16(0, 0, 0, 0, a, r, g, b);
	m_value = _mm_sub_epi16(m_value, temp);
}

void rgbint_t::subr(rgbint_t& color2)
{
	color2.m_value = _mm_sub_epi16(color2.m_value, m_value);
}

void rgbint_t::subr_imm(const INT16 imm)
{
	__m128i temp = _mm_set_epi16(0, 0, 0, 0, imm, imm, imm, imm);
	m_value = _mm_sub_epi16(temp, m_value);
}

void rgbint_t::subr_imm_rgb(const INT16 r, const INT16 g, const INT16 b)
{
	__m128i temp = _mm_set_epi16(0, 0, 0, 0, 0, r, g, b);
	m_value = _mm_sub_epi16(temp, m_value);
}

void rgbaint_t::subr_imm_rgba(const INT16 a, const INT16 r, const INT16 g, const INT16 b)
{
	__m128i temp = _mm_set_epi16(0, 0, 0, 0, a, r, g, b);
	m_value = _mm_sub_epi16(temp, m_value);
}

void rgbint_t::shl(UINT8 shift)
{
	m_value = _mm_slli_epi16(m_value, shift);
}

void rgbint_t::shr(UINT8 shift)
{
	m_value = _mm_srli_epi16(m_value, shift);
}


/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

void rgbint_t::blend(const rgbint_t& other, UINT8 factor)
{
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, *(__m128i *)&rgbsse_statics.scale_table[factor][0]);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_packs_epi32(m_value, m_value);
}

void rgbint_t::scale_and_clamp(const rgbint_t& scale)
{
	__m128i mscale = _mm_unpacklo_epi16(scale.m_value, _mm_setzero_si128());
	m_value = _mm_unpacklo_epi16(m_value, _mm_setzero_si128());
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_packs_epi32(m_value, m_value);
	m_value = _mm_min_epi16(m_value, *(__m128i *)&rgbsse_statics.maxbyte);
}

void rgbint_t::scale_imm_and_clamp(const INT16 scale)
{
	__m128i mscale = _mm_set1_epi16(scale);
	m_value = _mm_unpacklo_epi16(m_value, _mm_setzero_si128());
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_packs_epi32(m_value, m_value);
	m_value = _mm_min_epi16(m_value, *(__m128i *)&rgbsse_statics.maxbyte);
}

void rgbint_t::scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other, const rgbint_t& scale2)
{
	__m128i mscale = _mm_unpacklo_epi16(scale.m_value, scale2.m_value);
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_packs_epi32(m_value, m_value);
	m_value = _mm_min_epi16(m_value, *(__m128i *)&rgbsse_statics.maxbyte);
}

void rgbint_t::scale_imm_add_and_clamp(const INT16 scale, const rgbint_t& other)
{
	// color2 will get mutiplied by 2^8 (256) and then divided by 2^8 by the shift by 8
	__m128i mscale = _mm_unpacklo_epi16(_mm_set1_epi16(scale), _mm_set_epi16(0, 0, 0, 0, 256, 256, 256, 256));
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_packs_epi32(m_value, m_value);
	m_value = _mm_min_epi16(m_value, *(__m128i *)&rgbsse_statics.maxbyte);
}

void rgbint_t::scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other)
{
	// color2 will get mutiplied by 2^8 (256) and then divided by 2^8 by the shift by 8
	__m128i mscale = _mm_unpacklo_epi16(scale.m_value, _mm_set_epi16(0, 0, 0, 0, 256, 256, 256, 256));
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, mscale);
	m_value = _mm_srli_epi32(m_value, 8);
	m_value = _mm_packs_epi32(m_value, m_value);
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
