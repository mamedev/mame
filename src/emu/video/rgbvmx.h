// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbvmx.h

    VMX/Altivec optimised RGB utilities.

***************************************************************************/

#ifndef __RGBVMX__
#define __RGBVMX__

#include <altivec.h>

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class rgbaint_t
{
protected:
	typedef __vector signed char    VECS8;
	typedef __vector unsigned char  VECU8;
	typedef __vector signed short   VECS16;
	typedef __vector unsigned short VECU16;
	typedef __vector signed int     VECS32;
	typedef __vector unsigned int   VECU32;

public:
	inline rgbaint_t(): VECS8(0), VECU8(0), VECS16(0), VECU16(0), VECS32(0), VECU32(0) { }
	inline rgbaint_t(UINT32 rgba) { set(rgba); }
	inline rgbaint_t(INT32 a, INT32 r, INT32 g, INT32 b) { set(a, r, g, b); }
	inline rgbaint_t(rgb_t& rgb) { set(rgb); }
	inline rgbaint_t(VECS32 rgba): VECS8(0), VECU8(0), VECS16(0), VECU16(0), VECS32(0), VECU32(0) { m_value = rgba; }

	inline void set(rgbaint_t& other) { m_value = other.m_value; }

	inline void set(UINT32 rgba)
	{
		const VECU32 zero = { 0, 0, 0, 0 };
		const VECS8 temp = vec_perm(vec_lde(0, &rgba), zero, vec_lvsl(0, &rgba));
		m_value = vec_mergeh((VECS16)zero, (VECS16)vec_mergeh((VECS8)zero, temp));
	}

	inline void set(INT32 a, INT32 r, INT32 g, INT32 b)
	{
		VECS32 result = { a, r, g, b };
		m_value = result;
	}

	inline void set(rgb_t& rgb)
	{
		const VECU32 zero = { 0, 0, 0, 0 };
		const VECS8 temp = vec_perm(vec_lde(0, rgb.ptr()), zero, vec_lvsl(0, rgb.ptr()));
		m_value = vec_mergeh((VECS16)zero, (VECS16)vec_mergeh((VECS8)zero, temp));
	}

	inline rgb_t to_rgba()
	{
		VECU32 temp = vec_packs(m_value, m_value);
		temp = vec_packsu((VECS16)temp, (VECS16)temp);
		UINT32 result;
		vec_ste(temp, 0, &result);
		return result;
	}

	inline rgb_t to_rgba_clamp()
	{
		VECU32 temp = vec_packs(m_value, m_value);
		temp = vec_packsu((VECS16)temp, (VECS16)temp);
		UINT32 result;
		vec_ste(temp, 0, &result);
		return result;
	}

	inline void add(const rgbaint_t& color2)
	{
		m_value = vec_add(m_value, color2.m_value);
	}

	inline void add_imm(const INT32 imm)
	{
		const VECS32 temp = { imm, imm, imm, imm };
		m_value = vec_add(m_value, temp);
	}

	inline void add_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECS32 temp = { a, r, g, b };
		m_value = vec_add(m_value, temp);
	}

	inline void sub(const rgbaint_t& color2)
	{
		m_value = vec_sub(m_value, color2.m_value);
	}

	inline void sub_imm(const INT32 imm)
	{
		const VECS32 temp = { imm, imm, imm, imm };
		m_value = vec_sub(m_value, temp);
	}

	inline void sub_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECS32 temp = { a, r, g, b };
		m_value = vec_sub(m_value, temp);
	}

	inline void subr(rgbaint_t& color2)
	{
		m_value = vec_sub(color2.m_value, m_value);
	}

	inline void subr_imm(const INT32 imm)
	{
		const VECS32 temp = { imm, imm, imm, imm };
		m_value = vec_sub(temp, m_value);
	}

	inline void subr_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECS32 temp = { a, r, g, b };
		m_value = vec_sub(temp, m_value);
	}

	inline void set_a(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_perm(m_value, temp, alpha_perm);
	}

	inline void set_r(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_perm(m_value, temp, red_perm);
	}

	inline void set_g(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_perm(m_value, temp, green_perm);
	}

	inline void set_b(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_perm(m_value, temp, blue_perm);
	}

	inline UINT8 get_a() const
	{
		UINT8 result;
		vec_ste(vec_splat((VECU8)m_value, 3), 0, &result);
		return result;
	}

	inline UINT8 get_r() const
	{
		UINT8 result;
		vec_ste(vec_splat((VECU8)m_value, 7), 0, &result);
		return result;
	}

	inline UINT8 get_g() const
	{
		UINT8 result;
		vec_ste(vec_splat((VECU8)m_value, 11), 0, &result);
		return result;
	}

	inline UINT8 get_b() const
	{
		UINT8 result;
		vec_ste(vec_splat((VECU8)m_value, 15), 0, &result);
		return result;
	}

	inline INT32 get_a32() const
	{
		INT32 result;
		vec_ste(vec_splat(m_value, 0), 0, &result);
		return result;
	}

	inline INT32 get_r32() const
	{
		INT32 result;
		vec_ste(vec_splat(m_value, 1), 0, &result);
		return result;
	}

	inline INT32 get_g32() const
	{
		INT32 result;
		vec_ste(vec_splat(m_value, 2), 0, &result);
		return result;
	}

	inline INT32 get_b32() const
	{
		INT32 result;
		vec_ste(vec_splat(m_value, 3), 0, &result);
		return result;
	}

	inline void mul(const rgbaint_t& color)
	{
		const VECU32 shift = vec_splat_u32(-16);
		const VECU32 temp = vec_msum((VECU16)m_value, (VECU16)vec_rl(color.m_value, shift), vec_splat_u32(0));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((VECU16)m_value, (VECU16)color.m_value));
	}

	inline void mul_imm(const INT32 imm)
	{
		const VECU32 value = { imm, imm, imm, imm };
		const VECU32 shift = vec_splat_u32(-16);
		const VECU32 temp = vec_msum((VECU16)m_value, (VECU16)vec_rl(value, shift), vec_splat_u32(0));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((VECU16)m_value, (VECU16)value));
	}

	inline void mul_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECU32 value = { a, r, g, b };
		const VECU32 shift = vec_splat_u32(-16);
		const VECU32 temp = vec_msum((VECU16)m_value, (VECU16)vec_rl(value, shift), vec_splat_u32(0));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((VECU16)m_value, (VECU16)value));
	}

	inline void shl(const rgbaint_t& shift)
	{
		const VECU32 limit = { 32, 32, 32, 32 };
		m_value = vec_and(vec_sl(m_value, (VECU32)shift.m_value), vec_cmpgt(limit, (VECU32)shift.m_value));
	}

	inline void shl_imm(const UINT8 shift)
	{
		const VECU32 temp = { shift, shift, shift, shift };
		m_value = vec_sl(m_value, temp);
	}

	inline void shr(const rgbaint_t& shift)
	{
		const VECU32 limit = { 32, 32, 32, 32 };
		m_value = vec_and(vec_sr(m_value, (VECU32)shift.m_value), vec_cmpgt(limit, (VECU32)shift.m_value));
	}

	inline void shr_imm(const UINT8 shift)
	{
		const VECU32 temp = { shift, shift, shift, shift };
		m_value = vec_sr(m_value, temp);
	}

	inline void sra(const rgbaint_t& shift)
	{
		const VECU32 limit = { 31, 31, 31, 31 };
		m_value = vec_sra(m_value, vec_min((VECU32)shift.m_value, limit));
	}

	inline void sra_imm(const UINT8 shift)
	{
		const VECU32 temp = { shift, shift, shift, shift };
		m_value = vec_sra(m_value, temp);
	}

	inline void or_reg(const rgbaint_t& color2)
	{
		m_value = vec_or(m_value, color2.m_value);
	}

	inline void or_imm(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_or(m_value, temp);
	}

	inline void or_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECS32 temp = { a, r, g, b };
		m_value = vec_or(m_value, temp);
	}

	inline void and_reg(const rgbaint_t& color)
	{
		m_value = vec_and(m_value, color.m_value);
	}

	inline void andnot_reg(const rgbaint_t& color)
	{
		m_value = vec_andc(m_value, color.m_value);
	}

	inline void and_imm(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_and(m_value, temp);
	}

	inline void and_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECS32 temp = { a, r, g, b };
		m_value = vec_and(m_value, temp);
	}

	inline void xor_reg(const rgbaint_t& color2)
	{
		m_value = vec_xor(m_value, color2.m_value);
	}

	inline void xor_imm(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_xor(m_value, temp);
	}

	inline void xor_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECS32 temp = { a, r, g, b };
		m_value = vec_xor(m_value, temp);
	}

	inline void clamp_and_clear(const UINT32 sign)
	{
		const VECS32 vzero = { 0, 0, 0, 0 };
		VECS32 vsign = { sign, sign, sign, sign };
		m_value = vec_and(m_value, vec_cmpeq(vec_and(m_value, vsign), vzero));
		vsign = vec_nor(vec_sra(vsign, vec_splat_u32(1)), vzero);
		const VECS32 mask = vec_cmpgt(m_value, vsign);
		m_value = vec_or(vec_and(vsign, mask), vec_and(m_value, vec_nor(mask, vzero)));
	}

	inline void clamp_to_uint8()
	{
		const VECU32 zero = { 0, 0, 0, 0 };
		m_value = vec_packs(m_value, m_value);
		m_value = vec_packsu((VECS16)m_value, (VECS16)m_value);
		m_value = vec_mergeh((VECU8)zero, (VECU8)m_value);
		m_value = vec_mergeh((VECS16)zero, (VECS16)m_value);
	}

	inline void sign_extend(const UINT32 compare, const UINT32 sign)
	{
		const VECS32 compare_vec = { compare, compare, compare, compare };
		const VECS32 compare_mask = vec_cmpeq(vec_and(m_value, compare_vec), compare_vec);
		const VECS32 sign_vec = { sign, sign, sign, sign };
		m_value = vec_or(m_value, vec_and(sign_vec, compare_mask));
	}

	inline void min(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_min(m_value, temp);
	}

	inline void max(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_max(m_value, temp);
	}

	void blend(const rgbaint_t& other, UINT8 factor);

	void scale_and_clamp(const rgbaint_t& scale);
	void scale_imm_and_clamp(const INT32 scale);

	void scale_imm_add_and_clamp(const INT32 scale, const rgbaint_t& other)
	{
		mul_imm(scale);
		sra_imm(8);
		add(other);
		clamp_to_uint8();
	}

	void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other)
	{
		mul(scale);
		sra_imm(8);
		add(other);
		clamp_to_uint8();
	}

	void scale2_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2)
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
		m_value = vec_cmpeq(m_value, value.m_value);
	}

	inline void cmpeq_imm(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_cmpeq(m_value, temp);
	}

	inline void cmpeq_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECS32 temp = { a, r, g, b };
		m_value = vec_cmpeq(m_value, temp);
	}

	inline void cmpgt(const rgbaint_t& value)
	{
		m_value = vec_cmpgt(m_value, value.m_value);
	}

	inline void cmpgt_imm(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_cmpgt(m_value, temp);
	}

	inline void cmpgt_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECS32 temp = { a, r, g, b };
		m_value = vec_cmpgt(m_value, temp);
	}

	inline void cmplt(const rgbaint_t& value)
	{
		m_value = vec_cmplt(m_value, value.m_value);
	}

	inline void cmplt_imm(const INT32 value)
	{
		const VECS32 temp = { value, value, value, value };
		m_value = vec_cmplt(m_value, temp);
	}

	inline void cmplt_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		const VECS32 temp = { a, r, g, b };
		m_value = vec_cmplt(m_value, temp);
	}

	inline rgbaint_t operator=(const rgbaint_t& other)
	{
		m_value = other.m_value;
		return *this;
	}

	inline rgbaint_t& operator+=(const rgbaint_t& other)
	{
		m_value = vec_add(m_value, other.m_value);
		return *this;
	}

	inline rgbaint_t& operator+=(const INT32 other)
	{
		const VECS32 temp = { other, other, other, other };
		m_value = vec_add(m_value, temp);
		return *this;
	}

	inline rgbaint_t& operator-=(const rgbaint_t& other)
	{
		m_value = vec_sub(m_value, other.m_value);
		return *this;
	}

	inline rgbaint_t& operator*=(const rgbaint_t& other)
	{
		const VECU32 shift = vec_splat_u32(-16);
		const VECU32 temp = vec_msum((VECU16)m_value, (VECU16)vec_rl(other.m_value, shift), vec_splat_u32(0));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((VECU16)m_value, (VECU16)other.m_value));
		return *this;
	}

	inline rgbaint_t& operator*=(const INT32 other)
	{
		const VECS32 value = { other, other, other, other };
		const VECU32 shift = vec_splat_u32(-16);
		const VECU32 temp = vec_msum((VECU16)m_value, (VECU16)vec_rl(value, shift), vec_splat_u32(0));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((VECU16)m_value, (VECU16)value));
		return *this;
	}

	inline rgbaint_t& operator>>=(const INT32 shift)
	{
		const VECU32 temp = { shift, shift, shift, shift };
		m_value = vec_sra(m_value, temp);
		return *this;
	}

	inline void merge_alpha(const rgbaint_t& alpha)
	{
		m_value = vec_perm(m_value, alpha.m_value, alpha_perm);
	}

	static UINT32 bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
	{
		const VECS32 zero = vec_splat_s32(0);

		VECS32 color00 = vec_perm((VECS32)vec_lde(0, &rgb00), zero, vec_lvsl(0, &rgb00));
		VECS32 color01 = vec_perm((VECS32)vec_lde(0, &rgb01), zero, vec_lvsl(0, &rgb01));
		VECS32 color10 = vec_perm((VECS32)vec_lde(0, &rgb10), zero, vec_lvsl(0, &rgb10));
		VECS32 color11 = vec_perm((VECS32)vec_lde(0, &rgb11), zero, vec_lvsl(0, &rgb11));

		/* interleave color01 and color00 at the byte level */
		color01 = vec_mergeh((VECU8)color01, (VECU8)color00);
		color11 = vec_mergeh((VECU8)color11, (VECU8)color10);
		color01 = vec_mergeh((VECU8)zero, (VECU8)color01);
		color11 = vec_mergeh((VECU8)zero, (VECU8)color11);
		color01 = vec_msum((VECS16)color01, scale_table[u], zero);
		color11 = vec_msum((VECS16)color11, scale_table[u], zero);
		color01 = vec_sl(color01, vec_splat_u32(15));
		color11 = vec_sr(color11, vec_splat_u32(1));
		color01 = vec_max((VECS16)color01, (VECS16)color11);
		color01 = vec_msum((VECS16)color01, scale_table[v], zero);
		color01 = vec_sr(color01, vec_splat_u32(15));
		color01 = vec_packs(color01, color01);
		color01 = vec_packsu((VECS16)color01, (VECS16)color01);

		UINT32 result;
		vec_ste((VECU32)color01, 0, &result);
		return result;
	}

	inline void bilinear_filter_rgbaint(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
	{
		const VECS32 zero = vec_splat_s32(0);

		VECS32 color00 = vec_perm((VECS32)vec_lde(0, &rgb00), zero, vec_lvsl(0, &rgb00));
		VECS32 color01 = vec_perm((VECS32)vec_lde(0, &rgb01), zero, vec_lvsl(0, &rgb01));
		VECS32 color10 = vec_perm((VECS32)vec_lde(0, &rgb10), zero, vec_lvsl(0, &rgb10));
		VECS32 color11 = vec_perm((VECS32)vec_lde(0, &rgb11), zero, vec_lvsl(0, &rgb11));

		/* interleave color01 and color00 at the byte level */
		color01 = vec_mergeh((VECU8)color01, (VECU8)color00);
		color11 = vec_mergeh((VECU8)color11, (VECU8)color10);
		color01 = vec_mergeh((VECU8)zero, (VECU8)color01);
		color11 = vec_mergeh((VECU8)zero, (VECU8)color11);
		color01 = vec_msum((VECS16)color01, scale_table[u], zero);
		color11 = vec_msum((VECS16)color11, scale_table[u], zero);
		color01 = vec_sl(color01, vec_splat_u32(15));
		color11 = vec_sr(color11, vec_splat_u32(1));
		color01 = vec_max((VECS16)color01, (VECS16)color11);
		color01 = vec_msum((VECS16)color01, scale_table[v], zero);
		m_value = vec_sr(color01, vec_splat_u32(15));
	}

protected:
	VECS32                          m_value;

	static const VECU8              alpha_perm;
	static const VECU8              red_perm;
	static const VECU8              green_perm;
	static const VECU8              blue_perm;
	static const VECS16             scale_table[256];
};



// altivec.h somehow redefines "bool" in a bad way on PowerPC Mac OS X.  really.
#ifdef OSX_PPC
#undef vector
#undef pixel
#undef bool
#endif

#endif /* __RGBVMX__ */
