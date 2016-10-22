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
	explicit rgbaint_t(uint32_t rgba) { set(rgba); }
	rgbaint_t(int32_t a, int32_t r, int32_t g, int32_t b) { set(a, r, g, b); }
	explicit rgbaint_t(const rgb_t& rgba) { set(rgba); }

	rgbaint_t(const rgbaint_t& other) = default;
	rgbaint_t &operator=(const rgbaint_t& other) = default;

	void set(const rgbaint_t& other) { set(other.m_a, other.m_r, other.m_g, other.m_b); }
	void set(uint32_t rgba) { set((rgba >> 24) & 0xff, (rgba >> 16) & 0xff, (rgba >> 8) & 0xff, rgba & 0xff); }
	void set(int32_t a, int32_t r, int32_t g, int32_t b)
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
		const uint8_t a = (m_a < 0) ? 0 : (m_a > 255) ? 255 : m_a;
		const uint8_t r = (m_r < 0) ? 0 : (m_r > 255) ? 255 : m_r;
		const uint8_t g = (m_g < 0) ? 0 : (m_g > 255) ? 255 : m_g;
		const uint8_t b = (m_b < 0) ? 0 : (m_b > 255) ? 255 : m_b;
		return rgb_t(a, r, g, b);
	}

	void set_a(const int32_t value) { m_a = value; }
	void set_r(const int32_t value) { m_r = value; }
	void set_g(const int32_t value) { m_g = value; }
	void set_b(const int32_t value) { m_b = value; }

	uint8_t get_a() const { return uint8_t(uint32_t(m_a)); }
	uint8_t get_r() const { return uint8_t(uint32_t(m_r)); }
	uint8_t get_g() const { return uint8_t(uint32_t(m_g)); }
	uint8_t get_b() const { return uint8_t(uint32_t(m_b)); }

	int32_t get_a32() const { return m_a; }
	int32_t get_r32() const { return m_r; }
	int32_t get_g32() const { return m_g; }
	int32_t get_b32() const { return m_b; }

	inline void add(const rgbaint_t& color)
	{
		add_imm_rgba(color.m_a, color.m_r, color.m_g, color.m_b);
	}

	inline void add_imm(const int32_t imm)
	{
		add_imm_rgba(imm, imm, imm, imm);
	}

	inline void add_imm_rgba(const int32_t a, const int32_t r, const int32_t g, const int32_t b)
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

	inline void sub_imm(const int32_t imm)
	{
		sub_imm_rgba(imm, imm, imm, imm);
	}

	inline void sub_imm_rgba(const int32_t a, const int32_t r, const int32_t g, const int32_t b)
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

	inline void subr_imm(const int32_t imm)
	{
		subr_imm_rgba(imm, imm, imm, imm);
	}

	inline void subr_imm_rgba(const int32_t a, const int32_t r, const int32_t g, const int32_t b)
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

	inline void mul_imm(const int32_t imm)
	{
		mul_imm_rgba(imm, imm, imm, imm);
	}

	inline void mul_imm_rgba(const int32_t a, const int32_t r, const int32_t g, const int32_t b)
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

	inline void shl_imm(const uint8_t shift)
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
		m_a = int32_t(uint32_t(m_a) >> shift.m_a);
		m_r = int32_t(uint32_t(m_r) >> shift.m_r);
		m_g = int32_t(uint32_t(m_g) >> shift.m_g);
		m_b = int32_t(uint32_t(m_b) >> shift.m_b);
	}

	inline void shr_imm(const uint8_t shift)
	{
		if (shift == 0)
			return;

		m_a = int32_t(uint32_t(m_a) >> shift);
		m_r = int32_t(uint32_t(m_r) >> shift);
		m_g = int32_t(uint32_t(m_g) >> shift);
		m_b = int32_t(uint32_t(m_b) >> shift);
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

	inline void sra_imm(const uint8_t shift)
	{
		const uint32_t high_bit = 1 << (31 - shift);
		const uint32_t high_mask = ~0 << (32 - shift);

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

	void or_imm(int32_t imm) { or_imm_rgba(imm, imm, imm, imm); }
	void and_imm(int32_t imm) { and_imm_rgba(imm, imm, imm, imm); }
	void xor_imm(int32_t imm) { xor_imm_rgba(imm, imm, imm, imm); }

	inline void or_imm_rgba(const int32_t a, const int32_t r, const int32_t g, const int32_t b)
	{
		m_a |= a;
		m_r |= r;
		m_g |= g;
		m_b |= b;
	}

	inline void and_imm_rgba(const int32_t a, const int32_t r, const int32_t g, const int32_t b)
	{
		m_a &= a;
		m_r &= r;
		m_g &= g;
		m_b &= b;
	}

	inline void xor_imm_rgba(const int32_t a, const int32_t r, const int32_t g, const int32_t b)
	{
		m_a ^= a;
		m_r ^= r;
		m_g ^= g;
		m_b ^= b;
	}

	inline void clamp_and_clear(const uint32_t sign)
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

	inline void sign_extend(const uint32_t compare, const uint32_t sign)
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

	inline void min(const int32_t value)
	{
		m_a = (m_a > value) ? value : m_a;
		m_r = (m_r > value) ? value : m_r;
		m_g = (m_g > value) ? value : m_g;
		m_b = (m_b > value) ? value : m_b;
	}

	inline void max(const int32_t value)
	{
		m_a = (m_a < value) ? value : m_a;
		m_r = (m_r < value) ? value : m_r;
		m_g = (m_g < value) ? value : m_g;
		m_b = (m_b < value) ? value : m_b;
	}

	void blend(const rgbaint_t& other, uint8_t factor);

	void scale_and_clamp(const rgbaint_t& scale);
	void scale_imm_and_clamp(const int32_t scale);
	void scale2_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2);
	void scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other);
	void scale_imm_add_and_clamp(const int32_t scale, const rgbaint_t& other);

	void cmpeq(const rgbaint_t& value) { cmpeq_imm_rgba(value.m_a, value.m_r, value.m_g, value.m_b); }
	void cmpgt(const rgbaint_t& value) { cmpgt_imm_rgba(value.m_a, value.m_r, value.m_g, value.m_b); }
	void cmplt(const rgbaint_t& value) { cmplt_imm_rgba(value.m_a, value.m_r, value.m_g, value.m_b); }

	void cmpeq_imm(int32_t value) { cmpeq_imm_rgba(value, value, value, value); }
	void cmpgt_imm(int32_t value) { cmpgt_imm_rgba(value, value, value, value); }
	void cmplt_imm(int32_t value) { cmplt_imm_rgba(value, value, value, value); }

	void cmpeq_imm_rgba(int32_t a, int32_t r, int32_t g, int32_t b)
	{
		m_a = (m_a == a) ? 0xffffffff : 0;
		m_r = (m_r == r) ? 0xffffffff : 0;
		m_g = (m_g == g) ? 0xffffffff : 0;
		m_b = (m_b == b) ? 0xffffffff : 0;
	}

	void cmpgt_imm_rgba(int32_t a, int32_t r, int32_t g, int32_t b)
	{
		m_a = (m_a > a) ? 0xffffffff : 0;
		m_r = (m_r > r) ? 0xffffffff : 0;
		m_g = (m_g > g) ? 0xffffffff : 0;
		m_b = (m_b > b) ? 0xffffffff : 0;
	}

	void cmplt_imm_rgba(int32_t a, int32_t r, int32_t g, int32_t b)
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

	rgbaint_t& operator+=(const int32_t other)
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

	rgbaint_t& operator*=(const int32_t other)
	{
		mul_imm_rgba(other, other, other, other);
		return *this;
	}

	rgbaint_t& operator>>=(const int32_t shift)
	{
		sra_imm(shift);
		return *this;
	}

	static uint32_t bilinear_filter(uint32_t rgb00, uint32_t rgb01, uint32_t rgb10, uint32_t rgb11, uint8_t u, uint8_t v)
	{
		uint32_t rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
		uint32_t rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

		rgb00 >>= 8;
		rgb01 >>= 8;
		rgb10 >>= 8;
		rgb11 >>= 8;

		uint32_t ag0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
		uint32_t ag1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

		rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
		ag0 = (ag0 & 0x00ff00ff) + ((((ag1 & 0x00ff00ff) - (ag0 & 0x00ff00ff)) * v) >> 8);

		return ((ag0 << 8) & 0xff00ff00) | (rb0 & 0x00ff00ff);
	}

	void bilinear_filter_rgbaint(uint32_t rgb00, uint32_t rgb01, uint32_t rgb10, uint32_t rgb11, uint8_t u, uint8_t v)
	{
		uint32_t rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
		uint32_t rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

		rgb00 >>= 8;
		rgb01 >>= 8;
		rgb10 >>= 8;
		rgb11 >>= 8;

		uint32_t ag0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
		uint32_t ag1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);

		rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
		ag0 = (ag0 & 0x00ff00ff) + ((((ag1 & 0x00ff00ff) - (ag0 & 0x00ff00ff)) * v) >> 8);

		uint32_t result = ((ag0 << 8) & 0xff00ff00) | (rb0 & 0x00ff00ff);
		this->set(result);
	}

protected:
	int32_t m_a;
	int32_t m_r;
	int32_t m_g;
	int32_t m_b;
};

#endif // MAME_EMU_VIDEO_RGBGEN_H
