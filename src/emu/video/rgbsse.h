// license:BSD-3-Clause
// copyright-holders:Vas Crabb
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

class rgbint_t
{
public:
	rgbint_t();
	rgbint_t(UINT32 rgb);
	rgbint_t(INT32 r, INT32 g, INT32 b);
	rgbint_t(rgb_t& rgb);

	void set(void* value);
	__m128i get();
	void set(__m128i value);
	void set_rgb(UINT32 rgb);
	void set_rgb(INT32 r, INT32 g, INT32 b);
	void set_rgb(rgb_t& rgb);

	rgb_t to_rgb();
	rgb_t to_rgb_clamp();

	rgb_t to_rgba();
	rgb_t to_rgba_clamp();

	void add(const rgbint_t& color2);
	void add_imm(const INT32 imm);
	void add_imm_rgb(const INT32 r, const INT32 g, const INT32 b);

	void sub(const rgbint_t& color2);
	void sub_imm(const INT32 imm);
	void sub_imm_rgb(const INT32 r, const INT32 g, const INT32 b);

	void subr(rgbint_t& color);
	void subr_imm(const INT32 imm);
	void subr_imm_rgb(const INT32 r, const INT32 g, const INT32 b);

	void mul(rgbint_t& color);
	void mul_imm(const INT32 imm);
	void mul_imm_rgb(const INT32 r, const INT32 g, const INT32 b);

	void shl(const UINT8 shift);
	void shr(const UINT8 shift);
	void sra(const UINT8 shift);

	void blend(const rgbint_t& other, UINT8 factor);

	void scale_and_clamp(const rgbint_t& scale);
	void scale_imm_and_clamp(const INT16 scale);
	void scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other, const rgbint_t& scale2);
	void scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other);
	void scale_imm_add_and_clamp(const INT16 scale, const rgbint_t& other);

	void sign_extend(const INT32 compare, const INT32 sign);

	void print();

	rgbint_t operator=(const rgbint_t& other);
	rgbint_t& operator+=(const rgbint_t& other);
	rgbint_t& operator-=(const rgbint_t& other);
	rgbint_t& operator*=(const rgbint_t& other);
	rgbint_t& operator*=(const INT32 other);
	rgbint_t operator+(const rgbint_t& other);
	rgbint_t operator-(const rgbint_t& other);
	rgbint_t operator*(const rgbint_t& other);
	rgbint_t operator*(const INT32 other);

	static UINT32 bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v);

protected:
	volatile __m128i	m_value;

private:
	rgbint_t(__m128i value);
};

class rgbaint_t : public rgbint_t
{
public:
	rgbaint_t();
	rgbaint_t(UINT32 rgba);
	rgbaint_t(INT32 a, INT32 r, INT32 g, INT32 b);
	rgbaint_t(rgb_t& rgb);

	void set_rgba(INT32 a, INT32 r, INT32 g, INT32 b);

	void add_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b);
	void sub_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b);
	void subr_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b);
	void mul_imm_rgba(const INT32 a, const INT32 r, const INT32 g, const INT32 b);
};

/***************************************************************************
    TABLES
***************************************************************************/

extern const struct _rgbsse_statics
{
	__m128  dummy_for_alignment;
	INT16   maxbyte[8];
	INT16   scale_table[256][8];
} rgbsse_statics;

#endif /* __RGBSSE__ */
