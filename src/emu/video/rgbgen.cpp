// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbgen.c

    General RGB utilities.

***************************************************************************/

#include "emu.h"


#if !(defined(__ALTIVEC__) || ((!defined(MAME_DEBUG) || defined(__OPTIMIZE__)) && (defined(__SSE2__) || defined(_MSC_VER)) && defined(PTR64)))

#include "rgbgen.h"

/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    rgbaint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

void rgbaint_t::blend(const rgbaint_t& color2, u8 color1scale)
{
	s32 scale1 = s32(color1scale);
	s32 scale2 = 256 - scale1;

	m_a = (m_a * scale1 + color2.m_a * scale2) >> 8;
	m_r = (m_r * scale1 + color2.m_r * scale2) >> 8;
	m_g = (m_g * scale1 + color2.m_g * scale2) >> 8;
	m_b = (m_b * scale1 + color2.m_b * scale2) >> 8;
	m_a |= (m_a & 0x00800000) ? 0xff000000 : 0;
	m_r |= (m_r & 0x00800000) ? 0xff000000 : 0;
	m_g |= (m_g & 0x00800000) ? 0xff000000 : 0;
	m_b |= (m_b & 0x00800000) ? 0xff000000 : 0;
}


/*-------------------------------------------------
    rgbaint_scale_and_clamp - scale the given
    color by an 8.8 scale factor, immediate or
    per channel, and clamp to byte values
-------------------------------------------------*/

void rgbaint_t::scale_imm_and_clamp(s32 scale)
{
	m_a = (m_a * scale) >> 8;
	m_r = (m_r * scale) >> 8;
	m_g = (m_g * scale) >> 8;
	m_b = (m_b * scale) >> 8;
	m_a |= (m_a & 0x00800000) ? 0xff000000 : 0;
	m_r |= (m_r & 0x00800000) ? 0xff000000 : 0;
	m_g |= (m_g & 0x00800000) ? 0xff000000 : 0;
	m_b |= (m_b & 0x00800000) ? 0xff000000 : 0;
	if (u32(m_a) > 255) { m_a = (m_a < 0) ? 0 : 255; }
	if (u32(m_r) > 255) { m_r = (m_r < 0) ? 0 : 255; }
	if (u32(m_g) > 255) { m_g = (m_g < 0) ? 0 : 255; }
	if (u32(m_b) > 255) { m_b = (m_b < 0) ? 0 : 255; }
}

void rgbaint_t::scale_and_clamp(const rgbaint_t& scale)
{
	m_a = (m_a * scale.m_a) >> 8;
	m_r = (m_r * scale.m_r) >> 8;
	m_g = (m_g * scale.m_g) >> 8;
	m_b = (m_b * scale.m_b) >> 8;
	m_a |= (m_a & 0x00800000) ? 0xff000000 : 0;
	m_r |= (m_r & 0x00800000) ? 0xff000000 : 0;
	m_g |= (m_g & 0x00800000) ? 0xff000000 : 0;
	m_b |= (m_b & 0x00800000) ? 0xff000000 : 0;
	if (u32(m_a) > 255) { m_a = (m_a < 0) ? 0 : 255; }
	if (u32(m_r) > 255) { m_r = (m_r < 0) ? 0 : 255; }
	if (u32(m_g) > 255) { m_g = (m_g < 0) ? 0 : 255; }
	if (u32(m_b) > 255) { m_b = (m_b < 0) ? 0 : 255; }
}


void rgbaint_t::scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other)
{
	m_a = (m_a * scale.m_a) >> 8;
	m_r = (m_r * scale.m_r) >> 8;
	m_g = (m_g * scale.m_g) >> 8;
	m_b = (m_b * scale.m_b) >> 8;
	m_a |= (m_a & 0x00800000) ? 0xff000000 : 0;
	m_r |= (m_r & 0x00800000) ? 0xff000000 : 0;
	m_g |= (m_g & 0x00800000) ? 0xff000000 : 0;
	m_b |= (m_b & 0x00800000) ? 0xff000000 : 0;
	m_a += other.m_a;
	m_r += other.m_r;
	m_g += other.m_g;
	m_b += other.m_b;
	if (u32(m_a) > 255) { m_a = (m_a < 0) ? 0 : 255; }
	if (u32(m_r) > 255) { m_r = (m_r < 0) ? 0 : 255; }
	if (u32(m_g) > 255) { m_g = (m_g < 0) ? 0 : 255; }
	if (u32(m_b) > 255) { m_b = (m_b < 0) ? 0 : 255; }
}

void rgbaint_t::scale2_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2)
{
	m_a = (m_a * scale.m_a + other.m_a * scale2.m_a) >> 8;
	m_r = (m_r * scale.m_r + other.m_r * scale2.m_r) >> 8;
	m_g = (m_g * scale.m_g + other.m_g * scale2.m_g) >> 8;
	m_b = (m_b * scale.m_b + other.m_b * scale2.m_b) >> 8;
	m_a |= (m_a & 0x00800000) ? 0xff000000 : 0;
	m_r |= (m_r & 0x00800000) ? 0xff000000 : 0;
	m_g |= (m_g & 0x00800000) ? 0xff000000 : 0;
	m_b |= (m_b & 0x00800000) ? 0xff000000 : 0;
	if (u32(m_a) > 255) { m_a = (m_a < 0) ? 0 : 255; }
	if (u32(m_r) > 255) { m_r = (m_r < 0) ? 0 : 255; }
	if (u32(m_g) > 255) { m_g = (m_g < 0) ? 0 : 255; }
	if (u32(m_b) > 255) { m_b = (m_b < 0) ? 0 : 255; }
}

#endif // !defined(__ALTIVEC__)
