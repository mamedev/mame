// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbsse.h

    SSE optimized RGB utilities.

    WARNING: This code assumes SSE2 or greater capability.

***************************************************************************/

#ifndef __RGBSSE__
#define __RGBSSE__

#include <emmintrin.h>

/***************************************************************************
    TABLES
***************************************************************************/

extern const struct _rgbsse_statics
{
	__m128  dummy_for_alignment;
	INT16   maxbyte[8];
	INT16   alpha_mask[8];
	INT16   red_mask[8];
	INT16   green_mask[8];
	INT16   blue_mask[8];
	INT16   scale_table[256][8];
} rgbsse_statics;

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class rgbaint_t
{
public:
	inline rgbaint_t() { }
	inline rgbaint_t(UINT32 rgba) { set(rgba); }
	inline rgbaint_t(UINT32 a, UINT32 r, UINT32 g, UINT32 b) { set(a, r, g, b); }
	inline rgbaint_t(rgb_t& rgb) { set(rgb); }

	inline void set(rgbaint_t& other) { m_value = other.m_value; }
	inline void set(UINT32 rgba) { m_value = _mm_and_si128(_mm_set1_epi32(0xff), _mm_set_epi32(rgba >> 24, rgba >> 16, rgba >> 8, rgba)); }
	inline void set(UINT32 a, UINT32 r, UINT32 g, UINT32 b) { m_value = _mm_set_epi32(a, r, g, b); }
	inline void set(rgb_t& rgb) { m_value = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(rgb), _mm_setzero_si128()), _mm_setzero_si128()); }

	inline rgb_t to_rgba()
	{
		return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(m_value, _mm_setzero_si128()), _mm_setzero_si128()));
	}

	inline UINT32 to_argb8()
	{
		return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(m_value, _mm_setzero_si128()), _mm_setzero_si128()));
	}

	inline rgb_t to_rgba_clamp()
	{
		return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(m_value, m_value), _mm_setzero_si128()));
	}

	inline void add(const rgbaint_t& color2)
	{
		m_value = _mm_add_epi32(m_value, color2.m_value);
	}

	inline void add_imm(const UINT32 imm)
	{
		m_value = _mm_add_epi32(m_value, _mm_set1_epi32(imm));
	}

	inline void add_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		m_value = _mm_add_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void sub(const rgbaint_t& color2)
	{
		m_value = _mm_sub_epi32(m_value, color2.m_value);
	}

	inline void sub_imm(const UINT32 imm)
	{
		m_value = _mm_sub_epi32(m_value, _mm_set1_epi32(imm));
	}

	inline void sub_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		m_value = _mm_sub_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void subr(rgbaint_t& color2)
	{
		m_value = _mm_sub_epi32(color2.m_value, m_value);
	}

	inline void subr_imm(const UINT32 imm)
	{
		m_value = _mm_sub_epi32(_mm_set1_epi32(imm), m_value);
	}

	inline void subr_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		m_value = _mm_sub_epi32(_mm_set_epi32(a, r, g, b), m_value);
	}

	inline void set_a(const UINT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, *(__m128i *)&rgbsse_statics.alpha_mask), _mm_set_epi32(value, 0, 0, 0));
	}

	inline void set_r(const UINT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, *(__m128i *)&rgbsse_statics.red_mask), _mm_set_epi32(0, value, 0, 0));
	}

	inline void set_g(const UINT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, *(__m128i *)&rgbsse_statics.green_mask), _mm_set_epi32(0, 0, value, 0));
	}

	inline void set_b(const UINT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, *(__m128i *)&rgbsse_statics.blue_mask), _mm_set_epi32(0, 0, 0, value));
	}

	inline UINT8 get_a()
	{
		return _mm_extract_epi16(m_value, 6);
	}

	inline UINT8 get_r()
	{
		return _mm_extract_epi16(m_value, 4);
	}

	inline UINT8 get_g()
	{
		return _mm_extract_epi16(m_value, 2);
	}

	inline UINT8 get_b()
	{
		return _mm_extract_epi16(m_value, 0);
	}

	inline UINT32 get_a32()
	{
		return (_mm_extract_epi16(m_value, 7) << 16) | _mm_extract_epi16(m_value, 6);
	}

	inline UINT32 get_r32()
	{
		return (_mm_extract_epi16(m_value, 5) << 16) | _mm_extract_epi16(m_value, 4);
	}

	inline UINT32 get_g32()
	{
		return (_mm_extract_epi16(m_value, 3) << 16) | _mm_extract_epi16(m_value, 2);
	}

	inline UINT32 get_b32()
	{
		return (_mm_extract_epi16(m_value, 1) << 16) | _mm_extract_epi16(m_value, 0);
	}

	inline void mul(const rgbaint_t& color)
	{
		__m128i tmp1 = _mm_mul_epu32(m_value, color.m_value);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(color.m_value, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void mul_imm(const UINT32 imm)
	{
		__m128i immv = _mm_set1_epi32(imm);
		__m128i tmp1 = _mm_mul_epu32(m_value, immv);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void mul_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		__m128i immv = _mm_set_epi32(a, r, g, b);
		__m128i tmp1 = _mm_mul_epu32(m_value, immv);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void shl(const rgbaint_t& shift)
	{
		m_value = _mm_sll_epi32(m_value, shift.m_value);
	}

	inline void shl_imm(const UINT8 shift)
	{
		m_value = _mm_slli_epi32(m_value, shift);
	}

	inline void shl_imm_all(const UINT8 shift)
	{
		m_value = _mm_slli_si128(m_value, shift >> 3);
	}

	inline void shr(const rgbaint_t& shift)
	{
		m_value = _mm_srl_epi32(m_value, shift.m_value);
	}

	inline void shr_imm(const UINT8 shift)
	{
		m_value = _mm_srli_epi32(m_value, shift);
	}

	inline void shr_imm_all(const UINT8 shift)
	{
		m_value = _mm_srli_si128(m_value, shift >> 3);
	}

	inline void sra(const rgbaint_t& shift)
	{
		m_value = _mm_sra_epi32(m_value, shift.m_value);
	}

	inline void sra_imm(const UINT8 shift)
	{
		m_value = _mm_srai_epi32(m_value, shift);
	}

	inline void or_reg(const rgbaint_t& color2)
	{
		m_value = _mm_or_si128(m_value, color2.m_value);
	}

	inline void or_imm(const UINT32 value)
	{
		m_value = _mm_or_si128(m_value, _mm_set1_epi32(value));
	}

	inline void or_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		m_value = _mm_or_si128(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void and_reg(const rgbaint_t& color)
	{
		m_value = _mm_and_si128(m_value, color.m_value);
	}

	inline void and_imm(const UINT32 value)
	{
		m_value = _mm_and_si128(m_value, _mm_set1_epi32(value));
	}

	inline void and_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		m_value = _mm_and_si128(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void xor_reg(const rgbaint_t& color2)
	{
		m_value = _mm_xor_si128(m_value, color2.m_value);
	}

	inline void xor_imm(const INT32 value)
	{
		m_value = _mm_xor_si128(m_value, _mm_set1_epi32(value));
	}

	inline void xor_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		m_value = _mm_xor_si128(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void clamp_and_clear(const UINT32 sign)
	{
		__m128i vsign = _mm_set1_epi32(sign);
		m_value = _mm_and_si128(m_value, _mm_cmpeq_epi32(_mm_and_si128(m_value, vsign), _mm_setzero_si128()));
		vsign = _mm_srai_epi32(vsign, 1);
		vsign = _mm_xor_si128(vsign, _mm_set1_epi32(0xffffffff));
		__m128i mask = _mm_cmpgt_epi32(m_value, vsign);
		m_value = _mm_or_si128(_mm_and_si128(vsign, mask), _mm_and_si128(m_value, _mm_xor_si128(mask, _mm_set1_epi32(0xffffffff))));
	}

	inline void sign_extend(const UINT32 compare, const UINT32 sign)
	{
		__m128i compare_vec = _mm_set1_epi32(compare);
		__m128i compare_mask = _mm_cmpeq_epi32(_mm_and_si128(m_value, compare_vec), compare_vec);
		__m128i compared = _mm_and_si128(_mm_set1_epi32(sign), compare_mask);
		m_value = _mm_or_si128(m_value, compared);
	}

	inline void min(const UINT32 value)
	{
		__m128i val = _mm_set1_epi32(value);
		__m128i mask = _mm_cmpgt_epi32(m_value, val);
		m_value = _mm_or_si128(_mm_and_si128(val, mask), _mm_and_si128(m_value, _mm_xor_si128(mask, _mm_set1_epi32(0xffffffff))));
	}

	inline void max(const UINT32 value)
	{
		__m128i val = _mm_set1_epi32(value);
		__m128i mask = _mm_cmplt_epi32(m_value, val);
		m_value = _mm_or_si128(_mm_and_si128(val, mask), _mm_and_si128(m_value, _mm_xor_si128(mask, _mm_set1_epi32(0xffffffff))));
	}

	void blend(const rgbaint_t& other, UINT8 factor);

	void scale_and_clamp(const rgbaint_t& scale);
	void scale_imm_and_clamp(const INT32 scale);
	void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2);
	void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other);
	void scale_imm_add_and_clamp(const INT32 scale, const rgbaint_t& other);

	inline void cmpeq(const rgbaint_t& value)
	{
		m_value = _mm_cmpeq_epi32(m_value, value.m_value);
	}

	inline void cmpeq_imm(const UINT32 value)
	{
		m_value = _mm_cmpeq_epi32(m_value, _mm_set1_epi32(value));
	}

	inline void cmpeq_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		m_value = _mm_cmpeq_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void cmpgt(const rgbaint_t& value)
	{
		m_value = _mm_cmpgt_epi32(m_value, value.m_value);
	}

	inline void cmpgt_imm(const UINT32 value)
	{
		m_value = _mm_cmpgt_epi32(m_value, _mm_set1_epi32(value));
	}

	inline void cmpgt_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		m_value = _mm_cmpgt_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void cmplt(const rgbaint_t& value)
	{
		m_value = _mm_cmplt_epi32(m_value, value.m_value);
	}

	inline void cmplt_imm(const UINT32 value)
	{
		m_value = _mm_cmplt_epi32(m_value, _mm_set1_epi32(value));
	}

	inline void cmplt_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		m_value = _mm_cmplt_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline rgbaint_t operator=(const rgbaint_t& other)
	{
		m_value = other.m_value;
		return *this;
	}

	inline rgbaint_t& operator+=(const rgbaint_t& other)
	{
		m_value = _mm_add_epi32(m_value, other.m_value);
		return *this;
	}

	inline rgbaint_t& operator+=(const INT32 other)
	{
		m_value = _mm_add_epi32(m_value, _mm_set1_epi32(other));
		return *this;
	}

	inline rgbaint_t& operator-=(const rgbaint_t& other)
	{
		m_value = _mm_sub_epi32(m_value, other.m_value);
		return *this;
	}

	inline rgbaint_t& operator*=(const rgbaint_t& other)
	{
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(_mm_mul_epu32(m_value, other.m_value), _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(other.m_value, 4)), _MM_SHUFFLE(0, 0, 2, 0)));
		return *this;
	}

	inline rgbaint_t& operator*=(const INT32 other)
	{
		const __m128i immv = _mm_set1_epi32(other);
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(_mm_mul_epu32(m_value, immv), _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4)), _MM_SHUFFLE(0, 0, 2, 0)));
		return *this;
	}

	inline rgbaint_t& operator>>=(const INT32 shift)
	{
		m_value = _mm_srai_epi32(m_value, shift);
		return *this;
	}

	inline void merge_alpha(const rgbaint_t& alpha)
	{
		m_value = _mm_insert_epi16(m_value, _mm_extract_epi16(alpha.m_value, 7), 7);
		m_value = _mm_insert_epi16(m_value, _mm_extract_epi16(alpha.m_value, 6), 6);
	}

	static UINT32 bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
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
		color01 = _mm_packs_epi32(color01, _mm_setzero_si128());
		color01 = _mm_packus_epi16(color01, _mm_setzero_si128());
		return _mm_cvtsi128_si32(color01);
	}

protected:
	__m128i m_value;
};

#endif /* __RGBSSE__ */
