// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbgen.h

    General RGB utilities.

***************************************************************************/

#ifndef __RGBGEN__
#define __RGBGEN__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class rgbaint_t
{
public:
	inline rgbaint_t(): m_a(0), m_r(0), m_g(0), m_b(0) { }
	inline rgbaint_t(UINT32 rgba) { set(rgba); }
	inline rgbaint_t(INT32 a, INT32 r, INT32 g, INT32 b) { set(a, r, g, b); }
	inline rgbaint_t(rgb_t& rgba) { set(rgba); }

	inline void set(rgbaint_t& other) { set(other.m_a, other.m_r, other.m_g, other.m_b); }
	inline void set(UINT32 rgba) { set((rgba >> 24) & 0xff, (rgba >> 16) & 0xff, (rgba >> 8) & 0xff, rgba & 0xff); }
	inline void set(INT32 a, INT32 r, INT32 g, INT32 b)
	{
		m_a = a;
		m_r = r;
		m_g = g;
		m_b = b;
	}
	inline void set(rgb_t& rgba) { set(rgba.a(), rgba.r(), rgba.g(), rgba.b()); }

	inline rgb_t to_rgba()
	{
		return rgb_t(m_a, m_r, m_g, m_b);
	}

	inline rgb_t to_rgba_clamp()
	{
		UINT8 a = (m_a < 0) ? 0 : (m_a > 255) ? 255 : m_a;
		UINT8 r = (m_r < 0) ? 0 : (m_r > 255) ? 255 : m_r;
		UINT8 g = (m_g < 0) ? 0 : (m_g > 255) ? 255 : m_g;
		UINT8 b = (m_b < 0) ? 0 : (m_b > 255) ? 255 : m_b;
		return rgb_t(a, r, g, b);
	}

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

	inline void subr(rgbaint_t& color)
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

	inline void set_a(const INT32 value)
	{
		m_a = value;
	}

	inline void set_r(const INT32 value)
	{
		m_r = value;
	}

	inline void set_g(const INT32 value)
	{
		m_g = value;
	}

	inline void set_b(const INT32 value)
	{
		m_b = value;
	}

	inline UINT8 get_a() const
	{
		return m_a;
	}

	inline UINT8 get_r() const
	{
		return m_r;
	}

	inline UINT8 get_g() const
	{
		return m_g;
	}

	inline UINT8 get_b() const
	{
		return m_b;
	}

	inline INT32 get_a32() const
	{
		return m_a;
	}

	inline INT32 get_r32() const
	{
		return m_r;
	}

	inline INT32 get_g32() const
	{
		return m_g;
	}

	inline INT32 get_b32() const
	{
		return m_b;
	}

	inline void mul(rgbaint_t& color)
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
		m_a >>= shift.m_a;
		m_r >>= shift.m_r;
		m_g >>= shift.m_g;
		m_b >>= shift.m_b;
	}

	inline void shr_imm(const UINT8 shift)
	{
		if (shift == 0)
			return;

		m_a >>= shift;
		m_r >>= shift;
		m_g >>= shift;
		m_b >>= shift;
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

	inline void or_reg(const rgbaint_t& color)
	{
		or_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	inline void or_imm(const INT32 imm)
	{
		or_imm_rgba(imm, imm, imm, imm);
	}

	inline void or_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_a |= a;
		m_r |= r;
		m_g |= g;
		m_b |= b;
	}

	inline void and_reg(const rgbaint_t& color)
	{
		and_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	inline void andnot_reg(const rgbaint_t& color)
	{
		and_imm_rgba(~color.m_a, ~color.m_r, ~color.m_g, ~color.m_b);
	}

	inline void and_imm(const INT32 imm)
	{
		and_imm_rgba(imm, imm, imm, imm);
	}

	inline void and_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b)
	{
		m_a &= a;
		m_r &= r;
		m_g &= g;
		m_b &= b;
	}

	inline void xor_reg(const rgbaint_t& color)
	{
		xor_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	inline void xor_imm(const INT32 imm)
	{
		xor_imm_rgba(imm, imm, imm, imm);
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
		if (m_a & sign)
			m_a = 0;

		if (m_r & sign)
			m_r = 0;

		if (m_g & sign)
			m_g = 0;

		if (m_b & sign)
			m_b = 0;

		m_a = (m_a < 0) ? 0 : (m_a > 255) ? 255 : m_a;
		m_r = (m_r < 0) ? 0 : (m_r > 255) ? 255 : m_r;
		m_g = (m_g < 0) ? 0 : (m_g > 255) ? 255 : m_g;
		m_b = (m_b < 0) ? 0 : (m_b > 255) ? 255 : m_b;
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

	void blend(const rgbaint_t& other, UINT8 factor);

	void scale_and_clamp(const rgbaint_t& scale);
	void scale_imm_and_clamp(const INT32 scale);
	void scale2_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2);
	void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other);
	void scale_imm_add_and_clamp(const INT32 scale, const rgbaint_t& other);

	inline void cmpeq(const rgbaint_t& value)
	{
		m_a = (m_a == value.m_a) ? 0xffffffff : 0;
		m_r = (m_r == value.m_r) ? 0xffffffff : 0;
		m_g = (m_g == value.m_g) ? 0xffffffff : 0;
		m_b = (m_b == value.m_b) ? 0xffffffff : 0;
	}

	inline void cmpeq_imm(const INT32 value)
	{
		m_a = (m_a == value) ? 0xffffffff : 0;
		m_r = (m_r == value) ? 0xffffffff : 0;
		m_g = (m_g == value) ? 0xffffffff : 0;
		m_b = (m_b == value) ? 0xffffffff : 0;
	}

	inline void cmpgt(const rgbaint_t& value)
	{
		m_a = (m_a > value.m_a) ? 0xffffffff : 0;
		m_r = (m_r > value.m_r) ? 0xffffffff : 0;
		m_g = (m_g > value.m_g) ? 0xffffffff : 0;
		m_b = (m_b > value.m_b) ? 0xffffffff : 0;
	}

	inline void cmpgt_imm(const INT32 value)
	{
		m_a = (m_a > value) ? 0xffffffff : 0;
		m_r = (m_r > value) ? 0xffffffff : 0;
		m_g = (m_g > value) ? 0xffffffff : 0;
		m_b = (m_b > value) ? 0xffffffff : 0;
	}

	inline void cmplt(const rgbaint_t& value)
	{
		m_a = (m_a < value.m_a) ? 0xffffffff : 0;
		m_r = (m_r < value.m_r) ? 0xffffffff : 0;
		m_g = (m_g < value.m_g) ? 0xffffffff : 0;
		m_b = (m_b < value.m_b) ? 0xffffffff : 0;
	}

	inline void cmplt_imm(const INT32 value)
	{
		m_a = (m_a < value) ? 0xffffffff : 0;
		m_r = (m_r < value) ? 0xffffffff : 0;
		m_g = (m_g < value) ? 0xffffffff : 0;
		m_b = (m_b < value) ? 0xffffffff : 0;
	}

	inline void merge_alpha(rgbaint_t& alpha)
	{
		m_a = alpha.m_a;
	}

	inline rgbaint_t operator=(const rgbaint_t& other)
	{
		m_a = other.m_a;
		m_r = other.m_r;
		m_g = other.m_g;
		m_b = other.m_b;
		return *this;
	}

	inline rgbaint_t& operator+=(const rgbaint_t& other)
	{
		m_a += other.m_a;
		m_r += other.m_r;
		m_g += other.m_g;
		m_b += other.m_b;
		return *this;
	}

	inline rgbaint_t& operator+=(const INT32 other)
	{
		m_a += other;
		m_r += other;
		m_g += other;
		m_b += other;
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

	inline void bilinear_filter_rgbaint(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
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

#endif /* __RGBGEN__ */
