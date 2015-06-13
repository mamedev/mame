// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rgbutil.h

    Utility definitions for RGB manipulation. Allows RGB handling to be
    performed in an abstracted fashion and optimized with SIMD.

***************************************************************************/

#ifndef __RGBUTIL__
#define __RGBUTIL__

class rgbint_base_t
{
public:
	rgbint_base_t() { }
	rgbint_base_t(UINT32 rgb) { }
	rgbint_base_t(INT16 r, INT16 g, INT16 b) { }
	rgbint_base_t(rgb_t& rgb) { }

	virtual void set_rgb(UINT32 rgb) = 0;
	virtual void set_rgb(INT16 r, INT16 g, INT16 b) = 0;
	virtual void set_rgb(rgb_t& rgb) = 0;

	virtual rgb_t to_rgb() = 0;
	virtual rgb_t to_rgb_clamp() = 0;

	virtual rgb_t to_rgba() = 0;
	virtual rgb_t to_rgba_clamp() = 0;

	virtual void add_imm(const INT16 imm) = 0;
	virtual void add_imm_rgb(const INT16 imm_r, const INT16 imm_g, const INT16 imm_b) = 0;

	virtual void sub_imm(const INT16 imm) = 0;
	virtual void sub_imm_rgb(const INT16 imm_r, const INT16 imm_g, const INT16 imm_b) = 0;

	virtual void subr_imm(const INT16 imm) = 0;
	virtual void subr_imm_rgb(const INT16 imm_r, const INT16 imm_g, const INT16 imm_b) = 0;

	virtual void shl(const UINT8 shift) = 0;
	virtual void shr(const UINT8 shift) = 0;
};

/* use SSE on 64-bit implementations, where it can be assumed */
#if (!defined(MAME_DEBUG) || defined(__OPTIMIZE__)) && (defined(__SSE2__) || defined(_MSC_VER)) && defined(PTR64)
#include "rgbsse.h"
#elif defined(__ALTIVEC__)
#include "rgbvmx.h"
#else
#include "rgbgen.h"
#endif

#endif /* __RGBUTIL__ */
