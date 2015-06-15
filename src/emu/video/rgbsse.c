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

rgbint_t::rgbint_t(INT32 r, INT32 g, INT32 b)
{
	set_rgb(r, g, b);
}

rgbint_t::rgbint_t(rgb_t& rgb)
{
	m_value = _mm_unpacklo_epi8(_mm_cvtsi32_si128(rgb), _mm_setzero_si128());
}

rgbint_t::rgbint_t(__m128i value)
{
	m_value = value;
}

rgbaint_t::rgbaint_t()
{
	set_rgba(0, 0, 0, 0);
}

rgbaint_t::rgbaint_t(UINT32 argb)
{
	m_value = _mm_set_epi32((argb >> 24) & 0xff, (argb >> 16) & 0xff, (argb >> 8) & 0xff, argb & 0xff);
}

rgbaint_t::rgbaint_t(INT32 a, INT32 r, INT32 g, INT32 b)
{
	set_rgba(a, r, g, b);
}

rgbaint_t::rgbaint_t(rgb_t& rgba)
{
	m_value = _mm_unpacklo_epi8(_mm_cvtsi32_si128(rgba), _mm_setzero_si128());
}

void rgbint_t::set(void* value)
{
	m_value = *(__m128i*)value;
}

void rgbint_t::set(__m128i value)
{
	m_value = value;
}

__m128i rgbint_t::get()
{
	return m_value;
}

void rgbint_t::set_rgb(UINT32 rgb)
{
	m_value = _mm_set_epi32(0, rgb & 0xff, (rgb >> 8) & 0xff, (rgb >> 16) & 0xff);
}

void rgbint_t::set_rgb(rgb_t& rgb)
{
	m_value = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(rgb), _mm_setzero_si128()), _mm_setzero_si128());
}

void rgbint_t::set_rgb(INT32 r, INT32 g, INT32 b)
{
	m_value = _mm_set_epi32(0, r, g, b);
}

void rgbaint_t::set_rgba(INT32 a, INT32 r, INT32 g, INT32 b)
{
	m_value = _mm_set_epi32(a, r, g, b);
}

/***************************************************************************
    OPERATORS
***************************************************************************/

rgbint_t rgbint_t::operator=(const rgbint_t& other)
{
	m_value = other.m_value;
	return *this;
}

rgbint_t& rgbint_t::operator+=(const rgbint_t& other)
{
	m_value = _mm_add_epi32(m_value, other.m_value);
	return *this;
}

rgbint_t& rgbint_t::operator-=(const rgbint_t& other)
{
	m_value = _mm_sub_epi32(m_value, other.m_value);
	return *this;
}

rgbint_t& rgbint_t::operator*=(const rgbint_t& other)
{
	m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(_mm_mul_epu32(m_value, other.m_value), _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(other.m_value, 4)), _MM_SHUFFLE(0, 0, 2, 0)));
	return *this;
}

rgbint_t& rgbint_t::operator*=(const INT32 other)
{
	const __m128i immv = _mm_set1_epi32(other);
	m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(_mm_mul_epu32(m_value, immv), _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4)), _MM_SHUFFLE(0, 0, 2, 0)));
	return *this;
}

rgbint_t rgbint_t::operator+(const rgbint_t& other)
{
	return _mm_add_epi32(m_value, other.m_value);
}

rgbint_t rgbint_t::operator-(const rgbint_t& other)
{
	return _mm_sub_epi32(m_value, other.m_value);
}

rgbint_t rgbint_t::operator*(const rgbint_t& other)
{
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(_mm_mul_epu32(m_value, other.m_value), _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(other.m_value, 4)), _MM_SHUFFLE(0, 0, 2, 0)));
}

rgbint_t rgbint_t::operator*(const INT32 other)
{
	const __m128i immv = _mm_set1_epi32(other);
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(_mm_mul_epu32(m_value, immv), _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4)), _MM_SHUFFLE(0, 0, 2, 0)));
}

/***************************************************************************
    BASIC CONVERSIONS
***************************************************************************/

rgb_t rgbint_t::to_rgb()
{
	__m128i anded = _mm_and_si128(m_value, _mm_set_epi32(0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff));
	return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(anded, anded), _mm_setzero_si128()));
}

rgb_t rgbint_t::to_rgb_clamp()
{
	return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(m_value, m_value), _mm_setzero_si128()));
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
	m_value = _mm_add_epi32(m_value, color2.m_value);
}

