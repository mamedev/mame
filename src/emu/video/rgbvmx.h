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
public:
	inline rgbaint_t() { }
	inline rgbaint_t(UINT32 rgba) { set(rgba); }
	inline rgbaint_t(UINT32 a, UINT32 r, UINT32 g, UINT32 b) { set(a, r, g, b); }
	inline rgbaint_t(rgb_t& rgb) { set(rgb); }

	inline void set(rgbaint_t& other) { m_value = other.m_value; }

	inline void set(UINT32 rgba)
	{
		const vector unsigned int zero = vec_splat_u32(0);
		const vector unsigned char temp = vec_perm(vec_lde(0, &rgba), zero, vec_lvsl(0, &rgba));
		m_value = vec_mergeh((vector unsigned short)zero, (vector unsigned short)vec_mergeh((vector unsigned char)zero, temp));
	}

	inline void set(UINT32 a, UINT32 r, UINT32 g, UINT32 b)
	{
		vector unsigned int result = { a, r, g, b };
		m_value = result;
	}

	inline void set(rgb_t& rgb)
	{
		const vector unsigned int zero = vec_splat_u32(0);
		const vector unsigned char temp = vec_perm(vec_lde(0, rgb.ptr()), zero, vec_lvsl(0, rgb.ptr()));
		m_value = vec_mergeh((vector unsigned short)zero, (vector unsigned short)vec_mergeh((vector unsigned char)zero, temp));
	}

	inline rgb_t to_rgba()
	{
		const vector unsigned int temp = vec_splat((vector unsigned int)vec_pack(vec_pack(m_value, m_value), vec_splat_u16(0)), 0);
		UINT32 result;
		vec_ste(temp, 0, &result);
		return result;
	}

	inline rgb_t to_rgba_clamp()
	{
		const vector unsigned int temp = vec_splat((vector unsigned int)vec_packsu(vec_packsu(m_value, m_value), vec_splat_u16(0)), 0);
		UINT32 result;
		vec_ste(temp, 0, &result);
		return result;
	}

	inline void add(const rgbaint_t& color2)
	{
		m_value = vec_add(m_value, color2.m_value);
	}

	inline void add_imm(const UINT32 imm)
	{
		const vector unsigned int temp = { imm, imm, imm, imm };
		m_value = vec_add(m_value, temp);
	}

	inline void add_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int temp = { a, r, g, b };
		m_value = vec_add(m_value, temp);
	}

	inline void sub(const rgbaint_t& color2)
	{
		m_value = vec_sub(m_value, color2.m_value);
	}

	inline void sub_imm(const UINT32 imm)
	{
		const vector unsigned int temp = { imm, imm, imm, imm };
		m_value = vec_sub(m_value, temp);
	}

	inline void sub_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int temp = { a, r, g, b };
		m_value = vec_sub(m_value, temp);
	}

	inline void subr(rgbaint_t& color2)
	{
		m_value = vec_sub(color2.m_value, m_value);
	}

	inline void subr_imm(const UINT32 imm)
	{
		const vector unsigned int temp = { imm, imm, imm, imm };
		m_value = vec_sub(temp, m_value);
	}

	inline void subr_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int temp = { a, r, g, b };
		m_value = vec_sub(temp, m_value);
	}

	inline void set_a(const UINT32 value)
	{
		const vector unsigned int temp = { value, 0, 0, 0 };
		m_value = vec_perm(m_value, temp, alpha_perm);
	}

	inline void set_r(const UINT32 value)
	{
		const vector unsigned int temp = { value, 0, 0, 0 };
		m_value = vec_perm(m_value, temp, red_perm);
	}

	inline void set_g(const UINT32 value)
	{
		const vector unsigned int temp = { value, 0, 0, 0 };
		m_value = vec_perm(m_value, temp, green_perm);
	}

	inline void set_b(const UINT32 value)
	{
		const vector unsigned int temp = { value, 0, 0, 0 };
		m_value = vec_perm(m_value, temp, blue_perm);
	}

	inline UINT8 get_a()
	{
		UINT8 result;
		vec_ste(vec_splat((vector unsigned char)m_value, 3), 0, &result);
		return result;
	}

	inline UINT8 get_r()
	{
		UINT8 result;
		vec_ste(vec_splat((vector unsigned char)m_value, 7), 0, &result);
		return result;
	}

	inline UINT8 get_g()
	{
		UINT8 result;
		vec_ste(vec_splat((vector unsigned char)m_value, 11), 0, &result);
		return result;
	}

	inline UINT8 get_b()
	{
		UINT8 result;
		vec_ste(vec_splat((vector unsigned char)m_value, 15), 0, &result);
		return result;
	}

	inline UINT32 get_a32()
	{
		UINT32 result;
		vec_ste(vec_splat(m_value, 0), 0, &result);
		return result;
	}

	inline UINT32 get_r32()
	{
		UINT32 result;
		vec_ste(vec_splat(m_value, 1), 0, &result);
		return result;
	}

	inline UINT32 get_g32()
	{
		UINT32 result;
		vec_ste(vec_splat(m_value, 2), 0, &result);
		return result;
	}

	inline UINT32 get_b32()
	{
		UINT32 result;
		vec_ste(vec_splat(m_value, 3), 0, &result);
		return result;
	}

	inline void mul(const rgbaint_t& color)
	{
		const vector unsigned int shift = vec_splat_u32(-16);
		const vector unsigned int temp = vec_add(vec_mule((vector unsigned short)m_value, (vector unsigned short)vec_sl(color.m_value, shift)), vec_mule((vector unsigned short)vec_sl(m_value, shift), (vector unsigned short)color.m_value));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((vector unsigned short)m_value, (vector unsigned short)color.m_value));
	}

	inline void mul_imm(const UINT32 imm)
	{
		const vector unsigned int value = { imm, imm, imm, imm };
		const vector unsigned int shift = vec_splat_u32(-16);
		const vector unsigned int temp = vec_add(vec_mule((vector unsigned short)m_value, (vector unsigned short)vec_sl(value, shift)), vec_mule((vector unsigned short)vec_sl(m_value, shift), (vector unsigned short)value));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((vector unsigned short)m_value, (vector unsigned short)value));
	}

	inline void mul_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int value = { a, r, g, b };
		const vector unsigned int shift = vec_splat_u32(-16);
		const vector unsigned int temp = vec_add(vec_mule((vector unsigned short)m_value, (vector unsigned short)vec_sl(value, shift)), vec_mule((vector unsigned short)vec_sl(m_value, shift), (vector unsigned short)value));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((vector unsigned short)m_value, (vector unsigned short)value));
	}

	inline void shl(const rgbaint_t& shift)
	{
		const vector unsigned int limit = { 32, 32, 32, 32 };
		const vector unsigned int temp = vec_splat(shift.m_value, 3);
		m_value = vec_and(vec_sl(m_value, temp), vec_cmpgt(limit, temp));
	}

	inline void shl_imm(const UINT8 shift)
	{
		const vector unsigned int temp = { shift, shift, shift, shift };
		m_value = vec_sl(m_value, temp);
	}

	inline void shl_imm_all(const UINT8 shift)
	{
		const vector unsigned char limit = { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 };
		const vector unsigned char temp = { shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift };
		m_value = vec_and(vec_slo(m_value, temp), (vector unsigned int)vec_cmpgt(limit, temp));
	}

	inline void shr(const rgbaint_t& shift)
	{
		const vector unsigned int limit = { 32, 32, 32, 32 };
		const vector unsigned int temp = vec_splat(shift.m_value, 3);
		m_value = vec_and(vec_sr(m_value, temp), vec_cmpgt(limit, temp));
	}

	inline void shr_imm(const UINT8 shift)
	{
		const vector unsigned int temp = { shift, shift, shift, shift };
		m_value = vec_sr(m_value, temp);
	}

	inline void shr_imm_all(const UINT8 shift)
	{
		const vector unsigned char limit = { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 };
		const vector unsigned char temp = { shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift, shift };
		m_value = vec_and(vec_sro(m_value, temp), (vector unsigned int)vec_cmpgt(limit, temp));
	}

	inline void sra(const rgbaint_t& shift)
	{
		const vector unsigned int limit = { 31, 31, 31, 31 };
		m_value = vec_sra(m_value, vec_min(vec_splat(shift.m_value, 3), limit));
	}

	inline void sra_imm(const UINT8 shift)
	{
		const vector unsigned int temp = { shift, shift, shift, shift };
		m_value = vec_sra(m_value, temp);
	}

	inline void or_reg(const rgbaint_t& color2)
	{
		m_value = vec_or(m_value, color2.m_value);
	}

	inline void or_imm(const UINT32 value)
	{
		const vector unsigned int temp = { value, value, value, value };
		m_value = vec_or(m_value, temp);
	}

	inline void or_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int temp = { a, r, g, b };
		m_value = vec_or(m_value, temp);
	}

	inline void and_reg(const rgbaint_t& color)
	{
		m_value = vec_and(m_value, color.m_value);
	}

	inline void and_imm(const UINT32 value)
	{
		const vector unsigned int temp = { value, value, value, value };
		m_value = vec_and(m_value, temp);
	}

	inline void and_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int temp = { a, r, g, b };
		m_value = vec_and(m_value, temp);
	}

	inline void xor_reg(const rgbaint_t& color2)
	{
		m_value = vec_xor(m_value, color2.m_value);
	}

	inline void xor_imm(const INT32 value)
	{
		const vector unsigned int temp = { value, value, value, value };
		m_value = vec_xor(m_value, temp);
	}

	inline void xor_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int temp = { a, r, g, b };
		m_value = vec_xor(m_value, temp);
	}

	inline void clamp_and_clear(const UINT32 sign)
	{
		const vector unsigned int vzero = vec_splat_u32(0);
		vector unsigned int vsign = { sign, sign, sign, sign };
		m_value = vec_and(m_value, vec_cmpeq(vec_and(m_value, vsign), vzero));
		vsign = vec_nor(vec_sra(vsign, vec_splat_u32(1)), vzero);
		const vector unsigned int mask = vec_cmpgt(m_value, vsign);
		m_value = vec_or(vec_and(vsign, mask), vec_and(m_value, vec_nor(mask, vzero)));
	}

	inline void sign_extend(const UINT32 compare, const UINT32 sign)
	{
		const vector unsigned int compare_vec = { compare, compare, compare, compare };
		const vector unsigned int compare_mask = vec_cmpeq(vec_and(m_value, compare_vec), compare_vec);
		const vector unsigned int sign_vec = { sign, sign, sign, sign };
		m_value = vec_or(m_value, vec_and(sign_vec, compare_mask));
	}

	inline void min(const UINT32 value)
	{
		const vector unsigned int temp = { value, value, value, value };
		m_value = vec_min(m_value, temp);
	}

	void blend(const rgbaint_t& other, UINT8 factor);

	void scale_and_clamp(const rgbaint_t& scale);
	void scale_imm_and_clamp(const INT32 scale);
	void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2);
	void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other);
	void scale_imm_add_and_clamp(const INT32 scale, const rgbaint_t& other);

	inline void cmpeq(const rgbaint_t& value)
	{
		m_value = vec_cmpeq(m_value, value.m_value);
	}

	inline void cmpeq_imm(const UINT32 value)
	{
		const vector unsigned int temp = { value, value, value, value };
		m_value = vec_cmpeq(m_value, temp);
	}

	inline void cmpeq_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int temp = { a, r, g, b };
		m_value = vec_cmpeq(m_value, temp);
	}

	inline void cmpgt(const rgbaint_t& value)
	{
		m_value = vec_cmpgt(m_value, value.m_value);
	}

	inline void cmpgt_imm(const UINT32 value)
	{
		const vector unsigned int temp = { value, value, value, value };
		m_value = vec_cmpgt(m_value, temp);
	}

	inline void cmpgt_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int temp = { a, r, g, b };
		m_value = vec_cmpgt(m_value, temp);
	}

	inline void cmplt(const rgbaint_t& value)
	{
		m_value = vec_cmplt(m_value, value.m_value);
	}

	inline void cmplt_imm(const UINT32 value)
	{
		const vector unsigned int temp = { value, value, value, value };
		m_value = vec_cmplt(m_value, temp);
	}

	inline void cmplt_imm_rgba(const UINT32 a, const UINT32 r, const UINT32 g, const UINT32 b)
	{
		const vector unsigned int temp = { a, r, g, b };
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
		const vector unsigned int temp = { other, other, other, other };
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
		const vector unsigned int shift = vec_splat_u32(-16);
		const vector unsigned int temp = vec_add(vec_mule((vector unsigned short)m_value, (vector unsigned short)vec_sl(other.m_value, shift)), vec_mule((vector unsigned short)vec_sl(m_value, shift), (vector unsigned short)other.m_value));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((vector unsigned short)m_value, (vector unsigned short)other.m_value));
		return *this;
	}

	inline rgbaint_t& operator*=(const INT32 other)
	{
		const vector unsigned int value = { other, other, other, other };
		const vector unsigned int shift = vec_splat_u32(-16);
		const vector unsigned int temp = vec_add(vec_mule((vector unsigned short)m_value, (vector unsigned short)vec_sl(value, shift)), vec_mule((vector unsigned short)vec_sl(m_value, shift), (vector unsigned short)value));
		m_value = vec_add(vec_sl(temp, shift), vec_mulo((vector unsigned short)m_value, (vector unsigned short)value));
		return *this;
	}

	inline rgbaint_t& operator>>=(const INT32 shift)
	{
		const vector unsigned int temp = { shift, shift, shift, shift };
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

protected:
	typedef vector unsigned char    VECU8;
	typedef vector signed short     VECS16;
	typedef vector unsigned short   VECU16;
	typedef vector signed int       VECS32;
	typedef vector unsigned int     VECU32;

	vector VECU32                   m_value;

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
