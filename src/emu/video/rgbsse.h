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
    TYPE DEFINITIONS
***************************************************************************/

class rgbaint_t
{
public:
	inline rgbaint_t() { }
	inline rgbaint_t(UINT32 rgba) { set(rgba); }
	inline rgbaint_t(INT32 a, INT32 r, INT32 g, INT32 b) { set(a, r, g, b); }
	inline rgbaint_t(rgb_t& rgb) { set(rgb); }
	inline rgbaint_t(__m128i rgba) { m_value = rgba; }

	inline void set(rgbaint_t& other) { m_value = other.m_value; }
	inline void set(UINT32 rgba) { m_value = _mm_and_si128(_mm_set1_epi32(0xff), _mm_set_epi32(rgba >> 24, rgba >> 16, rgba >> 8, rgba)); }
	inline void set(INT32 a, INT32 r, INT32 g, INT32 b) { m_value = _mm_set_epi32(a, r, g, b); }
	inline void set(rgb_t& rgb) { m_value = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(rgb), _mm_setzero_si128()), _mm_setzero_si128()); }

	inline rgb_t to_rgba()
	{
		return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(m_value, _mm_setzero_si128()), _mm_setzero_si128()));
	}

	inline rgb_t to_rgba_clamp()
	{
		return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(m_value, _mm_setzero_si128()), _mm_setzero_si128()));
	}

	inline void add(const rgbaint_t& color2)
	{
		m_value = _mm_add_epi32(m_value, color2.m_value);
	}

	inline void add_imm(const INT32 imm)
	{
		m_value = _mm_add_epi32(m_value, _mm_set1_epi32(imm));
	}

	inline void add_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_value = _mm_add_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void sub(const rgbaint_t& color2)
	{
		m_value = _mm_sub_epi32(m_value, color2.m_value);
	}

	inline void sub_imm(const INT32 imm)
	{
		m_value = _mm_sub_epi32(m_value, _mm_set1_epi32(imm));
	}

	inline void sub_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_value = _mm_sub_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void subr(rgbaint_t& color2)
	{
		m_value = _mm_sub_epi32(color2.m_value, m_value);
	}

	inline void subr_imm(const INT32 imm)
	{
		m_value = _mm_sub_epi32(_mm_set1_epi32(imm), m_value);
	}

	inline void subr_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_value = _mm_sub_epi32(_mm_set_epi32(a, r, g, b), m_value);
	}

	inline void set_a(const INT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, alpha_mask()), _mm_set_epi32(value, 0, 0, 0));
	}

	inline void set_r(const INT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, red_mask()), _mm_set_epi32(0, value, 0, 0));
	}

	inline void set_g(const INT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, green_mask()), _mm_set_epi32(0, 0, value, 0));
	}

	inline void set_b(const INT32 value)
	{
		m_value = _mm_or_si128(_mm_and_si128(m_value, blue_mask()), _mm_set_epi32(0, 0, 0, value));
	}

	inline UINT8 get_a() const
	{
		return _mm_extract_epi16(m_value, 6);
	}

	inline UINT8 get_r() const
	{
		return _mm_extract_epi16(m_value, 4);
	}

	inline UINT8 get_g() const
	{
		return _mm_extract_epi16(m_value, 2);
	}

	inline UINT8 get_b() const
	{
		return _mm_extract_epi16(m_value, 0);
	}

	inline INT32 get_a32() const
	{
		return (_mm_extract_epi16(m_value, 7) << 16) | _mm_extract_epi16(m_value, 6);
	}

	inline INT32 get_r32() const
	{
		return (_mm_extract_epi16(m_value, 5) << 16) | _mm_extract_epi16(m_value, 4);
	}

	inline INT32 get_g32() const
	{
		return (_mm_extract_epi16(m_value, 3) << 16) | _mm_extract_epi16(m_value, 2);
	}

	inline INT32 get_b32() const
	{
		return (_mm_extract_epi16(m_value, 1) << 16) | _mm_extract_epi16(m_value, 0);
	}

	inline void mul(const rgbaint_t& color)
	{
		__m128i tmp1 = _mm_mul_epu32(m_value, color.m_value);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(color.m_value, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void mul_imm(const INT32 imm)
	{
		__m128i immv = _mm_set1_epi32(imm);
		__m128i tmp1 = _mm_mul_epu32(m_value, immv);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void mul_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		__m128i immv = _mm_set_epi32(a, r, g, b);
		__m128i tmp1 = _mm_mul_epu32(m_value, immv);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void shl(const rgbaint_t& shift)
	{
		rgbaint_t areg(*this);
		rgbaint_t rreg(*this);
		rgbaint_t greg(*this);
		rgbaint_t breg(*this);
		rgbaint_t ashift(0, 0, 0, shift.get_a32());
		rgbaint_t rshift(0, 0, 0, shift.get_r32());
		rgbaint_t gshift(0, 0, 0, shift.get_g32());
		rgbaint_t bshift(0, 0, 0, shift.get_b32());
		areg.m_value = _mm_sll_epi32(areg.m_value, ashift.m_value);
		rreg.m_value = _mm_sll_epi32(rreg.m_value, rshift.m_value);
		greg.m_value = _mm_sll_epi32(greg.m_value, gshift.m_value);
		breg.m_value = _mm_sll_epi32(breg.m_value, bshift.m_value);
		set(areg.get_a32(), rreg.get_r32(), greg.get_g32(), breg.get_b32());
	}

	inline void shl_imm(const UINT8 shift)
	{
		m_value = _mm_slli_epi32(m_value, shift);
	}

	inline void shr(const rgbaint_t& shift)
	{
		rgbaint_t areg(*this);
		rgbaint_t rreg(*this);
		rgbaint_t greg(*this);
		rgbaint_t breg(*this);
		rgbaint_t ashift(0, 0, 0, shift.get_a32());
		rgbaint_t rshift(0, 0, 0, shift.get_r32());
		rgbaint_t gshift(0, 0, 0, shift.get_g32());
		rgbaint_t bshift(0, 0, 0, shift.get_b32());
		areg.m_value = _mm_srl_epi32(areg.m_value, ashift.m_value);
		rreg.m_value = _mm_srl_epi32(rreg.m_value, rshift.m_value);
		greg.m_value = _mm_srl_epi32(greg.m_value, gshift.m_value);
		breg.m_value = _mm_srl_epi32(breg.m_value, bshift.m_value);
		set(areg.get_a32(), rreg.get_r32(), greg.get_g32(), breg.get_b32());
	}

	inline void shr_imm(const UINT8 shift)
	{
		m_value = _mm_srli_epi32(m_value, shift);
	}

	inline void sra(const rgbaint_t& shift)
	{
		rgbaint_t areg(*this);
		rgbaint_t rreg(*this);
		rgbaint_t greg(*this);
		rgbaint_t breg(*this);
		rgbaint_t ashift(0, 0, 0, shift.get_a32());
		rgbaint_t rshift(0, 0, 0, shift.get_r32());
		rgbaint_t gshift(0, 0, 0, shift.get_g32());
		rgbaint_t bshift(0, 0, 0, shift.get_b32());
		areg.m_value = _mm_sra_epi32(areg.m_value, ashift.m_value);
		rreg.m_value = _mm_sra_epi32(rreg.m_value, rshift.m_value);
		greg.m_value = _mm_sra_epi32(greg.m_value, gshift.m_value);
		breg.m_value = _mm_sra_epi32(breg.m_value, bshift.m_value);
		set(areg.get_a32(), rreg.get_r32(), greg.get_g32(), breg.get_b32());
	}

	inline void sra_imm(const UINT8 shift)
	{
		m_value = _mm_srai_epi32(m_value, shift);
	}

	inline void or_reg(const rgbaint_t& color2)
	{
		m_value = _mm_or_si128(m_value, color2.m_value);
	}

	inline void or_imm(const INT32 value)
	{
		m_value = _mm_or_si128(m_value, _mm_set1_epi32(value));
	}

	inline void or_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_value = _mm_or_si128(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void and_reg(const rgbaint_t& color)
	{
		m_value = _mm_and_si128(m_value, color.m_value);
	}

	inline void andnot_reg(const rgbaint_t& color)
	{
		m_value = _mm_andnot_si128(color.m_value, m_value);
	}

	inline void and_imm(const INT32 value)
	{
		m_value = _mm_and_si128(m_value, _mm_set1_epi32(value));
	}

	inline void and_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
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

	inline void xor_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
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

	inline void clamp_to_uint8()
	{
		m_value = _mm_packs_epi32(m_value, _mm_setzero_si128());
		m_value = _mm_packus_epi16(m_value, _mm_setzero_si128());
		m_value = _mm_unpacklo_epi8(m_value, _mm_setzero_si128());
		m_value = _mm_unpacklo_epi16(m_value, _mm_setzero_si128());
	}

	inline void sign_extend(const UINT32 compare, const UINT32 sign)
	{
		__m128i compare_vec = _mm_set1_epi32(compare);
		__m128i compare_mask = _mm_cmpeq_epi32(_mm_and_si128(m_value, compare_vec), compare_vec);
		__m128i compared = _mm_and_si128(_mm_set1_epi32(sign), compare_mask);
		m_value = _mm_or_si128(m_value, compared);
	}

	inline void min(const INT32 value)
	{
		__m128i val = _mm_set1_epi32(value);
		__m128i is_greater_than = _mm_cmpgt_epi32(m_value, val);

		__m128i val_to_set = _mm_and_si128(val, is_greater_than);
		__m128i keep_mask = _mm_xor_si128(is_greater_than, _mm_set1_epi32(0xffffffff));

		m_value = _mm_and_si128(m_value, keep_mask);
		m_value = _mm_or_si128(val_to_set, m_value);
	}

	inline void max(const INT32 value)
	{
		__m128i val = _mm_set1_epi32(value);
		__m128i is_less_than = _mm_cmplt_epi32(m_value, val);

		__m128i val_to_set = _mm_and_si128(val, is_less_than);
		__m128i keep_mask = _mm_xor_si128(is_less_than, _mm_set1_epi32(0xffffffff));

		m_value = _mm_and_si128(m_value, keep_mask);
		m_value = _mm_or_si128(val_to_set, m_value);
	}

	void blend(const rgbaint_t& other, UINT8 factor);

	void scale_and_clamp(const rgbaint_t& scale);
	void scale_imm_and_clamp(const INT32 scale);

	inline void scale_imm_add_and_clamp(const INT32 scale, const rgbaint_t& other)
	{
		mul_imm(scale);
		sra_imm(8);
		add(other);
		clamp_to_uint8();
	}

	inline void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other)
	{
		mul(scale);
		sra_imm(8);
		add(other);
		clamp_to_uint8();
	}

	inline void scale2_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2)
	{
		rgbaint_t color2(other);
		color2.mul(scale2);

		mul(scale);
		add(color2);
		sra_imm(8);
		clamp_to_uint8();
	}

	inline void cmpeq(const rgbaint_t& value)
	{
		m_value = _mm_cmpeq_epi32(m_value, value.m_value);
	}

	inline void cmpeq_imm(const INT32 value)
	{
		m_value = _mm_cmpeq_epi32(m_value, _mm_set1_epi32(value));
	}

	inline void cmpeq_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_value = _mm_cmpeq_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void cmpgt(const rgbaint_t& value)
	{
		m_value = _mm_cmpgt_epi32(m_value, value.m_value);
	}

	inline void cmpgt_imm(const INT32 value)
	{
		m_value = _mm_cmpgt_epi32(m_value, _mm_set1_epi32(value));
	}

	inline void cmpgt_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_value = _mm_cmpgt_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void cmplt(const rgbaint_t& value)
	{
		m_value = _mm_cmplt_epi32(m_value, value.m_value);
	}

	inline void cmplt_imm(const INT32 value)
	{
		m_value = _mm_cmplt_epi32(m_value, _mm_set1_epi32(value));
	}

	inline void cmplt_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
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
		color01 = _mm_madd_epi16(color01, scale_factor(u));
		color11 = _mm_madd_epi16(color11, scale_factor(u));
		color01 = _mm_slli_epi32(color01, 15);
		color11 = _mm_srli_epi32(color11, 1);
		color01 = _mm_max_epi16(color01, color11);
		color01 = _mm_madd_epi16(color01, scale_factor(v));
		color01 = _mm_srli_epi32(color01, 15);
		color01 = _mm_packs_epi32(color01, _mm_setzero_si128());
		color01 = _mm_packus_epi16(color01, _mm_setzero_si128());
		return _mm_cvtsi128_si32(color01);
	}

	inline void bilinear_filter_rgbaint(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
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
		color01 = _mm_madd_epi16(color01, scale_factor(u));
		color11 = _mm_madd_epi16(color11, scale_factor(u));
		color01 = _mm_slli_epi32(color01, 15);
		color11 = _mm_srli_epi32(color11, 1);
		color01 = _mm_max_epi16(color01, color11);
		color01 = _mm_madd_epi16(color01, scale_factor(v));
		m_value = _mm_srli_epi32(color01, 15);
	}

protected:
	struct _statics
	{
		__m128  dummy_for_alignment;
		UINT16   alpha_mask[8];
		UINT16   red_mask[8];
		UINT16   green_mask[8];
		UINT16   blue_mask[8];
		INT16   scale_table[256][8];
	};

	static inline __m128i alpha_mask() { return *(__m128i *)&statics.alpha_mask[0]; }
	static inline __m128i red_mask() { return *(__m128i *)&statics.red_mask[0]; }
	static inline __m128i green_mask() { return *(__m128i *)&statics.green_mask[0]; }
	static inline __m128i blue_mask() { return *(__m128i *)&statics.blue_mask[0]; }
	static inline __m128i scale_factor(UINT8 index) { return *(__m128i *)&statics.scale_table[index][0]; }

	__m128i m_value;

	static const _statics statics;

};

#endif /* __RGBSSE__ */
