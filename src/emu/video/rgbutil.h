// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbutil.h

    Utility definitions for RGB manipulation. Allows RGB handling to be
    performed in an abstracted fashion and optimized with SIMD.

***************************************************************************/

#ifndef MAME_EMU_VIDEO_RGBUTIL_H
#define MAME_EMU_VIDEO_RGBUTIL_H

#pragma once

#include <algorithm>


class rgbaint_t
{
public:
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 2)) || defined(__ALTIVEC__)
	static inline constexpr unsigned FILTER_SHIFT_INNER = 0;
	static inline constexpr unsigned FILTER_SHIFT_OUTER = 1;
#elif defined(__ARM_NEON)
	static inline constexpr unsigned FILTER_SHIFT_INNER = 0;
	static inline constexpr unsigned FILTER_SHIFT_OUTER = 0;
#else
	static inline constexpr unsigned FILTER_SHIFT_INNER = 1;
	static inline constexpr unsigned FILTER_SHIFT_OUTER = 0;
#endif

	rgbaint_t() = default;
	explicit rgbaint_t(u32 rgba) { set(rgba); }
	rgbaint_t(s32 a, s32 r, s32 g, s32 b) { set(a, r, g, b); }
	explicit rgbaint_t(const rgb_t &rgba) { set(rgba); }

	rgbaint_t(const rgbaint_t &other) = default;
	rgbaint_t &operator=(const rgbaint_t &other) = default;

	void set(const rgbaint_t &other) { *this = other; }
	void set(u32 rgba) { set((rgba >> 24) & 0xff, (rgba >> 16) & 0xff, (rgba >> 8) & 0xff, rgba & 0xff); }
	void set(s32 a, s32 r, s32 g, s32 b)
	{
		m_a = a;
		m_r = r;
		m_g = g;
		m_b = b;
	}
	void set(const rgb_t &rgba) { set(rgba.a(), rgba.r(), rgba.g(), rgba.b()); }
	// This function sets all elements to the same val
	void set_all(s32 val) { set(val, val, val, val); }
	// This function zeros all elements
	void zero() { set_all(0); }
	// This function zeros only the alpha element
	void zero_alpha() { m_a = 0; }

	rgb_t to_rgba() const { return rgb_t(get_a(), get_r(), get_g(), get_b()); }

	rgb_t to_rgba_clamp() const
	{
		const auto a = u8(u32(std::clamp<s32>(m_a, 0, 255)));
		const auto r = u8(u32(std::clamp<s32>(m_r, 0, 255)));
		const auto g = u8(u32(std::clamp<s32>(m_g, 0, 255)));
		const auto b = u8(u32(std::clamp<s32>(m_b, 0, 255)));
		return rgb_t(a, r, g, b);
	}

	void set_a16(s32 value) { m_a = value; }
	void set_a(s32 value) { m_a = value; }
	void set_r(s32 value) { m_r = value; }
	void set_g(s32 value) { m_g = value; }
	void set_b(s32 value) { m_b = value; }

	u8 get_a() const { return u8(u32(m_a)); }
	u8 get_r() const { return u8(u32(m_r)); }
	u8 get_g() const { return u8(u32(m_g)); }
	u8 get_b() const { return u8(u32(m_b)); }

	s32 get_a32() const { return m_a; }
	s32 get_r32() const { return m_r; }
	s32 get_g32() const { return m_g; }
	s32 get_b32() const { return m_b; }

	// These selects return an rgbaint_t with all fields set to the element choosen (a, r, g, or b)
	rgbaint_t select_alpha32() const { return rgbaint_t(m_a, m_a, m_a, m_a); }
	rgbaint_t select_red32() const { return rgbaint_t(m_r, m_r, m_r, m_r); }
	rgbaint_t select_green32() const { return rgbaint_t(m_g, m_g, m_g, m_g); }
	rgbaint_t select_blue32() const { return rgbaint_t(m_b, m_b, m_b, m_b); }

	void add(const rgbaint_t &color)
	{
		add_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	void add_imm(s32 imm)
	{
		add_imm_rgba(imm, imm, imm, imm);
	}

	void add_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a += a;
		m_r += r;
		m_g += g;
		m_b += b;
	}

	void sub(const rgbaint_t &color)
	{
		sub_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	void sub_imm(s32 imm)
	{
		sub_imm_rgba(imm, imm, imm, imm);
	}

	void sub_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a -= a;
		m_r -= r;
		m_g -= g;
		m_b -= b;
	}

	void subr(const rgbaint_t &color)
	{
		subr_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	void subr_imm(s32 imm)
	{
		subr_imm_rgba(imm, imm, imm, imm);
	}

	void subr_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a = a - m_a;
		m_r = r - m_r;
		m_g = g - m_g;
		m_b = b - m_b;
	}

	void mul(const rgbaint_t &color)
	{
		mul_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	void mul_imm(s32 imm)
	{
		mul_imm_rgba(imm, imm, imm, imm);
	}

	void mul_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a *= a;
		m_r *= r;
		m_g *= g;
		m_b *= b;
	}

	void shl(const rgbaint_t &shift)
	{
		m_a = (u32(shift.m_a) > 31) ? 0 : (m_a << shift.m_a);
		m_r = (u32(shift.m_r) > 31) ? 0 : (m_r << shift.m_r);
		m_g = (u32(shift.m_g) > 31) ? 0 : (m_g << shift.m_g);
		m_b = (u32(shift.m_b) > 31) ? 0 : (m_b << shift.m_b);
	}

	void shl_imm(u8 shift)
	{
		if (shift > 31)
		{
			zero();
		}
		else
		{
			m_a <<= shift;
			m_r <<= shift;
			m_g <<= shift;
			m_b <<= shift;
		}
	}

	void shr(const rgbaint_t &shift)
	{
		m_a = (u32(shift.m_a) > 31) ? 0 : s32(u32(m_a) >> shift.m_a);
		m_r = (u32(shift.m_r) > 31) ? 0 : s32(u32(m_r) >> shift.m_r);
		m_g = (u32(shift.m_g) > 31) ? 0 : s32(u32(m_g) >> shift.m_g);
		m_b = (u32(shift.m_b) > 31) ? 0 : s32(u32(m_b) >> shift.m_b);
	}

	void shr_imm(u8 shift)
	{
		if (shift > 31)
		{
			zero();
		}
		else
		{
			m_a = s32(u32(m_a) >> shift);
			m_r = s32(u32(m_r) >> shift);
			m_g = s32(u32(m_g) >> shift);
			m_b = s32(u32(m_b) >> shift);
		}
	}

	void sra(const rgbaint_t &shift)
	{
		m_a >>= (u32(shift.m_a) > 31) ? 31 : shift.m_a;
		m_r >>= (u32(shift.m_r) > 31) ? 31 : shift.m_r;
		m_g >>= (u32(shift.m_g) > 31) ? 31 : shift.m_g;
		m_b >>= (u32(shift.m_b) > 31) ? 31 : shift.m_b;
	}

	void sra_imm(u8 shift)
	{
		const u8 s = std::min<u8>(shift, 31);
		m_a >>= s;
		m_r >>= s;
		m_g >>= s;
		m_b >>= s;
	}

	void or_reg(const rgbaint_t &color) { or_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b); }
	void and_reg(const rgbaint_t &color) { and_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b); }
	void xor_reg(const rgbaint_t &color) { xor_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b); }

	void andnot_reg(const rgbaint_t &color) { and_imm_rgba(~color.m_a, ~color.m_r, ~color.m_g, ~color.m_b); }

	void or_imm(s32 imm) { or_imm_rgba(imm, imm, imm, imm); }
	void and_imm(s32 imm) { and_imm_rgba(imm, imm, imm, imm); }
	void xor_imm(s32 imm) { xor_imm_rgba(imm, imm, imm, imm); }

	void or_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a |= a;
		m_r |= r;
		m_g |= g;
		m_b |= b;
	}

	void and_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a &= a;
		m_r &= r;
		m_g &= g;
		m_b &= b;
	}

	void xor_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a ^= a;
		m_r ^= r;
		m_g ^= g;
		m_b ^= b;
	}

	void clamp_and_clear(u32 sign)
	{
		m_a = (m_a & sign) ? 0 : std::min<s32>(m_a, 255);
		m_r = (m_r & sign) ? 0 : std::min<s32>(m_r, 255);
		m_g = (m_g & sign) ? 0 : std::min<s32>(m_g, 255);
		m_b = (m_b & sign) ? 0 : std::min<s32>(m_b, 255);
	}

	void clamp_to_uint8()
	{
		m_a = std::clamp<s32>(m_a, 0, 255);
		m_r = std::clamp<s32>(m_r, 0, 255);
		m_g = std::clamp<s32>(m_g, 0, 255);
		m_b = std::clamp<s32>(m_b, 0, 255);
	}

	void sign_extend(u32 compare, u32 sign)
	{
		if ((m_a & compare) == compare)
			m_a |= sign;

		if ((m_r & compare) == compare)
			m_r |= sign;

		if ((m_g & compare) == compare)
			m_g |= sign;

		if ((m_b & compare) == compare)
			m_b |= sign;
	}

	void min(s32 value)
	{
		m_a = std::min(m_a, value);
		m_r = std::min(m_r, value);
		m_g = std::min(m_g, value);
		m_b = std::min(m_b, value);
	}

	void max(s32 value)
	{
		m_a = std::max(m_a, value);
		m_r = std::max(m_r, value);
		m_g = std::max(m_g, value);
		m_b = std::max(m_b, value);
	}

	void blend(const rgbaint_t &other, u8 factor)
	{
		const s32 scale1 = s32(u32(factor));
		const s32 scale2 = 256 - scale1;

		m_a = ((m_a * scale1) + (other.m_a * scale2)) >> 8;
		m_r = ((m_r * scale1) + (other.m_r * scale2)) >> 8;
		m_g = ((m_g * scale1) + (other.m_g * scale2)) >> 8;
		m_b = ((m_b * scale1) + (other.m_b * scale2)) >> 8;
	}

	void scale_and_clamp(const rgbaint_t &scale)
	{
		m_a = std::clamp<s32>((m_a * scale.m_a) >> 8, 0, 255);
		m_r = std::clamp<s32>((m_r * scale.m_r) >> 8, 0, 255);
		m_g = std::clamp<s32>((m_g * scale.m_g) >> 8, 0, 255);
		m_b = std::clamp<s32>((m_b * scale.m_b) >> 8, 0, 255);
	}

	void scale_imm_and_clamp(s32 scale)
	{
		m_a = std::clamp<s32>((m_a * scale) >> 8, 0, 255);
		m_r = std::clamp<s32>((m_r * scale) >> 8, 0, 255);
		m_g = std::clamp<s32>((m_g * scale) >> 8, 0, 255);
		m_b = std::clamp<s32>((m_b * scale) >> 8, 0, 255);
	}

	void scale_add_and_clamp(const rgbaint_t &scale, const rgbaint_t &other)
	{
		m_a = std::clamp<s32>(((m_a * scale.m_a) >> 8) + other.m_a, 0, 255);
		m_r = std::clamp<s32>(((m_r * scale.m_r) >> 8) + other.m_r, 0, 255);
		m_g = std::clamp<s32>(((m_g * scale.m_g) >> 8) + other.m_g, 0, 255);
		m_b = std::clamp<s32>(((m_b * scale.m_b) >> 8) + other.m_b, 0, 255);
	}

	void scale2_add_and_clamp(const rgbaint_t &scale, const rgbaint_t &other, const rgbaint_t &scale2)
	{
		m_a = std::clamp<s32>(((m_a * scale.m_a) + (other.m_a * scale2.m_a)) >> 8, 0, 255);
		m_r = std::clamp<s32>(((m_r * scale.m_r) + (other.m_r * scale2.m_r)) >> 8, 0, 255);
		m_g = std::clamp<s32>(((m_g * scale.m_g) + (other.m_g * scale2.m_g)) >> 8, 0, 255);
		m_b = std::clamp<s32>(((m_b * scale.m_b) + (other.m_b * scale2.m_b)) >> 8, 0, 255);
	}

	void cmpeq(const rgbaint_t &value) { cmpeq_imm_rgba(value.m_a, value.m_r, value.m_g, value.m_b); }
	void cmpgt(const rgbaint_t &value) { cmpgt_imm_rgba(value.m_a, value.m_r, value.m_g, value.m_b); }
	void cmplt(const rgbaint_t &value) { cmplt_imm_rgba(value.m_a, value.m_r, value.m_g, value.m_b); }

	void cmpeq_imm(s32 value) { cmpeq_imm_rgba(value, value, value, value); }
	void cmpgt_imm(s32 value) { cmpgt_imm_rgba(value, value, value, value); }
	void cmplt_imm(s32 value) { cmplt_imm_rgba(value, value, value, value); }

	void cmpeq_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a = (m_a == a) ? 0xffffffff : 0;
		m_r = (m_r == r) ? 0xffffffff : 0;
		m_g = (m_g == g) ? 0xffffffff : 0;
		m_b = (m_b == b) ? 0xffffffff : 0;
	}

	void cmpgt_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a = (m_a > a) ? 0xffffffff : 0;
		m_r = (m_r > r) ? 0xffffffff : 0;
		m_g = (m_g > g) ? 0xffffffff : 0;
		m_b = (m_b > b) ? 0xffffffff : 0;
	}

	void cmplt_imm_rgba(s32 a, s32 r, s32 g, s32 b)
	{
		m_a = (m_a < a) ? 0xffffffff : 0;
		m_r = (m_r < r) ? 0xffffffff : 0;
		m_g = (m_g < g) ? 0xffffffff : 0;
		m_b = (m_b < b) ? 0xffffffff : 0;
	}

	void merge_alpha16(const rgbaint_t &alpha)
	{
		m_a = alpha.m_a;
	}

	void merge_alpha(const rgbaint_t &alpha)
	{
		m_a = alpha.m_a;
	}

	rgbaint_t &operator+=(const rgbaint_t &other)
	{
		add_imm_rgba(other.m_a, other.m_r, other.m_g, other.m_b);
		return *this;
	}

	rgbaint_t &operator+=(s32 other)
	{
		add_imm_rgba(other, other, other, other);
		return *this;
	}

	rgbaint_t &operator-=(const rgbaint_t &other)
	{
		sub_imm_rgba(other.m_a, other.m_r, other.m_g, other.m_b);
		return *this;
	}

	rgbaint_t &operator*=(const rgbaint_t &other)
	{
		mul_imm_rgba(other.m_a, other.m_r, other.m_g, other.m_b);
		return *this;
	}

	rgbaint_t &operator*=(s32 other)
	{
		mul_imm_rgba(other, other, other, other);
		return *this;
	}

	rgbaint_t &operator>>=(u8 shift)
	{
		sra_imm(shift);
		return *this;
	}

	static u32 bilinear_filter(u32 rgb00, u32 rgb01, u32 rgb10, u32 rgb11, u8 u, u8 v) noexcept;
	void bilinear_filter_rgbaint(u32 rgb00, u32 rgb01, u32 rgb10, u32 rgb11, u8 u, u8 v) noexcept;

protected:
	s32 m_a;
	s32 m_r;
	s32 m_g;
	s32 m_b;
};

#endif // MAME_EMU_VIDEO_RGBUTIL_H
