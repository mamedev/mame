// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbgen.h

    General RGB utilities.

***************************************************************************/

#ifndef MAME_EMU_VIDEO_RGBGEN_H
#define MAME_EMU_VIDEO_RGBGEN_H


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class rgbaint_t
{
public:
	rgbaint_t(): m_a(0), m_r(0), m_g(0), m_b(0) { }
	explicit rgbaint_t(UINT32 rgba) { set(rgba); }
	rgbaint_t(INT32 a, INT32 r, INT32 g, INT32 b) { set(a, r, g, b); }
	explicit rgbaint_t(const rgb_t& rgba) { set(rgba); }

	rgbaint_t(const rgbaint_t& other) = default;
	rgbaint_t &operator=(const rgbaint_t& other) = default;

	void set(const rgbaint_t& other) { set(other.m_a, other.m_r, other.m_g, other.m_b); }
	void set(UINT32 rgba) { set((rgba >> 24) & 0xff, (rgba >> 16) & 0xff, (rgba >> 8) & 0xff, rgba & 0xff); }
	void set(INT32 a, INT32 r, INT32 g, INT32 b)
	{
		m_a = a;
		m_r = r;
		m_g = g;
		m_b = b;
	}
	void set(const rgb_t& rgba) { set(rgba.a(), rgba.r(), rgba.g(), rgba.b()); }

	rgb_t to_rgba() const { return rgb_t(get_a(), get_r(), get_g(), get_b()); }

	rgb_t to_rgba_clamp() const
	{
		const UINT8 a = (m_a < 0) ? 0 : (m_a > 255) ? 255 : m_a;
		const UINT8 r = (m_r < 0) ? 0 : (m_r > 255) ? 255 : m_r;
		const UINT8 g = (m_g < 0) ? 0 : (m_g > 255) ? 255 : m_g;
		const UINT8 b = (m_b < 0) ? 0 : (m_b > 255) ? 255 : m_b;
		return rgb_t(a, r, g, b);
	}

	void set_a(const INT32 value) { m_a = value; }
	void set_r(const INT32 value) { m_r = value; }
	void set_g(const INT32 value) { m_g = value; }
	void set_b(const INT32 value) { m_b = value; }

	UINT8 get_a() const { return UINT8(UINT32(m_a)); }
	UINT8 get_r() const { return UINT8(UINT32(m_r)); }
	UINT8 get_g() const { return UINT8(UINT32(m_g)); }
	UINT8 get_b() const { return UINT8(UINT32(m_b)); }

	INT32 get_a32() const { return m_a; }
	INT32 get_r32() const { return m_r; }
	INT32 get_g32() const { return m_g; }
	INT32 get_b32() const { return m_b; }

	inline void add(const rgbaint_t& color)
	{
		add_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	inline void add_imm(const INT32 imm)
	{
		add_imm_rgba(imm, imm, imm, imm);
	}

	inline void add_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_a += a;
		m_r += r;
		m_g += g;
		m_b += b;
	}

	inline void sub(const rgbaint_t& color)
	{
		sub_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	inline void sub_imm(const INT32 imm)
	{
		sub_imm_rgba(imm, imm, imm, imm);
	}

	inline void sub_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_a -= a;
		m_r -= r;
		m_g -= g;
		m_b -= b;
	}

	inline void subr(const rgbaint_t& color)
	{
		subr_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	inline void subr_imm(const INT32 imm)
	{
		subr_imm_rgba(imm, imm, imm, imm);
	}

	inline void subr_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_a = a - m_a;
		m_r = r - m_r;
		m_g = g - m_g;
		m_b = b - m_b;
	}

	inline void mul(const rgbaint_t& color)
	{
		mul_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	inline void mul_imm(const INT32 imm)
	{
		mul_imm_rgba(imm, imm, imm, imm);
	}

	inline void mul_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_a *= a;
		m_r *= r;
		m_g *= g;
		m_b *= b;
	}

	inline void shl(const rgbaint_t& shift)
	{
		m_a <<= shift.m_a;
		m_r <<= shift.m_r;
		m_g <<= shift.m_g;
		m_b <<= shift.m_b;
	}

	inline void shl_imm(const UINT8 shift)
	{
		if (shift == 0)
			return;

		m_a <<= shift;
		m_r <<= shift;
		m_g <<= shift;
		m_b <<= shift;
	}

	inline void shr(const rgbaint_t& shift)
	{
		m_a = INT32(UINT32(m_a) >> shift.m_a);
		m_r = INT32(UINT32(m_r) >> shift.m_r);
		m_g = INT32(UINT32(m_g) >> shift.m_g);
		m_b = INT32(UINT32(m_b) >> shift.m_b);
	}

	inline void shr_imm(const UINT8 shift)
	{
		if (shift == 0)
			return;

		m_a = INT32(UINT32(m_a) >> shift);
		m_r = INT32(UINT32(m_r) >> shift);
		m_g = INT32(UINT32(m_g) >> shift);
		m_b = INT32(UINT32(m_b) >> shift);
	}

	inline void sra(const rgbaint_t& shift)
	{
		m_a >>= shift.m_a;
		if (m_a & (1 << (31 - shift.m_a)))
			m_a |= ~0 << (32 - shift.m_a);

		m_r >>= shift.m_r;
		if (m_r & (1 << (31 - shift.m_r)))
			m_r |= ~0 << (32 - shift.m_r);

		m_g >>= shift.m_g;
		if (m_g & (1 << (31 - shift.m_g)))
			m_g |= ~0 << (32 - shift.m_g);

		m_b >>= shift.m_b;
		if (m_b & (1 << (31 - shift.m_b)))
			m_b |= ~0 << (32 - shift.m_b);
	}

	inline void sra_imm(const UINT8 shift)
	{
		const UINT32 high_bit = 1 << (31 - shift);
		const UINT32 high_mask = ~0 << (32 - shift);

		m_a >>= shift;
		if (m_a & high_bit)
			m_a |= high_mask;

		m_r >>= shift;
		if (m_r & high_bit)
			m_r |= high_mask;

		m_g >>= shift;
		if (m_g & high_bit)
			m_g |= high_mask;

		m_b >>= shift;
		if (m_b & high_bit)
			m_b |= high_mask;
	}

	void or_reg(const rgbaint_t& color) { or_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b); }
	void and_reg(const rgbaint_t& color) { and_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b); }
	void xor_reg(const rgbaint_t& color) { xor_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b); }

	void andnot_reg(const rgbaint_t& color) { and_imm_rgba(~color.m_a, ~color.m_r, ~color.m_g, ~color.m_b); }

	void or_imm(INT32 imm) { or_imm_rgba(imm, imm, imm, imm); }
	void and_imm(INT32 imm) { and_imm_rgba(imm, imm, imm, imm); }
	void xor_imm(INT32 imm) { xor_imm_rgba(imm, imm, imm, imm); }

	inline void or_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_a |= a;
		m_r |= r;
		m_g |= g;
		m_b |= b;
	}

	inline void and_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_a &= a;
		m_r &= r;
		m_g &= g;
		m_b &= b;
	}

	inline void xor_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_a ^= a;
		m_r ^= r;
		m_g ^= g;
		m_b ^= b;
	}

	inline void clamp_and_clear(const UINT32 sign)
	{
		if (m_a & sign) m_a = 0;
		if (m_r & sign) m_r = 0;
		if (m_g & sign) m_g = 0;
		if (m_b & sign) m_b = 0;

		clamp_to_uint8();
	}

	inline void clamp_to_uint8()
	{
		m_a = (m_a < 0) ? 0 : (m_a > 255) ? 255 : m_a;
		m_r = (m_r < 0) ? 0 : (m_r > 255) ? 255 : m_r;
		m_g = (m_g < 0) ? 0 : (m_g > 255) ? 255 : m_g;
		m_b = (m_b < 0) ? 0 : (m_b > 255) ? 255 : m_b;
	}

	inline void sign_extend(const UINT32 compare, const UINT32 sign)
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

	inline void min(const INT32 value)
	{
		m_a = (m_a > value) ? value : m_a;
		m_r = (m_r > value) ? value : m_r;
		m_g = (m_g > value) ? value : m_g;
		m_b = (m_b > value) ? value : m_b;
	}

	inline void max(const INT32 value)
	{
		m_a = (m_a < value) ? value : m_a;
		m_r = (m_r < value) ? value : m_r;
		m_g = (m_g < value) ? value : m_g;
		m_b = (m_b < value) ? value : m_b;
	}

	void blend(const rgbaint_t& other, UINT8 factor);

	void scale_and_clamp(const rgbaint_t& scale);
	void scale_imm_and_clamp(const INT32 scale);
	void scale2_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2);
	void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other);
	void scale_imm_add_and_clamp(const INT32 scale, const rgbaint_t& other);

	void cmpeq(const rgbaint_t& value) { cmpeq_imm_rgba(value.m_a, value.m_r, value.m_g, value.m_b); }
	void cmpgt(const rgbaint_t& value) { cmpgt_imm_rgba(value.m_a, value.m_r, value.m_g, value.m_b); }
	void cmplt(const rgbaint_t& value) { cmplt_imm_rgba(value.m_a, value.m_r, value.m_g, value.m_b); }

	void cmpeq_imm(INT32 value) { cmpeq_imm_rgba(value, value, value, value); }
	void cmpgt_imm(INT32 value) { cmpgt_imm_rgba(value, value, value, value); }
	void cmplt_imm(INT32 value) { cmplt_imm_rgba(value, value, value, value); }

	void cmpeq_imm_rgba(INT32 a, INT32 r, INT32 g, INT32 b)
	{
		m_a = (m_a == a) ? 0xffffffff : 0;
		m_r = (m_r == r) ? 0xffffffff : 0;
		m_g = (m_g == g) ? 0xffffffff : 0;
		m_b = (m_b == b) ? 0xffffffff : 0;
	}

	void cmpgt_imm_rgba(INT32 a, INT32 r, INT32 g, INT32 b)
	{
		m_a = (m_a > a) ? 0xffffffff : 0;
		m_r = (m_r > r) ? 0xffffffff : 0;
		m_g = (m_g > g) ? 0xffffffff : 0;
		m_b = (m_b > b) ? 0xffffffff : 0;
	}

	void cmplt_imm_rgba(INT32 a, INT32 r, INT32 g, INT32 b)
	{
		m_a = (m_a < a) ? 0xffffffff : 0;
		m_r = (m_r < r) ? 0xffffffff : 0;
		m_g = (m_g < g) ? 0xffffffff : 0;
		m_b = (m_b < b) ? 0xffffffff : 0;
	}

	void merge_alpha(const rgbaint_t& alpha)
	{
		m_a = alpha.m_a;
	}

	rgbaint_t& operator+=(const rgbaint_t& other)
	{
		add_imm_rgba(other.m_a, other.m_r, other.m_g, other.m_b);
		return *this;
	}

	rgbaint_t& operator+=(const INT32 other)
	{
		add_imm_rgba(other, other, other, other);
		return *this;
	}

	rgbaint_t &operator-=(const rgbaint_t& other)
	{
		sub_imm_rgba(other.m_a, other.m_r, other.m_g, other.m_b);
		return *this;
	}

	rgbaint_t& operator*=(const rgbaint_t& other)
	{
		mul_imm_rgba(other.m_a, other.m_r, other.m_g, other.m_b);
		return *this;
	}

	rgbaint_t& operator*=(const INT32 other)
	{
		mul_imm_rgba(other, other, other, other);
		return *this;
	}

	rgbaint_t& operator>>=(const INT32 shift)
	{
		sra_imm(shift);
		return *this;
	}

	static UINT32 bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
	{
		UINT32 rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
		UINT32 rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

		rgb00 >>= 8;
		rgb01 >>= 8;
		rgb10 >>= 8;
		rgb11 >>= 8;

		UINT32 ag0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
		UINT32 ag1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

		rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
		ag0 = (ag0 & 0x00ff00ff) + ((((ag1 & 0x00ff00ff) - (ag0 & 0x00ff00ff)) * v) >> 8);

		return ((ag0 << 8) & 0xff00ff00) | (rb0 & 0x00ff00ff);
	}

	void bilinear_filter_rgbaint(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
	{
		UINT32 rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
		UINT32 rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

		rgb00 >>= 8;
		rgb01 >>= 8;
		rgb10 >>= 8;
		rgb11 >>= 8;

		UINT32 ag0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
		UINT32 ag1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

		rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
		ag0 = (ag0 & 0x00ff00ff) + ((((ag1 & 0x00ff00ff) - (ag0 & 0x00ff00ff)) * v) >> 8);

		UINT32 result = ((ag0 << 8) & 0xff00ff00) | (rb0 & 0x00ff00ff);
		this->set(result);
	}

protected:
	INT32 m_a;
	INT32 m_r;
	INT32 m_g;
	INT32 m_b;
};

#endif // MAME_EMU_VIDEO_RGBGEN_H
