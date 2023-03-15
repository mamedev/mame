// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbsse.h

    SSE optimized RGB utilities.

    WARNING: This code assumes SSE2 or greater capability.

***************************************************************************/

#ifndef MAME_EMU_VIDEO_RGBSSE_H
#define MAME_EMU_VIDEO_RGBSSE_H

#pragma once

#include <emmintrin.h>
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class rgbaint_t
{
public:
	rgbaint_t() { }
	explicit rgbaint_t(u32 rgba) { set(rgba); }
	rgbaint_t(s32 a, s32 r, s32 g, s32 b) { set(a, r, g, b); }
	explicit rgbaint_t(const rgb_t& rgb) { set(rgb); }
	explicit rgbaint_t(__m128i rgba) { m_value = rgba; }

	rgbaint_t(const rgbaint_t& other) = default;
	rgbaint_t &operator=(const rgbaint_t& other) = default;

	void set(const rgbaint_t& other) { m_value = other.m_value; }
	void set(const u32& rgba) { m_value = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(rgba), _mm_setzero_si128()), _mm_setzero_si128()); }
	void set(s32 a, s32 r, s32 g, s32 b) { m_value = _mm_set_epi32(a, r, g, b); }
	void set(const rgb_t& rgb) { set((const u32&) rgb); }
	// This function sets all elements to the same val
	void set_all(const s32& val) { m_value = _mm_set1_epi32(val); }
	// This function zeros all elements
	void zero() { m_value = _mm_xor_si128(m_value, m_value); }
	// This function zeros only the alpha element
	void zero_alpha() { m_value = _mm_and_si128(m_value, alpha_mask()); }

	inline rgb_t to_rgba() const
	{
		return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(m_value, _mm_setzero_si128()), _mm_setzero_si128()));
	}

	inline rgb_t to_rgba_clamp() const
	{
		return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(m_value, _mm_setzero_si128()), _mm_setzero_si128()));
	}

	void set_a16(const s32 value) { m_value = _mm_insert_epi16(m_value, value, 6); }
#ifdef __SSE4_1__
	void set_a(const s32 value) { m_value = _mm_insert_epi32(m_value, value, 3); }
	void set_r(const s32 value) { m_value = _mm_insert_epi32(m_value, value, 2); }
	void set_g(const s32 value) { m_value = _mm_insert_epi32(m_value, value, 1); }
	void set_b(const s32 value) { m_value = _mm_insert_epi32(m_value, value, 0); }
#else
	void set_a(const s32 value) { m_value = _mm_or_si128(_mm_and_si128(m_value, alpha_mask()), _mm_set_epi32(value, 0, 0, 0)); }
	void set_r(const s32 value) { m_value = _mm_or_si128(_mm_and_si128(m_value, red_mask()), _mm_set_epi32(0, value, 0, 0)); }
	void set_g(const s32 value) { m_value = _mm_or_si128(_mm_and_si128(m_value, green_mask()), _mm_set_epi32(0, 0, value, 0)); }
	void set_b(const s32 value) { m_value = _mm_or_si128(_mm_and_si128(m_value, blue_mask()), _mm_set_epi32(0, 0, 0, value)); }
#endif

	u8 get_a() const { return u8(unsigned(_mm_extract_epi16(m_value, 6))); }
	u8 get_r() const { return u8(unsigned(_mm_extract_epi16(m_value, 4))); }
	u8 get_g() const { return u8(unsigned(_mm_extract_epi16(m_value, 2))); }
	u8 get_b() const { return u8(unsigned(_mm_cvtsi128_si32(m_value))); }

#ifdef __SSE4_1__
	s32 get_a32() const { return _mm_extract_epi32(m_value, 3); }
	s32 get_r32() const { return _mm_extract_epi32(m_value, 2); }
	s32 get_g32() const { return _mm_extract_epi32(m_value, 1); }
	s32 get_b32() const { return _mm_extract_epi32(m_value, 0); }
#else
	s32 get_a32() const { return (_mm_cvtsi128_si32(_mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 3)))); }
	s32 get_r32() const { return (_mm_cvtsi128_si32(_mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 2)))); }
	s32 get_g32() const { return (_mm_cvtsi128_si32(_mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 1)))); }
	s32 get_b32() const { return (_mm_cvtsi128_si32(m_value)); }
