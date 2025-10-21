// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbgen.cpp

    General RGB utilities.

***************************************************************************/

#include "emu.h"

#if ((defined(MAME_DEBUG) && !defined(__OPTIMIZE__)) || (!defined(__SSE2__) && (!defined(_M_IX86_FP) || (_M_IX86_FP < 2)))) && !defined(__ALTIVEC__)

#include "rgbgen.h"


u32 rgbaint_t::bilinear_filter(u32 rgb00, u32 rgb01, u32 rgb10, u32 rgb11, u8 u, u8 v) noexcept
{
	// interpolate on u axis
	const u32 ag00 = ((rgb00 >> 8) & 0x00ff00ff) * (256U  - u);
	const u32 rb00 = ((rgb00 >> 0) & 0x00ff00ff) * (256U  - u);
	const u32 ag10 = ((rgb10 >> 8) & 0x00ff00ff) * (256U  - u);
	const u32 rb10 = ((rgb10 >> 0) & 0x00ff00ff) * (256U  - u);
	const u32 ag01 = ((rgb01 >> 8) & 0x00ff00ff) * u;
	const u32 rb01 = ((rgb01 >> 0) & 0x00ff00ff) * u;
	const u32 ag11 = ((rgb11 >> 8) & 0x00ff00ff) * u;
	const u32 rb11 = ((rgb11 >> 0) & 0x00ff00ff) * u;
	const u32 ag0x = ((ag00 >> 1) & 0x7fff7fff) + ((ag01 >> 1) & 0x7fff7fff);
	const u32 rb0x = ((rb00 >> 1) & 0x7fff7fff) + ((rb01 >> 1) & 0x7fff7fff);
	const u32 ag1x = ((ag10 >> 1) & 0x7fff7fff) + ((ag11 >> 1) & 0x7fff7fff);
	const u32 rb1x = ((rb10 >> 1) & 0x7fff7fff) + ((rb11 >> 1) & 0x7fff7fff);

	// interpolate on v axis
	const u32 a = (((ag0x >> 16) * (256U - v)) + ((ag1x >> 16) * v)) >> 15;
	const u32 r = (((rb0x >> 16) * (256U - v)) + ((rb1x >> 16) * v)) >> 15;
	const u32 g = (((ag0x & 0xffffU) * (256U - v)) + ((ag1x & 0xffffU) * v)) >> 15;
	const u32 b = (((rb0x & 0xffffU) * (256U - v)) + ((rb1x & 0xffffU) * v)) >> 15;

	return (a << 24) | (r << 16) | (g << 8) | (b << 0);
}

void rgbaint_t::bilinear_filter_rgbaint(u32 rgb00, u32 rgb01, u32 rgb10, u32 rgb11, u8 u, u8 v) noexcept
{
	// interpolate on u axis
	const u32 ag00 = ((rgb00 >> 8) & 0x00ff00ff) * (256U  - u);
	const u32 rb00 = ((rgb00 >> 0) & 0x00ff00ff) * (256U  - u);
	const u32 ag10 = ((rgb10 >> 8) & 0x00ff00ff) * (256U  - u);
	const u32 rb10 = ((rgb10 >> 0) & 0x00ff00ff) * (256U  - u);
	const u32 ag01 = ((rgb01 >> 8) & 0x00ff00ff) * u;
	const u32 rb01 = ((rgb01 >> 0) & 0x00ff00ff) * u;
	const u32 ag11 = ((rgb11 >> 8) & 0x00ff00ff) * u;
	const u32 rb11 = ((rgb11 >> 0) & 0x00ff00ff) * u;
	const u32 ag0x = ((ag00 >> 1) & 0x7fff7fff) + ((ag01 >> 1) & 0x7fff7fff);
	const u32 rb0x = ((rb00 >> 1) & 0x7fff7fff) + ((rb01 >> 1) & 0x7fff7fff);
	const u32 ag1x = ((ag10 >> 1) & 0x7fff7fff) + ((ag11 >> 1) & 0x7fff7fff);
	const u32 rb1x = ((rb10 >> 1) & 0x7fff7fff) + ((rb11 >> 1) & 0x7fff7fff);

	// interpolate on v axis
	m_a = (((ag0x >> 16) * (256U - v)) + ((ag1x >> 16) * v)) >> 15;
	m_r = (((rb0x >> 16) * (256U - v)) + ((rb1x >> 16) * v)) >> 15;
	m_g = (((ag0x & 0xffffU) * (256U - v)) + ((ag1x & 0xffffU) * v)) >> 15;
	m_b = (((rb0x & 0xffffU) * (256U - v)) + ((rb1x & 0xffffU) * v)) >> 15;
}

#endif // ((defined(MAME_DEBUG) && !defined(__OPTIMIZE__)) || (!defined(__SSE2__) && (!defined(_M_IX86_FP) || (_M_IX86_FP < 2)))) && !defined(__ALTIVEC__)