void rgbint_t::add_imm(const INT32 imm)
{
	__m128i temp = _mm_set_epi32(imm, imm, imm, imm);
	m_value = _mm_add_epi32(m_value, temp);
}

void rgbint_t::add_imm_rgb(const INT32 r, const INT32 g, const INT32 b)
{
	__m128i temp = _mm_set_epi32(0, r, g, b);
	m_value = _mm_add_epi32(m_value, temp);
}

void rgbaint_t::add_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
{
	__m128i temp = _mm_set_epi32(a, r, g, b);
	m_value = _mm_add_epi32(m_value, temp);
}

void rgbint_t::sub(const rgbint_t& color2)
{
	m_value = _mm_sub_epi32(m_value, color2.m_value);
}

void rgbint_t::sub_imm(const INT32 imm)
{
	__m128i temp = _mm_set_epi32(imm, imm, imm, imm);
	m_value = _mm_sub_epi32(m_value, temp);
}

void rgbint_t::sub_imm_rgb(const INT32 r, const INT32 g, const INT32 b)
{
	__m128i temp = _mm_set_epi32(0, r, g, b);
	m_value = _mm_sub_epi32(m_value, temp);
}

void rgbaint_t::sub_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
{
	__m128i temp = _mm_set_epi32(a, r, g, b);
	m_value = _mm_sub_epi32(m_value, temp);
}

void rgbint_t::subr(rgbint_t& color2)
{
	color2.m_value = _mm_sub_epi32(color2.m_value, m_value);
}

void rgbint_t::subr_imm(const INT32 imm)
{
	__m128i temp = _mm_set_epi32(imm, imm, imm, imm);
	m_value = _mm_sub_epi32(temp, m_value);
}

void rgbint_t::subr_imm_rgb(const INT32 r, const INT32 g, const INT32 b)
{
	__m128i temp = _mm_set_epi32(0, r, g, b);
	m_value = _mm_sub_epi32(temp, m_value);
}

void rgbaint_t::subr_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
{
	__m128i temp = _mm_set_epi32(a, r, g, b);
	m_value = _mm_sub_epi32(temp, m_value);
}

void rgbint_t::mul(rgbint_t& color)
{
	__m128i tmp1 = _mm_mul_epu32(m_value, color.m_value);
	__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(color.m_value, 4));
	m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
}

void rgbint_t::print()
{
	printf("%04x ", _mm_extract_epi16(m_value, 0));
	printf("%04x ", _mm_extract_epi16(m_value, 1));
	printf("%04x ", _mm_extract_epi16(m_value, 2));
	printf("%04x ", _mm_extract_epi16(m_value, 3));
	printf("%04x ", _mm_extract_epi16(m_value, 4));
	printf("%04x ", _mm_extract_epi16(m_value, 5));
	printf("%04x ", _mm_extract_epi16(m_value, 6));
	printf("%04x\n", _mm_extract_epi16(m_value, 7));
}

void rgbint_t::mul_imm(const INT32 imm)
{
	__m128i immv = _mm_set1_epi32(imm);
	__m128i tmp1 = _mm_mul_epu32(m_value, immv);
	__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
	m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
}

void rgbint_t::mul_imm_rgb(const INT32 r, const INT32 g, const INT32 b)
{
	__m128i immv = _mm_set_epi32(0, r, g, b);
	__m128i tmp1 = _mm_mul_epu32(m_value, immv);
	__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
	m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
}

void rgbaint_t::mul_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
{
	__m128i immv = _mm_set_epi32(a, r, g, b);
	__m128i tmp1 = _mm_mul_epu32(m_value, immv);
	__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
	m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
}

void rgbint_t::shl(UINT8 shift)
{
	m_value = _mm_slli_epi32(m_value, shift);
}

void rgbint_t::shr(UINT8 shift)
{
	m_value = _mm_srli_epi32(m_value, shift);
}

void rgbint_t::sra(UINT8 shift)
{
	m_value = _mm_srai_epi32(m_value, shift);
}

void rgbint_t::sign_extend(const INT32 compare, const INT32 sign)
{
	__m128i compare_vec = _mm_set1_epi32(compare);
	__m128i compare_mask = _mm_cmpeq_epi32(_mm_and_si128(m_value, compare_vec), compare_vec);
	m_value = _mm_or_si128(m_value, _mm_and_si128(_mm_set1_epi32(sign), compare_mask));
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
