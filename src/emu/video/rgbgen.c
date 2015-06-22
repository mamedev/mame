// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbgen.c

    General RGB utilities.

***************************************************************************/

#if !defined(__ALTIVEC__)

#include "emu.h"
#include "rgbgen.h"

/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    rgbaint_blend - blend two colors by the given
    scale factor
-------------------------------------------------*/

void rgbaint_t::blend(const rgbaint_t& color2, UINT8 color1scale)
{
	INT32 scale1 = (INT32)color1scale;
	INT32 scale2 = 256 - scale1;

	m_a = (m_a * scale1 + color2.m_a * scale2) >> 8;
	m_r = (m_r * scale1 + color2.m_r * scale2) >> 8;
	m_g = (m_g * scale1 + color2.m_g * scale2) >> 8;
	m_b = (m_b * scale1 + color2.m_b * scale2) >> 8;
}


/*-------------------------------------------------
    rgbaint_scale_and_clamp - scale the given
    color by an 8.8 scale factor, immediate or
    per channel, and clamp to byte values
-------------------------------------------------*/

void rgbaint_t::scale_imm_and_clamp(INT32 scale)
{
	m_a = (m_a * scale) >> 8;
	if (m_a > 255) { m_a = (m_a < 0) ? 0 : 255; }
	m_r = (m_r * scale) >> 8;
	if (m_r > 255) { m_r = (m_r < 0) ? 0 : 255; }
	m_g = (m_g * scale) >> 8;
	if (m_g > 255) { m_g = (m_g < 0) ? 0 : 255; }
	m_b = (m_b * scale) >> 8;
	if (m_b > 255) { m_b = (m_b < 0) ? 0 : 255; }
}

void rgbaint_t::scale_and_clamp(const rgbaint_t& scale)
{
	m_a = (m_a * scale.m_a) >> 8;
	if (m_a > 255) { m_a = (m_a < 0) ? 0 : 255; }
	m_r = (m_r * scale.m_r) >> 8;
	if (m_r > 255) { m_r = (m_r < 0) ? 0 : 255; }
	m_g = (m_g * scale.m_g) >> 8;
	if (m_g > 255) { m_g = (m_g < 0) ? 0 : 255; }
	m_b = (m_b * scale.m_b) >> 8;
	if (m_b > 255) { m_b = (m_b < 0) ? 0 : 255; }
}


void rgbaint_t::scale_imm_add_and_clamp(INT32 scale, const rgbaint_t& other)
{
	m_a = (m_a * scale) >> 8;
	m_a += other.m_a;
	if (m_a > 255) { m_a = (m_a < 0) ? 0 : 255; }
	m_r = (m_r * scale) >> 8;
	m_r += other.m_r;
	if (m_r > 255) { m_r = (m_r < 0) ? 0 : 255; }
	m_g = (m_g * scale) >> 8;
	m_g += other.m_g;
	if (m_g > 255) { m_g = (m_g < 0) ? 0 : 255; }
	m_b = (m_b * scale) >> 8;
	m_b += other.m_b;
	if (m_b > 255) { m_b = (m_b < 0) ? 0 : 255; }
}

void rgbaint_t::scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other)
{
	m_a = (m_a * scale.m_a) >> 8;
	m_a += other.m_a;
	if (m_a > 255) { m_a = (m_a < 0) ? 0 : 255; }
	m_r = (m_r * scale.m_r) >> 8;
	m_r += other.m_r;
	if (m_r > 255) { m_r = (m_r < 0) ? 0 : 255; }
	m_g = (m_g * scale.m_g) >> 8;
	m_g += other.m_g;
	if (m_g > 255) { m_g = (m_g < 0) ? 0 : 255; }
	m_b = (m_b * scale.m_b) >> 8;
	m_b += other.m_b;
	if (m_b > 255) { m_b = (m_b < 0) ? 0 : 255; }
}

void rgbaint_t::scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2)
{
	m_a = (m_a * scale.m_a + other.m_a * scale2.m_a) >> 8;
	if ((UINT16)m_a > 255) { m_a = (m_a < 0) ? 0 : 255; }
	m_r = (m_r * scale.m_r + other.m_r * scale2.m_r) >> 8;
	if ((UINT16)m_r > 255) { m_r = (m_r < 0) ? 0 : 255; }
	m_g = (m_g * scale.m_g + other.m_g * scale2.m_g) >> 8;
	if ((UINT16)m_g > 255) { m_g = (m_g < 0) ? 0 : 255; }
	m_b = (m_b * scale.m_b + other.m_b * scale2.m_b) >> 8;
	if ((UINT16)m_b > 255) { m_b = (m_b < 0) ? 0 : 255; }
}


/*-------------------------------------------------
    bilinear_filter - bilinear filter between
    four pixel values; this code is derived from
    code provided by Michael Herf
-------------------------------------------------*/

UINT32 rgbaint_t::bilinear_filter(UINT32 rgb00, UINT32 rgb01, UINT32 rgb10, UINT32 rgb11, UINT8 u, UINT8 v)
{
	UINT32 ag0, ag1, rb0, rb1;

	rb0 = (rgb00 & 0x00ff00ff) + ((((rgb01 & 0x00ff00ff) - (rgb00 & 0x00ff00ff)) * u) >> 8);
	rb1 = (rgb10 & 0x00ff00ff) + ((((rgb11 & 0x00ff00ff) - (rgb10 & 0x00ff00ff)) * u) >> 8);
	ag0 = (rgb00 & 0xff00ff00) + ((((rgb01 & 0xff00ff00) - (rgb00 & 0xff00ff00)) * u) >> 8);
	ag1 = (rgb10 & 0xff00ff00) + ((((rgb11 & 0xff00ff00) - (rgb10 & 0xff00ff00)) * u) >> 8);

	rb0 = (rb0 & 0x00ff00ff) + ((((rb1 & 0x00ff00ff) - (rb0 & 0x00ff00ff)) * v) >> 8);
	ag0 = (ag0 & 0xff00ff00) + ((((ag1 & 0xff00ff00) - (ag0 & 0xff00ff00)) * v) >> 8);

	return (ag0 & 0xff00ff00) | (rb0 & 0x00ff00ff);
}

#endif // !defined(__ALTIVEC__)
