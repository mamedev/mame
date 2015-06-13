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

class rgbint_t : public rgbint_base_t
{
public:
	rgbint_t();
	rgbint_t(UINT32 rgb);
	rgbint_t(INT16 r, INT16 g, INT16 b);
	rgbint_t(rgb_t& rgb);

	virtual void* get_ptr() { return &m_value; }

	virtual void set(__m128i value);
	virtual void set_rgb(UINT32 rgb);
	virtual void set_rgb(INT16 r, INT16 g, INT16 b);
	virtual void set_rgb(rgb_t& rgb);

	virtual rgb_t to_rgb();
	virtual rgb_t to_rgb_clamp();

	virtual rgb_t to_rgba();
	virtual rgb_t to_rgba_clamp();

	void add(const rgbint_t& color2);
	virtual void add_imm(const INT16 imm);
	virtual void add_imm_rgb(const INT16 imm_r, const INT16 imm_g, const INT16 imm_b);

	virtual void sub(const rgbint_t& color2);
	virtual void sub_imm(const INT16 imm);
	virtual void sub_imm_rgb(const INT16 imm_r, const INT16 imm_g, const INT16 imm_b);

	void subr(rgbint_t& color);
	virtual void subr_imm(const INT16 imm);
	virtual void subr_imm_rgb(const INT16 imm_r, const INT16 imm_g, const INT16 imm_b);

	void shl(const UINT8 shift);
	void shr(const UINT8 shift);

	virtual void blend(const rgbint_t& other, UINT8 factor);

	virtual void scale_and_clamp(const rgbint_t& scale);
	virtual void scale_imm_and_clamp(const INT16 scale);
	virtual void scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other, const rgbint_t& scale2);
	virtual void scale_add_and_clamp(const rgbint_t& scale, const rgbint_t& other);
	virtual void scale_imm_add_and_clamp(const INT16 scale, const rgbint_t& other);

	static UINT32 bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v);

protected:
	__m128i	m_value;
};

class rgbaint_t : public rgbint_t
{
public:
	rgbaint_t();
	rgbaint_t(UINT32 rgba);
	rgbaint_t(INT16 a, INT16 r, INT16 g, INT16 b);
	rgbaint_t(rgb_t& rgb);

	virtual void set_rgba(INT16 a, INT16 r, INT16 g, INT16 b);

	virtual void add_imm_rgba(const INT16 imm_a, const INT16 imm_r, const INT16 imm_g, const INT16 imm_b);
	virtual void sub_imm_rgba(const INT16 imm_a, const INT16 imm_r, const INT16 imm_g, const INT16 imm_b);
	virtual void subr_imm_rgba(const INT16 imm_a, const INT16 imm_r, const INT16 imm_g, const INT16 imm_b);
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