#endif

	// These selects return an rgbaint_t with all fields set to the element choosen (a, r, g, or b)
	rgbaint_t select_alpha32() const { return (rgbaint_t)_mm_shuffle_epi32(m_value, _MM_SHUFFLE(3, 3, 3, 3)); }
	rgbaint_t select_red32() const { return (rgbaint_t)_mm_shuffle_epi32(m_value, _MM_SHUFFLE(2, 2, 2, 2)); }
	rgbaint_t select_green32() const { return (rgbaint_t)_mm_shuffle_epi32(m_value, _MM_SHUFFLE(1, 1, 1, 1)); }
	rgbaint_t select_blue32() const { return (rgbaint_t)_mm_shuffle_epi32(m_value, _MM_SHUFFLE(0, 0, 0, 0)); }

	inline void add(const rgbaint_t& color2)
	{
		m_value = _mm_add_epi32(m_value, color2.m_value);
	}

	inline void add_imm(const s32 imm)
	{
		m_value = _mm_add_epi32(m_value, _mm_set1_epi32(imm));
	}

	inline void add_imm_rgba(const s32 a, const s32 r, const s32 g, const s32 b)
	{
		m_value = _mm_add_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void sub(const rgbaint_t& color2)
	{
		m_value = _mm_sub_epi32(m_value, color2.m_value);
	}

	inline void sub_imm(const s32 imm)
	{
		m_value = _mm_sub_epi32(m_value, _mm_set1_epi32(imm));
	}

	inline void sub_imm_rgba(const s32 a, const s32 r, const s32 g, const s32 b)
	{
		m_value = _mm_sub_epi32(m_value, _mm_set_epi32(a, r, g, b));
	}

	inline void subr(const rgbaint_t& color2)
	{
		m_value = _mm_sub_epi32(color2.m_value, m_value);
	}

	inline void subr_imm(const s32 imm)
	{
		m_value = _mm_sub_epi32(_mm_set1_epi32(imm), m_value);
	}

	inline void subr_imm_rgba(const s32 a, const s32 r, const s32 g, const s32 b)
	{
		m_value = _mm_sub_epi32(_mm_set_epi32(a, r, g, b), m_value);
	}

	inline void mul(const rgbaint_t& color)
	{
		__m128i tmp1 = _mm_mul_epu32(m_value, color.m_value);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(color.m_value, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void mul_imm(const s32 imm)
	{
		__m128i immv = _mm_set1_epi32(imm);
		__m128i tmp1 = _mm_mul_epu32(m_value, immv);
		__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4));
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
	}

	inline void mul_imm_rgba(const s32 a, const s32 r, const s32 g, const s32 b)
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

	inline void shl_imm(const u8 shift)
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

	inline void shr_imm(const u8 shift)
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

	inline void sra_imm(const u8 shift)
	{
		m_value = _mm_srai_epi32(m_value, shift);
	}

	void or_reg(const rgbaint_t& color2) { m_value = _mm_or_si128(m_value, color2.m_value); }
	void and_reg(const rgbaint_t& color2) { m_value = _mm_and_si128(m_value, color2.m_value); }
	void xor_reg(const rgbaint_t& color2) { m_value = _mm_xor_si128(m_value, color2.m_value); }

	void andnot_reg(const rgbaint_t& color2) { m_value = _mm_andnot_si128(color2.m_value, m_value); }

	void or_imm(s32 value) { m_value = _mm_or_si128(m_value, _mm_set1_epi32(value)); }
	void and_imm(s32 value) { m_value = _mm_and_si128(m_value, _mm_set1_epi32(value)); }
	void xor_imm(s32 value) { m_value = _mm_xor_si128(m_value, _mm_set1_epi32(value)); }

	void or_imm_rgba(s32 a, s32 r, s32 g, s32 b) { m_value = _mm_or_si128(m_value, _mm_set_epi32(a, r, g, b)); }
	void and_imm_rgba(s32 a, s32 r, s32 g, s32 b) { m_value = _mm_and_si128(m_value, _mm_set_epi32(a, r, g, b)); }
	void xor_imm_rgba(s32 a, s32 r, s32 g, s32 b) { m_value = _mm_xor_si128(m_value, _mm_set_epi32(a, r, g, b)); }

	inline void clamp_and_clear(const u32 sign)
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

	inline void sign_extend(const u32 compare, const u32 sign)
	{
		__m128i compare_vec = _mm_set1_epi32(compare);
		__m128i compare_mask = _mm_cmpeq_epi32(_mm_and_si128(m_value, compare_vec), compare_vec);
		__m128i compared = _mm_and_si128(_mm_set1_epi32(sign), compare_mask);
		m_value = _mm_or_si128(m_value, compared);
	}

	inline void min(const s32 value)
	{
		__m128i val = _mm_set1_epi32(value);
#ifdef __SSE4_1__
		m_value = _mm_min_epi32(m_value, val);
#else
		__m128i is_greater_than = _mm_cmpgt_epi32(m_value, val);

		__m128i val_to_set = _mm_and_si128(val, is_greater_than);
		__m128i keep_mask = _mm_xor_si128(is_greater_than, _mm_set1_epi32(0xffffffff));

		m_value = _mm_and_si128(m_value, keep_mask);
		m_value = _mm_or_si128(val_to_set, m_value);
#endif
	}

	inline void max(const s32 value)
	{
		__m128i val = _mm_set1_epi32(value);
#ifdef __SSE4_1__
		m_value = _mm_max_epi32(m_value, val);
#else
		__m128i is_less_than = _mm_cmplt_epi32(m_value, val);

		__m128i val_to_set = _mm_and_si128(val, is_less_than);
		__m128i keep_mask = _mm_xor_si128(is_less_than, _mm_set1_epi32(0xffffffff));

		m_value = _mm_and_si128(m_value, keep_mask);
		m_value = _mm_or_si128(val_to_set, m_value);
#endif
	}

	void blend(const rgbaint_t& other, u8 factor);

	void scale_and_clamp(const rgbaint_t& scale);

	// Leave this here in case Model3 blows up...
	//inline void scale_imm_and_clamp(const s32 scale)
	//{
	//  mul_imm(scale);
	//  sra_imm(8);
	//  clamp_to_uint8();
	//}

	// This version needs absolute value of value and scale to be 11 bits or less
	inline void scale_imm_and_clamp(const s16 scale)
	{
		// Set mult a 16 bit inputs to scale
		__m128i immv = _mm_set1_epi16(scale);
		// Shift up by 4
		immv = _mm_slli_epi16(immv, 4);
		// Pack color into mult b 16 bit inputs
		m_value = _mm_packs_epi32(m_value, _mm_setzero_si128());
		// Shift up by 4
		m_value = _mm_slli_epi16(m_value, 4);
		// Do the 16 bit multiply, bottom 64 bits will contain 16 bit truncated results
		m_value = _mm_mulhi_epi16(m_value, immv);
		// Clamp to u8
		m_value = _mm_packus_epi16(m_value, _mm_setzero_si128());
		// Unpack up to s32
		m_value = _mm_unpacklo_epi8(m_value, _mm_setzero_si128());
		m_value = _mm_unpacklo_epi16(m_value, _mm_setzero_si128());
	}

	// This function needs absolute value of color and scale to be 15 bits or less
	inline void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other)
	{
#ifdef __SSE4_1__
		m_value = _mm_mullo_epi32(m_value, scale.m_value);
#else
		// Mask off the top 16 bits of each 32-bit value
		m_value = _mm_and_si128(m_value, _mm_set1_epi32(0x0000ffff));
		// Do 16x16 multiplies and sum into 32-bit pairs; the AND above ensures upper pair is always 0
		m_value = _mm_madd_epi16(m_value, scale.m_value);
#endif
		// Arithmetic shift down the result by 8 bits
		sra_imm(8);
		add(other);
		clamp_to_uint8();
	}

	// This function needs absolute value of color and scale to be 15 bits or less
	inline void scale2_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2)
	{
		// Pack 32-bit values to 16-bit values in low half, and scales in top half
		__m128i tmp1 = _mm_packs_epi32(m_value, scale.m_value);
		// Same for other and scale2
		__m128i tmp2 = _mm_packs_epi32(other.m_value, scale2.m_value);
		// Interleave the low halves (m_value, other)
		__m128i tmp3 = _mm_unpacklo_epi16(tmp1, tmp2);
		// Interleave the top halves (scale, scale2)
		__m128i tmp4 = _mm_unpackhi_epi16(tmp1, tmp2);
		// Multiply values by scales and add adjacent pairs
		m_value = _mm_madd_epi16(tmp3, tmp4);
		// Final shift by 8
		sra_imm(8);
		clamp_to_uint8();
	}

	void cmpeq(const rgbaint_t& value) { m_value = _mm_cmpeq_epi32(m_value, value.m_value); }
	void cmpgt(const rgbaint_t& value) { m_value = _mm_cmpgt_epi32(m_value, value.m_value); }
	void cmplt(const rgbaint_t& value) { m_value = _mm_cmplt_epi32(m_value, value.m_value); }

	void cmpeq_imm(s32 value) { m_value = _mm_cmpeq_epi32(m_value, _mm_set1_epi32(value)); }
	void cmpgt_imm(s32 value) { m_value = _mm_cmpgt_epi32(m_value, _mm_set1_epi32(value)); }
	void cmplt_imm(s32 value) { m_value = _mm_cmplt_epi32(m_value, _mm_set1_epi32(value)); }

	void cmpeq_imm_rgba(s32 a, s32 r, s32 g, s32 b) { m_value = _mm_cmpeq_epi32(m_value, _mm_set_epi32(a, r, g, b)); }
	void cmpgt_imm_rgba(s32 a, s32 r, s32 g, s32 b) { m_value = _mm_cmpgt_epi32(m_value, _mm_set_epi32(a, r, g, b)); }
	void cmplt_imm_rgba(s32 a, s32 r, s32 g, s32 b) { m_value = _mm_cmplt_epi32(m_value, _mm_set_epi32(a, r, g, b)); }

	inline rgbaint_t& operator+=(const rgbaint_t& other)
	{
		m_value = _mm_add_epi32(m_value, other.m_value);
		return *this;
	}

	inline rgbaint_t& operator+=(const s32 other)
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

	inline rgbaint_t& operator*=(const s32 other)
	{
		const __m128i immv = _mm_set1_epi32(other);
		m_value = _mm_unpacklo_epi32(_mm_shuffle_epi32(_mm_mul_epu32(m_value, immv), _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(_mm_mul_epu32(_mm_srli_si128(m_value, 4), _mm_srli_si128(immv, 4)), _MM_SHUFFLE(0, 0, 2, 0)));
		return *this;
	}

	inline rgbaint_t& operator>>=(const s32 shift)
	{
		m_value = _mm_srai_epi32(m_value, shift);
		return *this;
	}

	inline void merge_alpha16(const rgbaint_t& alpha)
	{
		m_value = _mm_insert_epi16(m_value, _mm_extract_epi16(alpha.m_value, 6), 6);
	}

	inline void merge_alpha(const rgbaint_t& alpha)
	{
#ifdef __SSE4_1__
		m_value = _mm_insert_epi32(m_value, _mm_extract_epi32(alpha.m_value, 3), 3);
#else
		m_value = _mm_insert_epi16(m_value, _mm_extract_epi16(alpha.m_value, 7), 7);
		m_value = _mm_insert_epi16(m_value, _mm_extract_epi16(alpha.m_value, 6), 6);
#endif
	}

	static u32 bilinear_filter(u32 rgb00, u32 rgb01, u32 rgb10, u32 rgb11, u8 u, u8 v)
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

	void bilinear_filter_rgbaint(u32 rgb00, u32 rgb01, u32 rgb10, u32 rgb11, u8 u, u8 v)
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
		u16     alpha_mask[8];
		u16     red_mask[8];
		u16     green_mask[8];
		u16     blue_mask[8];
		s16     scale_table[256][8];
	};

	static __m128i alpha_mask() { return *(__m128i *)&statics.alpha_mask[0]; }
	static __m128i red_mask() { return *(__m128i *)&statics.red_mask[0]; }
	static __m128i green_mask() { return *(__m128i *)&statics.green_mask[0]; }
	static __m128i blue_mask() { return *(__m128i *)&statics.blue_mask[0]; }
	static __m128i scale_factor(u8 index) { return *(__m128i *)&statics.scale_table[index][0]; }

	__m128i m_value;

	static const _statics statics;

};

#endif /* MAME_EMU_VIDEO_RGBSSE_H */
