// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,Tyler J. Stachecki
/***************************************************************************

    rspcp2.c

    Universal machine language-based Nintendo/SGI RSP COP2 emulator.
    Written by Ryan Holtz of the MAME team.

***************************************************************************/

#include "emu.h"
#include "rsp.h"
#include "rspcp2.h"

#if USE_SIMD
#include <emmintrin.h>

const rsp_cop2::vec_helpers_t rsp_cop2::m_vec_helpers = {
	{ 0 },
	{ // logic_mask
		{  0,  0,  0,  0,  0,  0,  0,  0 },
		{ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff }
	},
	{ // vrsq_mask_table
		{ 0xffff,  0,  0,  0,  0,  0,  0,  0 },
		{  0, 0xffff,  0,  0,  0,  0,  0,  0 },
		{  0,  0, 0xffff,  0,  0,  0,  0,  0 },
		{  0,  0,  0, 0xffff,  0,  0,  0,  0 },
		{  0,  0,  0,  0, 0xffff,  0,  0,  0 },
		{  0,  0,  0,  0,  0, 0xffff,  0,  0 },
		{  0,  0,  0,  0,  0,  0, 0xffff,  0 },
		{  0,  0,  0,  0,  0,  0,  0, 0xffff }
	},
	{ // shuffle_keys
		{ 0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e }, /* -- */
		{ 0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e }, /* -- */

		{ 0x0100, 0x0100, 0x0504, 0x0504, 0x0908, 0x0908, 0x0d0c, 0x0d0c }, /* 0q */
		{ 0x0302, 0x0302, 0x0706, 0x0706, 0x0b0a, 0x0b0a, 0x0f0e, 0x0f0e }, /* 1q */

		{ 0x0100, 0x0100, 0x0100, 0x0100, 0x0908, 0x0908, 0x0908, 0x0908 }, /* 0h */
		{ 0x0302, 0x0302, 0x0302, 0x0302, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a }, /* 1h */
		{ 0x0504, 0x0504, 0x0504, 0x0504, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c }, /* 2h */
		{ 0x0706, 0x0706, 0x0706, 0x0706, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e }, /* 3h */

		{ 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100 }, /* 0w */
		{ 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302 }, /* 1w */
		{ 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504 }, /* 2w */
		{ 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706 }, /* 3w */
		{ 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908 }, /* 4w */
		{ 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a }, /* 5w */
		{ 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c }, /* 6w */
		{ 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e }  /* 7w */
	},
	{ // sll_b2l_keys
		{ 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c },
		{ 0x8003, 0x0201, 0x0007, 0x0605, 0x040b, 0x0a09, 0x080f, 0x0e0d },
		{ 0x8080, 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e },
		{ 0x8080, 0x8003, 0x0201, 0x0007, 0x0605, 0x040b, 0x0a09, 0x080f },

		{ 0x8080, 0x8080, 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908 },
		{ 0x8080, 0x8080, 0x8003, 0x0201, 0x0007, 0x0605, 0x040b, 0x0a09 },
		{ 0x8080, 0x8080, 0x8080, 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a },
		{ 0x8080, 0x8080, 0x8080, 0x8003, 0x0201, 0x0007, 0x0605, 0x040b },

		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x0302, 0x0100, 0x0706, 0x0504 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8003, 0x0201, 0x0007, 0x0605 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0302, 0x0100, 0x0706 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8003, 0x0201, 0x0007 },

		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0302, 0x0100 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8003, 0x0201 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0302 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8003 },
	},
	{ // sll_l2b_keys
		{ 0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e },
		{ 0x0201, 0x8003, 0x0605, 0x0007, 0x0a09, 0x040b, 0x0e0d, 0x080f },
		{ 0x0302, 0x8080, 0x0706, 0x0100, 0x0b0a, 0x0504, 0x0f0e, 0x0908 },
		{ 0x8003, 0x8080, 0x0007, 0x0201, 0x040b, 0x0605, 0x080f, 0x0a09 },

		{ 0x8080, 0x8080, 0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a },
		{ 0x8080, 0x8080, 0x0201, 0x8003, 0x0605, 0x0007, 0x0a09, 0x040b },
		{ 0x8080, 0x8080, 0x0302, 0x8080, 0x0706, 0x0100, 0x0b0a, 0x0504 },
		{ 0x8080, 0x8080, 0x8003, 0x8080, 0x0007, 0x0201, 0x040b, 0x0605 },

		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x0100, 0x0302, 0x0504, 0x0706 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x0201, 0x8003, 0x0605, 0x0007 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x0302, 0x8080, 0x0706, 0x0100 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8003, 0x8080, 0x0007, 0x0201 },

		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0100, 0x0302 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0201, 0x8003 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x0302, 0x8080 },
		{ 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8003, 0x8080 },
	},
	{ // srl_b2l_keys
		{ 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c },
		{ 0x0201, 0x0007, 0x0605, 0x040b, 0x0a09, 0x080f, 0x0e0d, 0x0c80 },
		{ 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x8080 },
		{ 0x0007, 0x0605, 0x040b, 0x0a09, 0x080f, 0x0e0d, 0x0c80, 0x8080 },

		{ 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x8080, 0x8080 },
		{ 0x0605, 0x040b, 0x0a09, 0x080f, 0x0e0d, 0x0c80, 0x8080, 0x8080 },
		{ 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x8080, 0x8080, 0x8080 },
		{ 0x040b, 0x0a09, 0x080f, 0x0e0d, 0x0c80, 0x8080, 0x8080, 0x8080 },

		{ 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x8080, 0x8080, 0x8080, 0x8080 },
		{ 0x0a09, 0x080f, 0x0e0d, 0x0c80, 0x8080, 0x8080, 0x8080, 0x8080 },
		{ 0x0908, 0x0f0e, 0x0d0c, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080 },
		{ 0x080f, 0x0e0d, 0x0c80, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080 },

		{ 0x0f0e, 0x0d0c, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080 },
		{ 0x0e0d, 0x0c80, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080 },
		{ 0x0d0c, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080 },
		{ 0x0c80, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080 },
	},
	{ // ror_b2l_keys
		{ 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c },
		{ 0x0201, 0x0007, 0x0605, 0x040b, 0x0a09, 0x080f, 0x0e0d, 0x0c03 },
		{ 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x0302 },
		{ 0x0007, 0x0605, 0x040b, 0x0a09, 0x080f, 0x0e0d, 0x0c03, 0x0201 },

		{ 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x0302, 0x0100 },
		{ 0x0605, 0x040b, 0x0a09, 0x080f, 0x0e0d, 0x0c03, 0x0201, 0x0007 },
		{ 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x0302, 0x0100, 0x0706 },
		{ 0x040b, 0x0a09, 0x080f, 0x0e0d, 0x0c03, 0x0201, 0x0007, 0x0605 },

		{ 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x0302, 0x0100, 0x0706, 0x0504 },
		{ 0x0a09, 0x080f, 0x0e0d, 0x0c03, 0x0201, 0x0007, 0x0605, 0x040b },
		{ 0x0908, 0x0f0e, 0x0d0c, 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a },
		{ 0x080f, 0x0e0d, 0x0c03, 0x0201, 0x0007, 0x0605, 0x040b, 0x0a09 },

		{ 0x0f0e, 0x0d0c, 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908 },
		{ 0x0e0d, 0x0c03, 0x0201, 0x0007, 0x0605, 0x040b, 0x0a09, 0x080f },
		{ 0x0d0c, 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e },
		{ 0x0c03, 0x0201, 0x0007, 0x0605, 0x040b, 0x0a09, 0x080f, 0x0e0d },
	},
	{ // rol_l2b_keys
		{ 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c },
		{ 0x0003, 0x0e01, 0x0407, 0x0205, 0x080b, 0x0609, 0x0c0f, 0x0a0d },
		{ 0x0100, 0x0f0e, 0x0504, 0x0302, 0x0908, 0x0706, 0x0d0c, 0x0b0a },
		{ 0x0e01, 0x0c0f, 0x0205, 0x0003, 0x0609, 0x0407, 0x0a0d, 0x080b },

		{ 0x0f0e, 0x0d0c, 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908 },
		{ 0x0c0f, 0x0a0d, 0x0003, 0x0e01, 0x0407, 0x0205, 0x080b, 0x0609 },
		{ 0x0d0c, 0x0b0a, 0x0100, 0x0f0e, 0x0504, 0x0302, 0x0908, 0x0706 },
		{ 0x0a0d, 0x080b, 0x0e01, 0x0c0f, 0x0205, 0x0003, 0x0609, 0x0407 },

		{ 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x0302, 0x0100, 0x0706, 0x0504 },
		{ 0x080b, 0x0609, 0x0c0f, 0x0a0d, 0x0003, 0x0e01, 0x0407, 0x0205 },
		{ 0x0908, 0x0706, 0x0d0c, 0x0b0a, 0x0100, 0x0f0e, 0x0504, 0x0302 },
		{ 0x0609, 0x0407, 0x0a0d, 0x080b, 0x0e01, 0x0c0f, 0x0205, 0x0003 },

		{ 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x0302, 0x0100 },
		{ 0x0407, 0x0205, 0x080b, 0x0609, 0x0c0f, 0x0a0d, 0x0003, 0x0e01 },
		{ 0x0504, 0x0302, 0x0908, 0x0706, 0x0d0c, 0x0b0a, 0x0100, 0x0f0e },
		{ 0x0205, 0x0003, 0x0609, 0x0407, 0x0a0d, 0x080b, 0x0e01, 0x0c0f },
	},
	{ // ror_l2b_keys
		{ 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c },
		{ 0x0205, 0x0003, 0x0609, 0x0407, 0x0a0d, 0x080b, 0x0e01, 0x0c0f },
		{ 0x0504, 0x0302, 0x0908, 0x0706, 0x0d0c, 0x0b0a, 0x0100, 0x0f0e },
		{ 0x0407, 0x0205, 0x080b, 0x0609, 0x0c0f, 0x0a0d, 0x0003, 0x0e01 },

		{ 0x0706, 0x0504, 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x0302, 0x0100 },
		{ 0x0609, 0x0407, 0x0a0d, 0x080b, 0x0e01, 0x0c0f, 0x0205, 0x0003 },
		{ 0x0908, 0x0706, 0x0d0c, 0x0b0a, 0x0100, 0x0f0e, 0x0504, 0x0302 },
		{ 0x080b, 0x0609, 0x0c0f, 0x0a0d, 0x0003, 0x0e01, 0x0407, 0x0205 },

		{ 0x0b0a, 0x0908, 0x0f0e, 0x0d0c, 0x0302, 0x0100, 0x0706, 0x0504 },
		{ 0x0a0d, 0x080b, 0x0e01, 0x0c0f, 0x0205, 0x0003, 0x0609, 0x0407 },
		{ 0x0d0c, 0x0b0a, 0x0100, 0x0f0e, 0x0504, 0x0302, 0x0908, 0x0706 },
		{ 0x0c0f, 0x0a0d, 0x0003, 0x0e01, 0x0407, 0x0205, 0x080b, 0x0609 },

		{ 0x0f0e, 0x0d0c, 0x0302, 0x0100, 0x0706, 0x0504, 0x0b0a, 0x0908 },
		{ 0x0e01, 0x0c0f, 0x0205, 0x0003, 0x0609, 0x0407, 0x0a0d, 0x080b },
		{ 0x0100, 0x0f0e, 0x0504, 0x0302, 0x0908, 0x0706, 0x0d0c, 0x0b0a },
		{ 0x0003, 0x0e01, 0x0407, 0x0205, 0x080b, 0x0609, 0x0c0f, 0x0a0d },
	},
	{ // qr_lut
		{ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff },
		{ 0xffff, 0xff00, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff },
		{ 0xffff, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff },
		{ 0xff00, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff },

		{ 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff},
		{ 0x0000, 0x0000, 0xffff, 0xff00, 0xffff, 0xffff, 0xffff, 0xffff },
		{ 0x0000, 0x0000, 0xffff, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff },
		{ 0x0000, 0x0000, 0xff00, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff },

		{ 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff },
		{ 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xff00, 0xffff, 0xffff },
		{ 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0x0000, 0xffff, 0xffff },
		{ 0x0000, 0x0000, 0x0000, 0x0000, 0xff00, 0x0000, 0xffff, 0xffff },

		{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff },
		{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xff00 },
		{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0x0000 },
		{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xff00, 0x0000 }
	},
	{ // bdls_lut - mask to denote which part of the vector to load/store.
		{ 0x0000, 0xff00, 0x0000, 0x0000 }, // B
		{ 0x0000, 0xffff, 0x0000, 0x0000 }, // S
		{ 0xffff, 0xffff, 0x0000, 0x0000 }, // L
		{ 0xffff, 0xffff, 0xffff, 0xffff }  // D
	},
	{ // word_reverse
		0x0203, 0x0001, 0x0607, 0x0405, 0x0a0b, 0x0809, 0x0e0f, 0x0c0d
	}
};

#if !(defined(__SSSE3__) || defined(_MSC_VER))
// TODO: Highly optimized. More of a stopgap measure.
static inline rsp_vec_t sse2_pshufb(rsp_vec_t v, const UINT16 *keys)
{
	UINT8 dest[16];
	UINT8 temp[16];

	_mm_storeu_si128((rsp_vec_t *) temp, v);

	for (UINT32 j = 0; j < 8; j++)
	{
		UINT16 key = keys[j];
		UINT8 key_hi = key >> 8;
		UINT8 key_lo = key >> 0;

		dest[(j << 1) + 1] = key_hi == 0x80 ? 0x00 : temp[key_hi];
		dest[(j << 1) + 0] = key_lo == 0x80 ? 0x00 : temp[key_lo];
	}

	return _mm_loadu_si128((rsp_vec_t *) dest);
}

rsp_vec_t rsp_cop2::vec_load_and_shuffle_operand(const UINT16* src, UINT32 element)
{
	if (element >= 8) // element => 0w ... 7w
	{
		UINT16 word_lo;

		memcpy(&word_lo, src + (element - 8), sizeof(word_lo));
		UINT64 dword = word_lo | ((UINT32) word_lo << 16);

		return _mm_shuffle_epi32(_mm_loadl_epi64((rsp_vec_t*) &dword), _MM_SHUFFLE(0,0,0,0));
	}
	else if (element >= 4) // element => 0h ... 3h
	{
		UINT16 word_lo;
		UINT16 word_hi;

		memcpy(&word_hi, src + element - 0, sizeof(word_hi));
		memcpy(&word_lo, src + element - 4, sizeof(word_lo));
		UINT64 dword = word_lo | ((UINT32) word_hi << 16);

		rsp_vec_t v = _mm_loadl_epi64((rsp_vec_t*) &dword);
		v = _mm_shufflelo_epi16(v, _MM_SHUFFLE(1,1,0,0));
		return _mm_shuffle_epi32(v, _MM_SHUFFLE(1,1,0,0));
	}
	else if (element >= 2) // element => 0q ... 1q
	{
		rsp_vec_t v = vec_load_unshuffled_operand(src);

		if (element == 2) {
			v = _mm_shufflelo_epi16(v, _MM_SHUFFLE(3,3,1,1));
			v = _mm_shufflehi_epi16(v, _MM_SHUFFLE(3,3,1,1));
		}
		else
		{
			v = _mm_shufflelo_epi16(v, _MM_SHUFFLE(2,2,0,0));
			v = _mm_shufflehi_epi16(v, _MM_SHUFFLE(2,2,0,0));
		}

		return v;
	}

	return vec_load_unshuffled_operand(src);
}
#else
rsp_vec_t rsp_cop2::vec_load_and_shuffle_operand(const UINT16* src, UINT32 element)
{
	rsp_vec_t operand = _mm_load_si128((rsp_vec_t*) src);
	rsp_vec_t key = _mm_load_si128((rsp_vec_t*) m_vec_helpers.shuffle_keys[element]);

	return _mm_shuffle_epi8(operand, key);
}
#endif
//
// SSSE3+ accelerated loads for group I. Byteswap big-endian to 2-byte
// little-endian vector. Start at vector element offset, discarding any
// wraparound as necessary.
//
// TODO: Reverse-engineer what happens when loads to vector elements must
//       wraparound. Do we just discard the data, as below, or does the
//       data effectively get rotated around the edge of the vector?
//
void rsp_cop2::vec_load_group1(UINT32 addr, UINT32 element, UINT16 *regp, rsp_vec_t reg, rsp_vec_t dqm)
{
	UINT32 offset = addr & 0x7;
	UINT32 ror = offset - element;

	// Always load in 8-byte chunks to emulate wraparound.
	rsp_vec_t data;
	if (offset) {
		UINT32 aligned_addr_lo = addr & ~0x7;
		UINT32 aligned_addr_hi = (aligned_addr_lo + 8) & 0xFFF;

		data = _mm_loadl_epi64((rsp_vec_t *) (m_rsp.get_dmem() + aligned_addr_lo));
		rsp_vec_t temp = _mm_loadl_epi64((rsp_vec_t *) (m_rsp.get_dmem() + aligned_addr_hi));
		data = _mm_unpacklo_epi64(data, temp);
	}
	else
	{
		data = _mm_loadl_epi64((rsp_vec_t *) (m_rsp.get_dmem() + addr));
	}

	// Shift the DQM up to the point where we mux in the data.
#if !(defined(__SSSE3__) || defined(_MSC_VER))
	dqm = sse2_pshufb(dqm, m_vec_helpers.sll_b2l_keys[element]);
#else
	rsp_vec_t ekey = _mm_load_si128((rsp_vec_t *) (m_vec_helpers.sll_b2l_keys[element]));
	dqm = _mm_shuffle_epi8(dqm, ekey);
#endif

	// Align the data to the DQM so we can mask it in.
#if !(defined(__SSSE3__) || defined(_MSC_VER))
	data = sse2_pshufb(data, m_vec_helpers.ror_b2l_keys[ror & 0xF]);
#else
	ekey = _mm_load_si128((rsp_vec_t *) (m_vec_helpers.ror_b2l_keys[ror & 0xF]));
	data = _mm_shuffle_epi8(data, ekey);
#endif

	// Mask and mux in the data.
#if (defined(__SSE4_1__) || defined(_MSC_VER))
	reg = _mm_blendv_epi8(reg, data, dqm);
#else
	data = _mm_and_si128(dqm, data);
	reg = _mm_andnot_si128(dqm, reg);
	reg = _mm_or_si128(data, reg);
#endif

	_mm_store_si128((rsp_vec_t *) regp, reg);
}

//
// SSSE3+ accelerated loads for group II.
//
// TODO: Reverse-engineer what happens when loads to vector elements must
//       wraparound. Do we just discard the data, as below, or does the
//       data effectively get rotated around the edge of the vector?
//
// TODO: Reverse-engineer what happens when element != 0.
//
void rsp_cop2::vec_load_group2(UINT32 addr, UINT32 element, UINT16 *regp, rsp_vec_t reg, rsp_vec_t dqm, rsp_mem_request_type request_type) {
	UINT32 offset = addr & 0x7;
	rsp_vec_t data;

	// Always load in 8-byte chunks to emulate wraparound.
	if (offset) {
		UINT32 aligned_addr_lo = addr & ~0x7;
		UINT32 aligned_addr_hi = (aligned_addr_lo + 8) & 0xFFF;
		UINT64 datalow, datahigh;

		memcpy(&datalow, m_rsp.get_dmem() + aligned_addr_lo, sizeof(datalow));
		memcpy(&datahigh, m_rsp.get_dmem() + aligned_addr_hi, sizeof(datahigh));

		// TODO: Test for endian issues?
		datahigh >>= ((8 - offset) << 3);
		datalow <<= (offset << 3);
		datalow = datahigh | datalow;

		data = _mm_loadl_epi64((rsp_vec_t *) &datalow);
	}
	else
	{
		data = _mm_loadl_epi64((rsp_vec_t *) (m_rsp.get_dmem() + addr));
	}

	// "Unpack" the data.
	rsp_vec_t zero = _mm_setzero_si128();
	data = _mm_unpacklo_epi8(zero, data);

	if (request_type != RSP_MEM_REQUEST_PACK)
	{
		data = _mm_srli_epi16(data, 1);
	}

	data = _mm_shufflehi_epi16(data, _MM_SHUFFLE(0, 1, 2, 3));
	data = _mm_shufflelo_epi16(data, _MM_SHUFFLE(0, 1, 2, 3));

	_mm_store_si128((rsp_vec_t *) regp, data);
}

//
// SSSE3+ accelerated loads for group IV. Byteswap big-endian to 2-byte
// little-endian vector. Stop loading at quadword boundaries.
//
// TODO: Reverse-engineer what happens when loads from vector elements
//       must wraparound (i.e., the address offset is small, starting
//       element is large).
//
void rsp_cop2::vec_load_group4(UINT32 addr, UINT32 element, UINT16 *regp, rsp_vec_t reg, rsp_vec_t dqm, rsp_mem_request_type request_type)
{
	UINT32 aligned_addr = addr & 0xFF0;
	UINT32 offset = addr & 0xF;
	static UINT32 call_count = 0;

	rsp_vec_t data = _mm_load_si128((rsp_vec_t *) (m_rsp.get_dmem() + aligned_addr));

	UINT32 ror;
	if (request_type == RSP_MEM_REQUEST_QUAD)
	{
		ror = 16 - element + offset;
	}
	else
	{
		// TODO: How is this adjusted for LRV when e != 0?
		dqm = _mm_cmpeq_epi8(_mm_setzero_si128(), dqm);
		ror = 16 - offset;
	}

#if !(defined(__SSSE3__) || defined(_MSC_VER))
	data = sse2_pshufb(data, m_vec_helpers.ror_b2l_keys[ror & 0xF]);
	dqm = sse2_pshufb(dqm, m_vec_helpers.ror_b2l_keys[ror & 0xF]);
#else
	rsp_vec_t dkey = _mm_load_si128((rsp_vec_t *) (m_vec_helpers.ror_b2l_keys[ror & 0xF]));
	data = _mm_shuffle_epi8(data, dkey);
	dqm = _mm_shuffle_epi8(dqm, dkey);
#endif

	// Mask and mux in the data.
#if (defined(__SSE4_1__) || defined(_MSC_VER))
	data = _mm_blendv_epi8(reg, data, dqm);
#else
	data = _mm_and_si128(dqm, data);
	reg = _mm_andnot_si128(dqm, reg);
	data = _mm_or_si128(data, reg);
#endif

	_mm_store_si128((rsp_vec_t *) regp, data);

	call_count++;
}

//
// SSE3+ accelerated stores for group I. Byteswap 2-byte little-endian
// vector back to big-endian. Start at vector element offset, wrapping
// around the edge of the vector as necessary.
//
// TODO: Reverse-engineer what happens when stores from vector elements
//       must wraparound. Do we just stop storing the data, or do we
//       continue storing from the front of the vector, as below?
//
void rsp_cop2::vec_store_group1(UINT32 addr, UINT32 element, UINT16 *regp, rsp_vec_t reg, rsp_vec_t dqm)
{
	UINT32 offset = addr & 0x7;
	UINT32 ror = element - offset;

	// Shift the DQM up to the point where we mux in the data.
#if !(defined(__SSSE3__) || defined(_MSC_VER))
	dqm = sse2_pshufb(dqm, m_vec_helpers.sll_l2b_keys[offset]);
#else
	rsp_vec_t ekey = _mm_load_si128((rsp_vec_t *) (m_vec_helpers.sll_l2b_keys[offset]));
	dqm = _mm_shuffle_epi8(dqm, ekey);
#endif

	// Rotate the reg to align with the DQM.
#if !(defined(__SSSE3__) || defined(_MSC_VER))
	reg = sse2_pshufb(reg, m_vec_helpers.ror_l2b_keys[ror & 0xF]);
#else
	ekey = _mm_load_si128((rsp_vec_t *) (m_vec_helpers.ror_l2b_keys[ror & 0xF]));
	reg = _mm_shuffle_epi8(reg, ekey);
#endif

	// Always load in 8-byte chunks to emulate wraparound.
	rsp_vec_t data;
	if (offset)
	{
		UINT32 aligned_addr_lo = addr & ~0x7;
		UINT32 aligned_addr_hi = (aligned_addr_lo + 8) & 0xFFF;

		data = _mm_loadl_epi64((rsp_vec_t *) (m_rsp.get_dmem() + aligned_addr_lo));
		rsp_vec_t temp = _mm_loadl_epi64((rsp_vec_t *) (m_rsp.get_dmem() + aligned_addr_hi));
		data = _mm_unpacklo_epi64(data, temp);

		// Mask and mux in the data.
#if (defined(__SSE4_1__) || defined(_MSC_VER))
		data = _mm_blendv_epi8(data, reg, dqm);
#else
		data = _mm_andnot_si128(dqm, data);
		reg = _mm_and_si128(dqm, reg);
		data = _mm_or_si128(data, reg);
#endif

		_mm_storel_epi64((rsp_vec_t *) (m_rsp.get_dmem() + aligned_addr_lo), data);

		data = _mm_srli_si128(data, 8);
		_mm_storel_epi64((rsp_vec_t *) (m_rsp.get_dmem() + aligned_addr_hi), data);
	}
	else
	{
		data = _mm_loadl_epi64((rsp_vec_t *) (m_rsp.get_dmem() + addr));

		// Mask and mux in the data.
#if (defined(__SSE4_1__) || defined(_MSC_VER))
		data = _mm_blendv_epi8(data, reg, dqm);
#else
		data = _mm_andnot_si128(dqm, data);
		reg = _mm_and_si128(dqm, reg);
		data = _mm_or_si128(data, reg);
#endif

		_mm_storel_epi64((rsp_vec_t *) (m_rsp.get_dmem() + addr), data);
	}
}

//
// SSE3+ accelerated stores for group II. Byteswap 2-byte little-endian
// vector back to big-endian. Start at vector element offset, wrapping
// around the edge of the vector as necessary.
//
// TODO: Reverse-engineer what happens when stores from vector elements
//       must wraparound. Do we just stop storing the data, or do we
//       continue storing from the front of the vector, as below?
//
// TODO: Reverse-engineer what happens when element != 0.
//
void rsp_cop2::vec_store_group2(UINT32 addr, UINT32 element, UINT16 *regp, rsp_vec_t reg, rsp_vec_t dqm, rsp_mem_request_type request_type) {
	// "Pack" the data.
	if (request_type != RSP_MEM_REQUEST_PACK)
	{
		reg = _mm_slli_epi16(reg, 1);
	}

	reg = _mm_srai_epi16(reg, 8);
	reg = _mm_packs_epi16(reg, reg);

#if !(defined(__SSSE3__) || defined(_MSC_VER))
	reg = sse2_pshufb(reg, m_vec_helpers.word_reverse);
#else
	rsp_vec_t dkey = _mm_load_si128((rsp_vec_t *) (m_vec_helpers.word_reverse));
	reg = _mm_shuffle_epi8(reg, dkey);
#endif

	// TODO: Always store in 8-byte chunks to emulate wraparound.
	_mm_storel_epi64((rsp_vec_t *) (m_rsp.get_dmem() + addr), reg);
}

//
// SSE3+ accelerated stores for group IV. Byteswap 2-byte little-endian
// vector back to big-endian. Stop storing at quadword boundaries.
//
void rsp_cop2::vec_store_group4(UINT32 addr, UINT32 element, UINT16 *regp, rsp_vec_t reg, rsp_vec_t dqm, rsp_mem_request_type request_type) {
	UINT32 aligned_addr = addr & 0xFF0;
	UINT32 offset = addr & 0xF;
	UINT32 rol = offset;

	rsp_vec_t data = _mm_load_si128((rsp_vec_t *) (m_rsp.get_dmem() + aligned_addr));

	if (request_type == RSP_MEM_REQUEST_QUAD)
	{
		rol -= element;
	}
	else
	{
		// TODO: How is this adjusted for SRV when e != 0?
		dqm = _mm_cmpeq_epi8(_mm_setzero_si128(), dqm);
	}

#if !(defined(__SSSE3__) || defined(_MSC_VER))
	reg = sse2_pshufb(reg, m_vec_helpers.rol_l2b_keys[rol & 0xF]);
#else
	rsp_vec_t ekey = _mm_load_si128((rsp_vec_t *) (m_vec_helpers.rol_l2b_keys[rol & 0xF]));
	reg = _mm_shuffle_epi8(reg, ekey);
#endif

	// Mask and mux out the data, write.
#if (defined(__SSE4_1__) || defined(_MSC_VER))
	data = _mm_blendv_epi8(data, reg, dqm);
#else
	reg = _mm_and_si128(dqm, reg);
	data = _mm_andnot_si128(dqm, data);
	data = _mm_or_si128(data, reg);
#endif

	_mm_store_si128((rsp_vec_t *) (m_rsp.get_dmem() + aligned_addr), data);
}
#endif

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);

/***************************************************************************
    Helpful Defines
***************************************************************************/

#define VDREG   ((op >> 6) & 0x1f)
#define VS1REG  ((op >> 11) & 0x1f)
#define VS2REG  ((op >> 16) & 0x1f)
#define EL      ((op >> 21) & 0xf)

#define RSVAL   (m_rsp.m_rsp_state->r[RSREG])
#define RTVAL   (m_rsp.m_rsp_state->r[RTREG])
#define RDVAL   (m_rsp.m_rsp_state->r[RDREG])

#define VREG_B(reg, offset)     m_v[(reg)].b[(offset)^1]
#define VREG_S(reg, offset)     m_v[(reg)].s[(offset)]
#define VREG_L(reg, offset)     m_v[(reg)].l[(offset)]

#define R_VREG_B(reg, offset)       m_v[(reg)].b[(offset)^1]
#define R_VREG_S(reg, offset)       (INT16)m_v[(reg)].s[(offset)]
#define R_VREG_L(reg, offset)       m_v[(reg)].l[(offset)]

#define W_VREG_B(reg, offset, val)  (m_v[(reg)].b[(offset)^1] = val)
#define W_VREG_S(reg, offset, val)  (m_v[(reg)].s[(offset)] = val)
#define W_VREG_L(reg, offset, val)  (m_v[(reg)].l[(offset)] = val)

#define VEC_EL_2(x,z)               (vector_elements_2[(x)][(z)])

#define CARRY       0
#define COMPARE     1
#define CLIP1       2
#define ZERO        3
#define CLIP2       4

#define ACCUM(x)            m_accum[x].q
#define ACCUM_H(x)          (UINT16)m_accum[x].w[3]
#define ACCUM_M(x)          (UINT16)m_accum[x].w[2]
#define ACCUM_L(x)          (UINT16)m_accum[x].w[1]
#define ACCUM_LL(x)         (UINT16)m_accum[x].w[0]

#define SET_ACCUM_H(v, x)       m_accum[x].w[3] = v;
#define SET_ACCUM_M(v, x)       m_accum[x].w[2] = v;
#define SET_ACCUM_L(v, x)       m_accum[x].w[1] = v;
#define SET_ACCUM_LL(v, x)      m_accum[x].w[0] = v;

#define CARRY_FLAG(x)          (m_vflag[CARRY][x & 7] != 0 ? 0xffff : 0)
#define COMPARE_FLAG(x)        (m_vflag[COMPARE][x & 7] != 0 ? 0xffff : 0)
#define CLIP1_FLAG(x)          (m_vflag[CLIP1][x & 7] != 0 ? 0xffff : 0)
#define ZERO_FLAG(x)           (m_vflag[ZERO][x & 7] != 0 ? 0xffff : 0)
#define CLIP2_FLAG(x)          (m_vflag[CLIP2][x & 7] != 0 ? 0xffff : 0)

#define CLEAR_CARRY_FLAGS()         { memset(m_vflag[CARRY], 0, 16); }
#define CLEAR_COMPARE_FLAGS()       { memset(m_vflag[COMPARE], 0, 16); }
#define CLEAR_CLIP1_FLAGS()         { memset(m_vflag[CLIP1], 0, 16); }
#define CLEAR_ZERO_FLAGS()          { memset(m_vflag[ZERO], 0, 16); }
#define CLEAR_CLIP2_FLAGS()         { memset(m_vflag[CLIP2], 0, 16); }

#define SET_CARRY_FLAG(x)           { m_vflag[CARRY][x & 7] = 0xffff; }
#define SET_COMPARE_FLAG(x)         { m_vflag[COMPARE][x & 7] = 0xffff; }
#define SET_CLIP1_FLAG(x)           { m_vflag[CLIP1][x & 7] = 0xffff; }
#define SET_ZERO_FLAG(x)            { m_vflag[ZERO][x & 7] = 0xffff; }
#define SET_CLIP2_FLAG(x)           { m_vflag[CLIP2][x & 7] = 0xffff; }

#define CLEAR_CARRY_FLAG(x)         { m_vflag[CARRY][x & 7] = 0; }
#define CLEAR_COMPARE_FLAG(x)       { m_vflag[COMPARE][x & 7] = 0; }
#define CLEAR_CLIP1_FLAG(x)         { m_vflag[CLIP1][x & 7] = 0; }
#define CLEAR_ZERO_FLAG(x)          { m_vflag[ZERO][x & 7] = 0; }
#define CLEAR_CLIP2_FLAG(x)         { m_vflag[CLIP2][x & 7] = 0; }

#define WRITEBACK_RESULT() { \
		VREG_S(VDREG, 0) = m_vres[0];   \
		VREG_S(VDREG, 1) = m_vres[1];   \
		VREG_S(VDREG, 2) = m_vres[2];   \
		VREG_S(VDREG, 3) = m_vres[3];   \
		VREG_S(VDREG, 4) = m_vres[4];   \
		VREG_S(VDREG, 5) = m_vres[5];   \
		VREG_S(VDREG, 6) = m_vres[6];   \
		VREG_S(VDREG, 7) = m_vres[7];   \
}

#if !USE_SIMD
static const int vector_elements_2[16][8] =
{
	{ 0, 1, 2, 3, 4, 5, 6, 7 },     // none
	{ 0, 1, 2, 3, 4, 5, 6, 7 },     // ???
	{ 0, 0, 2, 2, 4, 4, 6, 6 },     // 0q
	{ 1, 1, 3, 3, 5, 5, 7, 7 },     // 1q
	{ 0, 0, 0, 0, 4, 4, 4, 4 },     // 0h
	{ 1, 1, 1, 1, 5, 5, 5, 5 },     // 1h
	{ 2, 2, 2, 2, 6, 6, 6, 6 },     // 2h
	{ 3, 3, 3, 3, 7, 7, 7, 7 },     // 3h
	{ 0, 0, 0, 0, 0, 0, 0, 0 },     // 0
	{ 1, 1, 1, 1, 1, 1, 1, 1 },     // 1
	{ 2, 2, 2, 2, 2, 2, 2, 2 },     // 2
	{ 3, 3, 3, 3, 3, 3, 3, 3 },     // 3
	{ 4, 4, 4, 4, 4, 4, 4, 4 },     // 4
	{ 5, 5, 5, 5, 5, 5, 5, 5 },     // 5
	{ 6, 6, 6, 6, 6, 6, 6, 6 },     // 6
	{ 7, 7, 7, 7, 7, 7, 7, 7 },     // 7
};
#endif

rsp_cop2::rsp_cop2(rsp_device &rsp, running_machine &machine)
	: m_rsp(rsp)
	, m_machine(machine)
	, m_reciprocal_res(0)
	, m_reciprocal_high(0)
	, m_dp_allowed(0)
{
	memset(m_vres, 0, sizeof(m_vres));
	memset(m_v, 0, sizeof(m_v));
	memset(m_vflag, 0, sizeof(m_vflag));
	memset(m_accum, 0, sizeof(m_accum));
#if USE_SIMD
	memset(&m_acc, 0, sizeof(m_acc));
	memset(&m_flags, 0, sizeof(aligned_rsp_2vect_t) * 3);
	m_div_out = 0;
	m_div_in = 0;
#endif
	m_rspcop2_state = (internal_rspcop2_state *)rsp.m_cache.alloc_near(sizeof(internal_rspcop2_state));
}

rsp_cop2::~rsp_cop2()
{
}

void rsp_cop2::init()
{
	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();
}

void rsp_cop2::start()
{
	for(int regIdx = 0; regIdx < 32; regIdx++ )
	{
		m_v[regIdx].d[0] = 0;
		m_v[regIdx].d[1] = 0;
	}

	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();
	m_reciprocal_res = 0;
	m_reciprocal_high = 0;

	// Accumulators do not power on to a random state
	for(int accumIdx = 0; accumIdx < 8; accumIdx++ )
	{
		m_accum[accumIdx].q = 0;
	}
}

void rsp_cop2::state_string_export(const int index, std::string &str)
{
	switch (index)
	{
		case RSP_V0:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 0, 0), (UINT16)VREG_S( 0, 1), (UINT16)VREG_S( 0, 2), (UINT16)VREG_S( 0, 3), (UINT16)VREG_S( 0, 4), (UINT16)VREG_S( 0, 5), (UINT16)VREG_S( 0, 6), (UINT16)VREG_S( 0, 7));
			break;
		case RSP_V1:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 1, 0), (UINT16)VREG_S( 1, 1), (UINT16)VREG_S( 1, 2), (UINT16)VREG_S( 1, 3), (UINT16)VREG_S( 1, 4), (UINT16)VREG_S( 1, 5), (UINT16)VREG_S( 1, 6), (UINT16)VREG_S( 1, 7));
			break;
		case RSP_V2:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 2, 0), (UINT16)VREG_S( 2, 1), (UINT16)VREG_S( 2, 2), (UINT16)VREG_S( 2, 3), (UINT16)VREG_S( 2, 4), (UINT16)VREG_S( 2, 5), (UINT16)VREG_S( 2, 6), (UINT16)VREG_S( 2, 7));
			break;
		case RSP_V3:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 3, 0), (UINT16)VREG_S( 3, 1), (UINT16)VREG_S( 3, 2), (UINT16)VREG_S( 3, 3), (UINT16)VREG_S( 3, 4), (UINT16)VREG_S( 3, 5), (UINT16)VREG_S( 3, 6), (UINT16)VREG_S( 3, 7));
			break;
		case RSP_V4:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 4, 0), (UINT16)VREG_S( 4, 1), (UINT16)VREG_S( 4, 2), (UINT16)VREG_S( 4, 3), (UINT16)VREG_S( 4, 4), (UINT16)VREG_S( 4, 5), (UINT16)VREG_S( 4, 6), (UINT16)VREG_S( 4, 7));
			break;
		case RSP_V5:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 5, 0), (UINT16)VREG_S( 5, 1), (UINT16)VREG_S( 5, 2), (UINT16)VREG_S( 5, 3), (UINT16)VREG_S( 5, 4), (UINT16)VREG_S( 5, 5), (UINT16)VREG_S( 5, 6), (UINT16)VREG_S( 5, 7));
			break;
		case RSP_V6:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 6, 0), (UINT16)VREG_S( 6, 1), (UINT16)VREG_S( 6, 2), (UINT16)VREG_S( 6, 3), (UINT16)VREG_S( 6, 4), (UINT16)VREG_S( 6, 5), (UINT16)VREG_S( 6, 6), (UINT16)VREG_S( 6, 7));
			break;
		case RSP_V7:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 7, 0), (UINT16)VREG_S( 7, 1), (UINT16)VREG_S( 7, 2), (UINT16)VREG_S( 7, 3), (UINT16)VREG_S( 7, 4), (UINT16)VREG_S( 7, 5), (UINT16)VREG_S( 7, 6), (UINT16)VREG_S( 7, 7));
			break;
		case RSP_V8:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 8, 0), (UINT16)VREG_S( 8, 1), (UINT16)VREG_S( 8, 2), (UINT16)VREG_S( 8, 3), (UINT16)VREG_S( 8, 4), (UINT16)VREG_S( 8, 5), (UINT16)VREG_S( 8, 6), (UINT16)VREG_S( 8, 7));
			break;
		case RSP_V9:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S( 9, 0), (UINT16)VREG_S( 9, 1), (UINT16)VREG_S( 9, 2), (UINT16)VREG_S( 9, 3), (UINT16)VREG_S( 9, 4), (UINT16)VREG_S( 9, 5), (UINT16)VREG_S( 9, 6), (UINT16)VREG_S( 9, 7));
			break;
		case RSP_V10:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(10, 0), (UINT16)VREG_S(10, 1), (UINT16)VREG_S(10, 2), (UINT16)VREG_S(10, 3), (UINT16)VREG_S(10, 4), (UINT16)VREG_S(10, 5), (UINT16)VREG_S(10, 6), (UINT16)VREG_S(10, 7));
			break;
		case RSP_V11:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(11, 0), (UINT16)VREG_S(11, 1), (UINT16)VREG_S(11, 2), (UINT16)VREG_S(11, 3), (UINT16)VREG_S(11, 4), (UINT16)VREG_S(11, 5), (UINT16)VREG_S(11, 6), (UINT16)VREG_S(11, 7));
			break;
		case RSP_V12:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(12, 0), (UINT16)VREG_S(12, 1), (UINT16)VREG_S(12, 2), (UINT16)VREG_S(12, 3), (UINT16)VREG_S(12, 4), (UINT16)VREG_S(12, 5), (UINT16)VREG_S(12, 6), (UINT16)VREG_S(12, 7));
			break;
		case RSP_V13:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(13, 0), (UINT16)VREG_S(13, 1), (UINT16)VREG_S(13, 2), (UINT16)VREG_S(13, 3), (UINT16)VREG_S(13, 4), (UINT16)VREG_S(13, 5), (UINT16)VREG_S(13, 6), (UINT16)VREG_S(13, 7));
			break;
		case RSP_V14:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(14, 0), (UINT16)VREG_S(14, 1), (UINT16)VREG_S(14, 2), (UINT16)VREG_S(14, 3), (UINT16)VREG_S(14, 4), (UINT16)VREG_S(14, 5), (UINT16)VREG_S(14, 6), (UINT16)VREG_S(14, 7));
			break;
		case RSP_V15:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(15, 0), (UINT16)VREG_S(15, 1), (UINT16)VREG_S(15, 2), (UINT16)VREG_S(15, 3), (UINT16)VREG_S(15, 4), (UINT16)VREG_S(15, 5), (UINT16)VREG_S(15, 6), (UINT16)VREG_S(15, 7));
			break;
		case RSP_V16:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(16, 0), (UINT16)VREG_S(16, 1), (UINT16)VREG_S(16, 2), (UINT16)VREG_S(16, 3), (UINT16)VREG_S(16, 4), (UINT16)VREG_S(16, 5), (UINT16)VREG_S(16, 6), (UINT16)VREG_S(16, 7));
			break;
		case RSP_V17:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(17, 0), (UINT16)VREG_S(17, 1), (UINT16)VREG_S(17, 2), (UINT16)VREG_S(17, 3), (UINT16)VREG_S(17, 4), (UINT16)VREG_S(17, 5), (UINT16)VREG_S(17, 6), (UINT16)VREG_S(17, 7));
			break;
		case RSP_V18:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(18, 0), (UINT16)VREG_S(18, 1), (UINT16)VREG_S(18, 2), (UINT16)VREG_S(18, 3), (UINT16)VREG_S(18, 4), (UINT16)VREG_S(18, 5), (UINT16)VREG_S(18, 6), (UINT16)VREG_S(18, 7));
			break;
		case RSP_V19:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(19, 0), (UINT16)VREG_S(19, 1), (UINT16)VREG_S(19, 2), (UINT16)VREG_S(19, 3), (UINT16)VREG_S(19, 4), (UINT16)VREG_S(19, 5), (UINT16)VREG_S(19, 6), (UINT16)VREG_S(19, 7));
			break;
		case RSP_V20:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(20, 0), (UINT16)VREG_S(20, 1), (UINT16)VREG_S(20, 2), (UINT16)VREG_S(20, 3), (UINT16)VREG_S(20, 4), (UINT16)VREG_S(20, 5), (UINT16)VREG_S(20, 6), (UINT16)VREG_S(20, 7));
			break;
		case RSP_V21:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(21, 0), (UINT16)VREG_S(21, 1), (UINT16)VREG_S(21, 2), (UINT16)VREG_S(21, 3), (UINT16)VREG_S(21, 4), (UINT16)VREG_S(21, 5), (UINT16)VREG_S(21, 6), (UINT16)VREG_S(21, 7));
			break;
		case RSP_V22:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(22, 0), (UINT16)VREG_S(22, 1), (UINT16)VREG_S(22, 2), (UINT16)VREG_S(22, 3), (UINT16)VREG_S(22, 4), (UINT16)VREG_S(22, 5), (UINT16)VREG_S(22, 6), (UINT16)VREG_S(22, 7));
			break;
		case RSP_V23:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(23, 0), (UINT16)VREG_S(23, 1), (UINT16)VREG_S(23, 2), (UINT16)VREG_S(23, 3), (UINT16)VREG_S(23, 4), (UINT16)VREG_S(23, 5), (UINT16)VREG_S(23, 6), (UINT16)VREG_S(23, 7));
			break;
		case RSP_V24:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(24, 0), (UINT16)VREG_S(24, 1), (UINT16)VREG_S(24, 2), (UINT16)VREG_S(24, 3), (UINT16)VREG_S(24, 4), (UINT16)VREG_S(24, 5), (UINT16)VREG_S(24, 6), (UINT16)VREG_S(24, 7));
			break;
		case RSP_V25:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(25, 0), (UINT16)VREG_S(25, 1), (UINT16)VREG_S(25, 2), (UINT16)VREG_S(25, 3), (UINT16)VREG_S(25, 4), (UINT16)VREG_S(25, 5), (UINT16)VREG_S(25, 6), (UINT16)VREG_S(25, 7));
			break;
		case RSP_V26:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(26, 0), (UINT16)VREG_S(26, 1), (UINT16)VREG_S(26, 2), (UINT16)VREG_S(26, 3), (UINT16)VREG_S(26, 4), (UINT16)VREG_S(26, 5), (UINT16)VREG_S(26, 6), (UINT16)VREG_S(26, 7));
			break;
		case RSP_V27:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(27, 0), (UINT16)VREG_S(27, 1), (UINT16)VREG_S(27, 2), (UINT16)VREG_S(27, 3), (UINT16)VREG_S(27, 4), (UINT16)VREG_S(27, 5), (UINT16)VREG_S(27, 6), (UINT16)VREG_S(27, 7));
			break;
		case RSP_V28:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(28, 0), (UINT16)VREG_S(28, 1), (UINT16)VREG_S(28, 2), (UINT16)VREG_S(28, 3), (UINT16)VREG_S(28, 4), (UINT16)VREG_S(28, 5), (UINT16)VREG_S(28, 6), (UINT16)VREG_S(28, 7));
			break;
		case RSP_V29:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(29, 0), (UINT16)VREG_S(29, 1), (UINT16)VREG_S(29, 2), (UINT16)VREG_S(29, 3), (UINT16)VREG_S(29, 4), (UINT16)VREG_S(29, 5), (UINT16)VREG_S(29, 6), (UINT16)VREG_S(29, 7));
			break;
		case RSP_V30:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(30, 0), (UINT16)VREG_S(30, 1), (UINT16)VREG_S(30, 2), (UINT16)VREG_S(30, 3), (UINT16)VREG_S(30, 4), (UINT16)VREG_S(30, 5), (UINT16)VREG_S(30, 6), (UINT16)VREG_S(30, 7));
			break;
		case RSP_V31:
			strprintf(str, "%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)VREG_S(31, 0), (UINT16)VREG_S(31, 1), (UINT16)VREG_S(31, 2), (UINT16)VREG_S(31, 3), (UINT16)VREG_S(31, 4), (UINT16)VREG_S(31, 5), (UINT16)VREG_S(31, 6), (UINT16)VREG_S(31, 7));
			break;
	}
}

/***************************************************************************
    Vector Load Instructions
***************************************************************************/

void rsp_cop2::handle_lwc2(UINT32 op)
{
	int base = (op >> 21) & 0x1f;
#if !USE_SIMD
	int i, end;
	UINT32 ea;
	int dest = (op >> 16) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffffc0;
#endif

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* LBV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Load 1 byte to vector byte index

			//printf("LBV ");
#if USE_SIMD
			vec_lbdlsv_sbdlsv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + offset : offset;
			VREG_B(dest, index) = m_rsp.READ8(ea);
#endif
			//
			break;
		}
		case 0x01:      /* LSV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads 2 bytes starting from vector byte index

			//printf("LSV ");
#if USE_SIMD
			vec_lbdlsv_sbdlsv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 2) : (offset * 2);

			end = index + 2;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
#endif
			//
			break;
		}
		case 0x02:      /* LLV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads 4 bytes starting from vector byte index

			//printf("LLV ");
#if USE_SIMD
			vec_lbdlsv_sbdlsv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 4) : (offset * 4);

			end = index + 4;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
#endif
			//
			break;
		}
		case 0x03:      /* LDV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads 8 bytes starting from vector byte index

			//printf("LDV ");
#if USE_SIMD
			vec_lbdlsv_sbdlsv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			end = index + 8;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
#endif
			//
			break;
		}
		case 0x04:      /* LQV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00100 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads up to 16 bytes starting from vector byte index

			//printf("LQV ");
#if USE_SIMD
			vec_lqrv_sqrv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = index + (16 - (ea & 0xf));
			if (end > 16) end = 16;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
#endif
			//
			break;
		}
		case 0x05:      /* LRV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00101 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from right side until 16-byte boundary

			//printf("LRV ");
#if USE_SIMD
			vec_lqrv_sqrv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			index = 16 - ((ea & 0xf) - index);
			end = 16;
			ea &= ~0xf;

			for (i=index; i < end; i++)
			{
				VREG_B(dest, i) = m_rsp.READ8(ea);
				ea++;
			}
#endif
			//
			break;
		}
		case 0x06:      /* LPV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00110 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the upper 8 bits of each element

			//printf("LPV ");
#if USE_SIMD
			vec_lfhpuv_sfhpuv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = m_rsp.READ8(ea + (((16-index) + i) & 0xf)) << 8;
			}
#endif
			//
			break;
		}
		case 0x07:      /* LUV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 00111 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the bits 14-7 of each element

			//printf("LUV ");
#if USE_SIMD
			vec_lfhpuv_sfhpuv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = m_rsp.READ8(ea + (((16-index) + i) & 0xf)) << 7;
			}
#endif
			//
			break;
		}
		case 0x08:      /* LHV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the bits 14-7 of each element, with 2-byte stride

			//printf("LHV ");
#if USE_SIMD
			vec_lfhpuv_sfhpuv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			for (i=0; i < 8; i++)
			{
				VREG_S(dest, i) = m_rsp.READ8(ea + (((16-index) + (i<<1)) & 0xf)) << 7;
			}
#endif
			//
			break;
		}
		case 0x09:      /* LFV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads a byte as the bits 14-7 of upper or lower quad, with 4-byte stride

			//printf("LFV ");
#if USE_SIMD
			vec_lfhpuv_sfhpuv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			// not sure what happens if 16-byte boundary is crossed...

			end = (index >> 1) + 4;

			for (i=index >> 1; i < end; i++)
			{
				VREG_S(dest, i) = m_rsp.READ8(ea) << 7;
				ea += 4;
			}
#endif
			//
			break;
		}
		case 0x0a:      /* LWV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads the full 128-bit vector starting from vector byte index and wrapping to index 0
			// after byte index 15

			//printf("LWV ");
#if USE_SIMD
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = (16 - index) + 16;

			for (i=(16 - index); i < end; i++)
			{
				VREG_B(dest, i & 0xf) = m_rsp.READ8(ea);
				ea += 4;
			}
#endif
			//
			break;
		}
		case 0x0b:      /* LTV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 110010 | BBBBB | TTTTT | 01011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Loads one element to maximum of 8 vectors, while incrementing element index

			// FIXME: has a small problem with odd indices

			//printf("LTV ");
#if 0
#else
			INT32 index = (op >> 7) & 0xf;
			INT32 offset = (op & 0x7f);
			if (offset & 0x40)
				offset |= 0xffffffc0;

			INT32 vs = (op >> 16) & 0x1f;
			INT32 ve = vs + 8;
			if (ve > 32)
				ve = 32;

			INT32 element = 7 - (index >> 1);

			if (index & 1)  fatalerror("RSP: LTV: index = %d\n", index);

			UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			ea = ((ea + 8) & ~0xf) + (index & 1);
			for (INT32 i = vs; i < ve; i++)
			{
				element = ((8 - (index >> 1) + (i-vs)) << 1);
				VREG_B(i, (element & 0xf)) = m_rsp.READ8(ea);
				VREG_B(i, ((element + 1) & 0xf)) = m_rsp.READ8(ea + 1);

				ea += 2;
			}
#endif
			//
			break;
		}

		default:
		{
			m_rsp.unimplemented_opcode(op);
			break;
		}
	}
}


/***************************************************************************
    Vector Store Instructions
***************************************************************************/

void rsp_cop2::handle_swc2(UINT32 op)
{
	int base = (op >> 21) & 0x1f;
#if !USE_SIMD
	int i, end;
	int eaoffset;
	UINT32 ea;
	int dest = (op >> 16) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
		offset |= 0xffffffc0;
#endif

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:      /* SBV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 1 byte from vector byte index

			//printf("SBV ");
#if USE_SIMD
			vec_lbdlsv_sbdlsv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + offset : offset;
			m_rsp.WRITE8(ea, VREG_B(dest, index));
#endif
			//
			break;
		}
		case 0x01:      /* SSV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 2 bytes starting from vector byte index

			//printf("SSV ");
#if USE_SIMD
			vec_lbdlsv_sbdlsv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 2) : (offset * 2);

			end = index + 2;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, i));
				ea++;
			}
#endif
			//
			break;
		}
		case 0x02:      /* SLV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 4 bytes starting from vector byte index

			//printf("SLV ");
#if USE_SIMD
			vec_lbdlsv_sbdlsv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 4) : (offset * 4);

			end = index + 4;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, i));
				ea++;
			}
#endif
			//
			break;
		}
		case 0x03:      /* SDV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 8 bytes starting from vector byte index

			//printf("SDV ");
#if USE_SIMD
			vec_lbdlsv_sbdlsv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

			end = index + 8;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, i));
				ea++;
			}
#endif
			//
			break;
		}
		case 0x04:      /* SQV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00100 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from vector byte index until 16-byte boundary

			//printf("SQV ");
#if USE_SIMD
			vec_lqrv_sqrv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = index + (16 - (ea & 0xf));

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, i & 0xf));
				ea++;
			}
#endif
			//
			break;
		}
		case 0x05:      /* SRV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00101 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from right side until 16-byte boundary

			//printf("SRV ");
#if USE_SIMD
			vec_lqrv_sqrv(op, m_rsp.m_rsp_state->r[base]);
#else
			int o;
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			end = index + (ea & 0xf);
			o = (16 - (ea & 0xf)) & 0xf;
			ea &= ~0xf;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea, VREG_B(dest, ((i + o) & 0xf)));
				ea++;
			}
#endif
			//
			break;
		}
		case 0x06:      /* SPV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00110 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores upper 8 bits of each element

			//printf("SPV ");
#if USE_SIMD
			vec_lfhpuv_sfhpuv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					m_rsp.WRITE8(ea, VREG_B(dest, ((i & 0xf) << 1)));
				}
				else
				{
					m_rsp.WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
				}
				ea++;
			}
#endif
			//
			break;
		}
		case 0x07:      /* SUV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00111 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of each element

			//printf("SUV ");
#if USE_SIMD
			vec_lfhpuv_sfhpuv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					m_rsp.WRITE8(ea, VREG_S(dest, (i & 0x7)) >> 7);
				}
				else
				{
					m_rsp.WRITE8(ea, VREG_B(dest, ((i & 0x7) << 1)));
				}
				ea++;
			}
#endif
			//
			break;
		}
		case 0x08:      /* SHV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of each element, with 2-byte stride

			//printf("SHV ");
#if USE_SIMD
			vec_lfhpuv_sfhpuv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			for (i=0; i < 8; i++)
			{
				UINT8 d = ((VREG_B(dest, ((index + (i << 1) + 0) & 0xf))) << 1) |
							((VREG_B(dest, ((index + (i << 1) + 1) & 0xf))) >> 7);

				m_rsp.WRITE8(ea, d);
				ea += 2;
			}
#endif
			//
			break;
		}
		case 0x09:      /* SFV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of upper or lower quad, with 4-byte stride

			// FIXME: only works for index 0 and index 8

			//printf("SFV ");
#if USE_SIMD
			vec_lfhpuv_sfhpuv(op, m_rsp.m_rsp_state->r[base]);
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = (index >> 1) + 4;

			for (i=index >> 1; i < end; i++)
			{
				m_rsp.WRITE8(ea + (eaoffset & 0xf), VREG_S(dest, i) >> 7);
				eaoffset += 4;
			}
#endif
			//
			break;
		}
		case 0x0a:      /* SWV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores the full 128-bit vector starting from vector byte index and wrapping to index 0
			// after byte index 15

			//printf("SWV ");
#if USE_SIMD
#else
			ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = index + 16;

			for (i=index; i < end; i++)
			{
				m_rsp.WRITE8(ea + (eaoffset & 0xf), VREG_B(dest, i & 0xf));
				eaoffset++;
			}
#endif
			//
			break;
		}
		case 0x0b:      /* STV */
		{
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores one element from maximum of 8 vectors, while incrementing element index

			//printf("STV ");
#if 0
#else
			INT32 index = (op >> 7) & 0xf;
			INT32 offset = (op & 0x7f);
			if (offset & 0x40)
				offset |= 0xffffffc0;

			INT32 vs = (op >> 16) & 0x1f;
			INT32 ve = vs + 8;
			if (ve > 32)
				ve = 32;

			INT32 element = 8 - (index >> 1);

			UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

			INT32 eaoffset = (ea & 0xf) + (element * 2);
			ea &= ~0xf;

			for (INT32 i = vs; i < ve; i++)
			{
				m_rsp.WRITE16(ea + (eaoffset & 0xf), VREG_S(i, element & 0x7));
				eaoffset += 2;
				element++;
			}
#endif
			//
			break;
		}

		default:
		{
			m_rsp.unimplemented_opcode(op);
			break;
		}
	}
}

/***************************************************************************
    Vector Accumulator Helpers
***************************************************************************/

UINT16 rsp_cop2::SATURATE_ACCUM(int accum, int slice, UINT16 negative, UINT16 positive)
{
	if ((INT16)ACCUM_H(accum) < 0)
	{
		if ((UINT16)(ACCUM_H(accum)) != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) >= 0)
			{
				return negative;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else if (slice == 1)
				{
					return ACCUM_M(accum);
				}
			}
		}
	}
	else
	{
		if ((UINT16)(ACCUM_H(accum)) != 0)
		{
			return positive;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) < 0)
			{
				return positive;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else
				{
					return ACCUM_M(accum);
				}
			}
		}
	}
	return 0;
}


/***************************************************************************
    Vector Opcodes
***************************************************************************/

void rsp_cop2::handle_vector_ops(UINT32 op)
{
#if !USE_SIMD
	int i;
#endif

	// Opcode legend:
	//    E = VS2 element type
	//    S = VS1, Source vector 1
	//    T = VS2, Source vector 2
	//    D = Destination vector

	switch (op & 0x3f)
	{
		case 0x00:      /* VMULF */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000000 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer * 2

			//printf("MULF ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmulf_vmulu(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

				if (s1 == -32768 && s2 == -32768)
				{
					// overflow
					SET_ACCUM_H(0, i);
					SET_ACCUM_M(-32768, i);
					SET_ACCUM_L(-32768, i);
					m_vres[i] = 0x7fff;
				}
				else
				{
					INT64 r =  s1 * s2 * 2;
					r += 0x8000;    // rounding ?
					SET_ACCUM_H((r < 0) ? 0xffff : 0, i);      // sign-extend to 48-bit
					SET_ACCUM_M((INT16)(r >> 16), i);
					SET_ACCUM_L((UINT16)(r), i);
					m_vres[i] = ACCUM_M(i);
				}
			}
			WRITEBACK_RESULT();
#endif
			//
			break;

		}

		case 0x01:      /* VMULU */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000001 |
			// ------------------------------------------------------
			//

			//printf("MULU ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmulf_vmulu(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

				INT64 r = s1 * s2 * 2;
				r += 0x8000;    // rounding ?

				SET_ACCUM_H((UINT16)(r >> 32), i);
				SET_ACCUM_M((UINT16)(r >> 16), i);
				SET_ACCUM_L((UINT16)(r), i);

				if (r < 0)
				{
					m_vres[i] = 0;
				}
				else if (((INT16)(ACCUM_H(i)) ^ (INT16)(ACCUM_M(i))) < 0)
				{
					m_vres[i] = -1;
				}
				else
				{
					m_vres[i] = ACCUM_M(i);
				}
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x04:      /* VMUDL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000100 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by unsigned fraction
			// Stores the higher 16 bits of the 32-bit result to accumulator
			// The low slice of accumulator is stored into destination element

			//printf("MUDL ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			acc_lo = read_acc_lo(acc);
			acc_mid = read_acc_mid(acc);
			acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmadl_vmudl(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i=0; i < 8; i++)
			{
				UINT32 s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				UINT32 s2 = (UINT32)(UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				UINT32 r = s1 * s2;

				SET_ACCUM_H(0, i);
				SET_ACCUM_M(0, i);
				SET_ACCUM_L((UINT16)(r >> 16), i);

				m_vres[i] = ACCUM_L(i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x05:      /* VMUDM */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000101 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by unsigned fraction
			// The result is stored into accumulator
			// The middle slice of accumulator is stored into destination element

			//printf("MUDM ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			acc_lo = read_acc_lo(acc);
			acc_mid = read_acc_mid(acc);
			acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmadm_vmudm(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));   // not sign-extended
				INT32 r =  s1 * s2;

				SET_ACCUM_H((r < 0) ? 0xffff : 0, i);      // sign-extend to 48-bit
				SET_ACCUM_M((INT16)(r >> 16), i);
				SET_ACCUM_L((UINT16)(r), i);

				m_vres[i] = ACCUM_M(i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;

		}

		case 0x06:      /* VMUDN */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000110 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by signed integer
			// The result is stored into accumulator
			// The low slice of accumulator is stored into destination element

			//printf("MUDN ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo = read_acc_lo(acc);
			rsp_vec_t acc_mid = read_acc_mid(acc);
			rsp_vec_t acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmadn_vmudn(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i=0; i < 8; i++)
			{
				INT32 s1 = (UINT16)VREG_S(VS1REG, i);     // not sign-extended
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 * s2;

				SET_ACCUM_H((r < 0) ? 0xffff : 0, i);      // sign-extend to 48-bit
				SET_ACCUM_M((INT16)(r >> 16), i);
				SET_ACCUM_L((UINT16)(r), i);

				m_vres[i] = ACCUM_L(i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x07:      /* VMUDH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000111 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer
			// The result is stored into highest 32 bits of accumulator, the low slice is zero
			// The highest 32 bits of accumulator is saturated into destination element

			//printf("MUDH ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			acc_lo = read_acc_lo(acc);
			acc_mid = read_acc_mid(acc);
			acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmadh_vmudh(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 * s2;

				SET_ACCUM_H((INT16)(r >> 16), i);
				SET_ACCUM_M((UINT16)(r), i);
				SET_ACCUM_L(0, i);

				if (r < -32768) r = -32768;
				if (r >  32767) r = 32767;
				m_vres[i] = (INT16)(r);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x08:      /* VMACF */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001000 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer * 2
			// The result is added to accumulator

			//printf("MACF ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			acc_lo = read_acc_lo(acc);
			acc_mid = read_acc_mid(acc);
			acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmacf_vmacu(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 * s2;

				UINT64 q = (UINT64)(UINT16)ACCUM_LL(i);
				q |= (((UINT64)(UINT16)ACCUM_L(i)) << 16);
				q |= (((UINT64)(UINT16)ACCUM_M(i)) << 32);
				q |= (((UINT64)(UINT16)ACCUM_H(i)) << 48);

				q += (INT64)(r) << 17;

				SET_ACCUM_LL((UINT16)q, i);
				SET_ACCUM_L((UINT16)(q >> 16), i);
				SET_ACCUM_M((UINT16)(q >> 32), i);
				SET_ACCUM_H((UINT16)(q >> 48), i);

				m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}
		case 0x09:      /* VMACU */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001001 |
			// ------------------------------------------------------
			//

			//printf("MACU ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			acc_lo = read_acc_lo(acc);
			acc_mid = read_acc_mid(acc);
			acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmacf_vmacu(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i = 0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r1 = s1 * s2;
				UINT32 r2 = (UINT16)ACCUM_L(i) + ((UINT16)(r1) * 2);
				UINT32 r3 = (UINT16)ACCUM_M(i) + (UINT16)((r1 >> 16) * 2) + (UINT16)(r2 >> 16);

				SET_ACCUM_L((UINT16)(r2), i);
				SET_ACCUM_M((UINT16)(r3), i);
				SET_ACCUM_H(ACCUM_H(i) + (UINT16)(r3 >> 16) + (UINT16)(r1 >> 31), i);

				if ((INT16)ACCUM_H(i) < 0)
				{
					m_vres[i] = 0;
				}
				else
				{
					if (ACCUM_H(i) != 0)
					{
						m_vres[i] = 0xffff;
					}
					else
					{
						if ((INT16)ACCUM_M(i) < 0)
						{
							m_vres[i] = 0xffff;
						}
						else
						{
							m_vres[i] = ACCUM_M(i);
						}
					}
				}
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x0c:      /* VMADL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001100 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by unsigned fraction
			// Adds the higher 16 bits of the 32-bit result to accumulator
			// The low slice of accumulator is stored into destination element

			//printf("MADL ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			acc_lo = read_acc_lo(acc);
			acc_mid = read_acc_mid(acc);
			acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmadl_vmudl(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i = 0; i < 8; i++)
			{
				UINT32 s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				UINT32 s2 = (UINT32)(UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				UINT32 r1 = s1 * s2;
				UINT32 r2 = (UINT16)ACCUM_L(i) + (r1 >> 16);
				UINT32 r3 = (UINT16)ACCUM_M(i) + (r2 >> 16);

				SET_ACCUM_L((UINT16)(r2), i);
				SET_ACCUM_M((UINT16)(r3), i);
				SET_ACCUM_H(ACCUM_H(i) + (INT16)(r3 >> 16), i);

				m_vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x0d:      /* VMADM */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001101 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by unsigned fraction
			// The result is added into accumulator
			// The middle slice of accumulator is stored into destination element

			//printf("MADM ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			acc_lo = read_acc_lo(acc);
			acc_mid = read_acc_mid(acc);
			acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmadm_vmudm(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i=0; i < 8; i++)
			{
				UINT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				UINT32 s2 = (UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));   // not sign-extended
				UINT32 r1 = s1 * s2;
				UINT32 r2 = (UINT16)ACCUM_L(i) + (UINT16)(r1);
				UINT32 r3 = (UINT16)ACCUM_M(i) + (r1 >> 16) + (r2 >> 16);

				SET_ACCUM_L((UINT16)(r2), i);
				SET_ACCUM_M((UINT16)(r3), i);
				SET_ACCUM_H(ACCUM_H(i) + (UINT16)(r3 >> 16), i);
				if ((INT32)(r1) < 0)
					SET_ACCUM_H(ACCUM_H(i) - 1, i);

				m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x0e:      /* VMADN */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
			// ------------------------------------------------------
			//
			// Multiplies unsigned fraction by signed integer
			// The result is added into accumulator
			// The low slice of accumulator is stored into destination element

			//printf("MADN ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			acc_lo = read_acc_lo(acc);
			acc_mid = read_acc_mid(acc);
			acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmadn_vmudn(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i=0; i < 8; i++)
			{
				INT32 s1 = (UINT16)VREG_S(VS1REG, i);     // not sign-extended
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

				UINT64 q = (UINT64)ACCUM_LL(i);
				q |= (((UINT64)ACCUM_L(i)) << 16);
				q |= (((UINT64)ACCUM_M(i)) << 32);
				q |= (((UINT64)ACCUM_H(i)) << 48);
				q += (INT64)(s1*s2) << 16;

				SET_ACCUM_LL((UINT16)q, i);
				SET_ACCUM_L((UINT16)(q >> 16), i);
				SET_ACCUM_M((UINT16)(q >> 32), i);
				SET_ACCUM_H((UINT16)(q >> 48), i);

				m_vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
			}
			WRITEBACK_RESULT();

#endif
			//
			break;
		}

		case 0x0f:      /* VMADH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001111 |
			// ------------------------------------------------------
			//
			// Multiplies signed integer by signed integer
			// The result is added into highest 32 bits of accumulator, the low slice is zero
			// The highest 32 bits of accumulator is saturated into destination element

			//printf("MADH ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t acc_lo, acc_mid, acc_hi;

			acc_lo = read_acc_lo(acc);
			acc_mid = read_acc_mid(acc);
			acc_hi = read_acc_hi(acc);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmadh_vmudh(op, vs, vt_shuffle, vec_zero(), &acc_lo, &acc_mid, &acc_hi);

			write_acc_lo(acc, acc_lo);
			write_acc_mid(acc, acc_mid);
			write_acc_hi(acc, acc_hi);
#else
			for (i = 0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

				INT32 accum = (UINT32)(UINT16)ACCUM_M(i);
				accum |= ((UINT32)((UINT16)ACCUM_H(i))) << 16;
				accum += s1 * s2;

				SET_ACCUM_H((UINT16)(accum >> 16), i);
				SET_ACCUM_M((UINT16)accum, i);

				m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
			}
			WRITEBACK_RESULT();

#endif
			//
			break;
		}

		case 0x10:      /* VADD */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010000 |
			// ------------------------------------------------------
			//
			// Adds two vector registers and carry flag, the result is saturated to 32767

			// TODO: check VS2REG == VDREG

			//printf("ADD ");
#if USE_SIMD
			rsp_vec_t acc_lo;
			UINT16 *acc = m_acc.s;
			rsp_vec_t carry = read_vco_lo(m_flags[RSP_VCO].s);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vadd(vs, vt_shuffle, carry, &acc_lo);

			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, vec_zero());
			write_acc_lo(acc, acc_lo);
#else
			for (i=0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 + s2 + (CARRY_FLAG(i) != 0 ? 1 : 0);

				SET_ACCUM_L((INT16)(r), i);

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;
				m_vres[i] = (INT16)(r);
			}
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x11:      /* VSUB */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010001 |
			// ------------------------------------------------------
			//
			// Subtracts two vector registers and carry flag, the result is saturated to -32768

			// TODO: check VS2REG == VDREG

			//printf("SUB ");
#if USE_SIMD
			rsp_vec_t acc_lo;
			UINT16 *acc = m_acc.s;
			rsp_vec_t carry = read_vco_lo(m_flags[RSP_VCO].s);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vsub(vs, vt_shuffle, carry, &acc_lo);

			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, vec_zero());
			write_acc_lo(acc, acc_lo);
#else
			for (i = 0; i < 8; i++)
			{
				INT32 s1 = (INT32)(INT16)VREG_S(VS1REG, i);
				INT32 s2 = (INT32)(INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 - s2 - (CARRY_FLAG(i) != 0 ? 1 : 0);

				SET_ACCUM_L((INT16)(r), i);

				if (r > 32767) r = 32767;
				if (r < -32768) r = -32768;

				m_vres[i] = (INT16)(r);
			}
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x13:      /* VABS */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010011 |
			// ------------------------------------------------------
			//
			// Changes the sign of source register 2 if source register 1 is negative and stores
			// the result to destination register

			//printf("ABS ");
#if USE_SIMD
			rsp_vec_t acc_lo;
			UINT16 *acc = m_acc.s;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vabs(vs, vt_shuffle, vec_zero(), &acc_lo);

			write_acc_lo(acc, acc_lo);
#else
			for (i=0; i < 8; i++)
			{
				INT16 s1 = (INT16)VREG_S(VS1REG, i);
				INT16 s2 = (INT16)VREG_S(VS2REG, VEC_EL_2(EL, i));

				if (s1 < 0)
				{
					if (s2 == -32768)
					{
						m_vres[i] = 32767;
					}
					else
					{
						m_vres[i] = -s2;
					}
				}
				else if (s1 > 0)
				{
					m_vres[i] = s2;
				}
				else
				{
					m_vres[i] = 0;
				}

				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x14:      /* VADDC */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010100 |
			// ------------------------------------------------------
			//
			// Adds two vector registers, the carry out is stored into carry register

			// TODO: check VS2REG = VDREG

			//printf("ADDC ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t sn;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vaddc(vs, vt_shuffle, vec_zero(), &sn);

			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, sn);
			write_acc_lo(acc, m_v[VDREG].v);
#else
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				INT32 s2 = (UINT32)(UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 + s2;

				m_vres[i] = (INT16)(r);
				SET_ACCUM_L((INT16)(r), i);

				if (r & 0xffff0000)
				{
					SET_CARRY_FLAG(i);
				}
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x15:      /* VSUBC */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010101 |
			// ------------------------------------------------------
			//
			// Subtracts two vector registers, the carry out is stored into carry register

			// TODO: check VS2REG = VDREG

			//printf("SUBC ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t eq, sn;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vsubc(vs, vt_shuffle, vec_zero(), &eq, &sn);

			write_vco_hi(m_flags[RSP_VCO].s, eq);
			write_vco_lo(m_flags[RSP_VCO].s, sn);
			write_acc_lo(acc, m_v[VDREG].v);
#else
			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT32 s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
				INT32 s2 = (UINT32)(UINT16)VREG_S(VS2REG, VEC_EL_2(EL, i));
				INT32 r = s1 - s2;

				m_vres[i] = (INT16)(r);
				SET_ACCUM_L((UINT16)(r), i);

				if ((UINT16)(r) != 0)
				{
					SET_ZERO_FLAG(i);
				}
				if (r & 0xffff0000)
				{
					SET_CARRY_FLAG(i);
				}
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x1d:      /* VSAW */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 011101 |
			// ------------------------------------------------------
			//
			// Stores high, middle or low slice of accumulator to destination vector

			//printf("SAW ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			switch (EL)
			{
				case 8:
					m_v[VDREG].v = read_acc_hi(acc);
					break;
				case 9:
					m_v[VDREG].v = read_acc_mid(acc);
					break;
				case 10:
					m_v[VDREG].v = read_acc_lo(acc);
					break;

				default:
					m_v[VDREG].v = _mm_setzero_si128();
					break;
			}
#else
			switch (EL)
			{
				case 0x08:      // VSAWH
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_H(i);
					}
					break;
				}
				case 0x09:      // VSAWM
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_M(i);
					}
					break;
				}
				case 0x0a:      // VSAWL
				{
					for (i=0; i < 8; i++)
					{
						VREG_S(VDREG, i) = ACCUM_L(i);
					}
					break;
				}
				default:    //fatalerror("RSP: VSAW: el = %d\n", EL);//???????
					printf("RSP: VSAW: el = %d\n", EL);//??? ???
					exit(0);
			}
#endif
			//
			break;
		}

		case 0x20:      /* VLT */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100000 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are less than VS2
			// Moves the element in VS2 to destination vector

			//printf("LT ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t le;

			rsp_vec_t eq = read_vco_hi(m_flags[RSP_VCO].s);
			rsp_vec_t sign = read_vco_lo(m_flags[RSP_VCO].s);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_veq_vge_vlt_vne(op, vs, vt_shuffle, vec_zero(), &le, eq, sign);

			write_vcc_hi(m_flags[RSP_VCC].s, vec_zero());
			write_vcc_lo(m_flags[RSP_VCC].s, le);
			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, vec_zero());
			write_acc_lo(acc, m_v[VDREG].v);
#else
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT16 s1, s2;
				s1 = VREG_S(VS1REG, i);
				s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));
				if (s1 < s2)
				{
					SET_COMPARE_FLAG(i);
				}
				else if (s1 == s2)
				{
					if (ZERO_FLAG(i) != 0 && CARRY_FLAG(i) != 0)
					{
						SET_COMPARE_FLAG(i);
					}
				}

				if (COMPARE_FLAG(i) != 0)
				{
					m_vres[i] = s1;
				}
				else
				{
					m_vres[i] = s2;
				}

				SET_ACCUM_L(m_vres[i], i);
			}

			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x21:      /* VEQ */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100001 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are equal with VS2
			// Moves the element in VS2 to destination vector

			//printf("EQ ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t le;

			rsp_vec_t eq = read_vco_hi(m_flags[RSP_VCO].s);
			rsp_vec_t sign = read_vco_lo(m_flags[RSP_VCO].s);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_veq_vge_vlt_vne(op, vs, vt_shuffle, vec_zero(), &le, eq, sign);

			write_vcc_hi(m_flags[RSP_VCC].s, vec_zero());
			write_vcc_lo(m_flags[RSP_VCC].s, le);
			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, vec_zero());
			write_acc_lo(acc, m_v[VDREG].v);
#else
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i = 0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

				if ((s1 == s2) && ZERO_FLAG(i) == 0)
				{
					SET_COMPARE_FLAG(i);
					m_vres[i] = s1;
				}
				else
				{
					m_vres[i] = s2;
				}
				SET_ACCUM_L(m_vres[i], i);
			}

			CLEAR_ZERO_FLAGS();
			CLEAR_CARRY_FLAGS();
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x22:      /* VNE */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100010 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are not equal with VS2
			// Moves the element in VS2 to destination vector

			//printf("NE ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t le;

			rsp_vec_t eq = read_vco_hi(m_flags[RSP_VCO].s);
			rsp_vec_t sign = read_vco_lo(m_flags[RSP_VCO].s);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_veq_vge_vlt_vne(op, vs, vt_shuffle, vec_zero(), &le, eq, sign);

			write_vcc_hi(m_flags[RSP_VCC].s, vec_zero());
			write_vcc_lo(m_flags[RSP_VCC].s, le);
			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, vec_zero());
			write_acc_lo(acc, m_v[VDREG].v);
#else
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i = 0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

				if (s1 != s2 || ZERO_FLAG(i) != 0)
				{
					SET_COMPARE_FLAG(i);
					m_vres[i] = s1;
				}
				else
				{
					m_vres[i] = s2;
				}

				SET_ACCUM_L(m_vres[i], i);
			}

			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x23:      /* VGE */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100011 |
			// ------------------------------------------------------
			//
			// Sets compare flags if elements in VS1 are greater or equal with VS2
			// Moves the element in VS2 to destination vector

			//printf("GE ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t le;

			rsp_vec_t eq = read_vco_hi(m_flags[RSP_VCO].s);
			rsp_vec_t sign = read_vco_lo(m_flags[RSP_VCO].s);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_veq_vge_vlt_vne(op, vs, vt_shuffle, vec_zero(), &le, eq, sign);

			write_vcc_hi(m_flags[RSP_VCC].s, vec_zero());
			write_vcc_lo(m_flags[RSP_VCC].s, le);
			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, vec_zero());
			write_acc_lo(acc, m_v[VDREG].v);
#else
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

				if ((s1 == s2 && (ZERO_FLAG(i) == 0 || CARRY_FLAG(i) == 0)) || s1 > s2)
				{
					SET_COMPARE_FLAG(i);
					m_vres[i] = s1;
				}
				else
				{
					m_vres[i] = s2;
				}

				SET_ACCUM_L(m_vres[i], i);
			}

			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x24:      /* VCL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100100 |
			// ------------------------------------------------------
			//
			// Vector clip low

			//printf("CL ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;

			rsp_vec_t ge = read_vcc_hi(m_flags[RSP_VCC].s);
			rsp_vec_t le = read_vcc_lo(m_flags[RSP_VCC].s);
			rsp_vec_t eq = read_vco_hi(m_flags[RSP_VCO].s);
			rsp_vec_t sign = read_vco_lo(m_flags[RSP_VCO].s);
			rsp_vec_t vce = read_vce(m_flags[RSP_VCE].s);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);
			m_v[VDREG].v = vec_vcl(vs, vt_shuffle, vec_zero(), &ge, &le, eq, sign, vce);

			write_vcc_hi(m_flags[RSP_VCC].s, ge);
			write_vcc_lo(m_flags[RSP_VCC].s, le);
			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, vec_zero());
			write_vce(m_flags[RSP_VCE].s, vec_zero());
			write_acc_lo(acc, m_v[VDREG].v);
#else
			for (i = 0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

				if (CARRY_FLAG(i) != 0) // vco_lo
				{
					if (ZERO_FLAG(i) != 0) // vco_hi
					{
						if (COMPARE_FLAG(i) != 0) // vcc_lo
						{
							SET_ACCUM_L(-(UINT16)s2, i);
						}
						else
						{
							SET_ACCUM_L(s1, i);
						}
					}
					else
					{
						if (CLIP1_FLAG(i) != 0) // vce
						{
							if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) > 0x10000)
							{
								SET_ACCUM_L(s1, i);
								CLEAR_COMPARE_FLAG(i);
							}
							else
							{
								SET_ACCUM_L(-((UINT16)s2), i);
								SET_COMPARE_FLAG(i);
							}
						}
						else
						{
							if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) != 0)
							{
								SET_ACCUM_L(s1, i);
								CLEAR_COMPARE_FLAG(i);
							}
							else
							{
								SET_ACCUM_L(-((UINT16)s2), i);
								SET_COMPARE_FLAG(i);
							}
						}
					}
				}
				else
				{
					if (ZERO_FLAG(i) != 0) // vco_hi
					{
						if (CLIP2_FLAG(i) != 0) // vcc_hi
						{
							SET_ACCUM_L(s2, i);
						}
						else
						{
							SET_ACCUM_L(s1, i);
						}
					}
					else
					{
						if (((INT32)(UINT16)s1 - (INT32)(UINT16)s2) >= 0)
						{
							SET_ACCUM_L(s2, i);
							SET_CLIP2_FLAG(i);
						}
						else
						{
							SET_ACCUM_L(s1, i);
							CLEAR_CLIP2_FLAG(i);
						}
					}
				}

				m_vres[i] = ACCUM_L(i);
			}
			CLEAR_CARRY_FLAGS(); // vco_lo
			CLEAR_ZERO_FLAGS(); // vco_hi
			CLEAR_CLIP1_FLAGS(); // vce
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x25:      /* VCH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100101 |
			// ------------------------------------------------------
			//
			// Vector clip high

			//printf("CH ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t ge, le, sign, eq, vce;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vch(vs, vt_shuffle, vec_zero(), &ge, &le, &eq, &sign, &vce);

			write_vcc_hi(m_flags[RSP_VCC].s, ge);
			write_vcc_lo(m_flags[RSP_VCC].s, le);
			write_vco_hi(m_flags[RSP_VCO].s, eq);
			write_vco_lo(m_flags[RSP_VCO].s, sign);
			write_vce(m_flags[RSP_VCE].s, vce);
			write_acc_lo(acc, m_v[VDREG].v);
#else
			CLEAR_CARRY_FLAGS();
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP1_FLAGS();
			CLEAR_ZERO_FLAGS();
			CLEAR_CLIP2_FLAGS();
			UINT32 vce = 0;

			for (i=0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

				if ((s1 ^ s2) < 0)
				{
					vce = (s1 + s2 == -1);
					SET_CARRY_FLAG(i);
					if (s2 < 0)
					{
						SET_CLIP2_FLAG(i);
					}

					if (s1 + s2 <= 0)
					{
						SET_COMPARE_FLAG(i);
						m_vres[i] = -((UINT16)s2);
					}
					else
					{
						m_vres[i] = s1;
					}

					if (s1 + s2 != 0)
					{
						if (s1 != ~s2)
						{
							SET_ZERO_FLAG(i);
						}
					}
				}
				else
				{
					vce = 0;
					if (s2 < 0)
					{
						SET_COMPARE_FLAG(i);
					}
					if (s1 - s2 >= 0)
					{
						SET_CLIP2_FLAG(i);
						m_vres[i] = s2;
					}
					else
					{
						m_vres[i] = s1;
					}

					if ((s1 - s2) != 0)
					{
						if (s1 != ~s2)
						{
							SET_ZERO_FLAG(i);
						}
					}
				}
				if (vce != 0)
				{
					SET_CLIP1_FLAG(i);
				}

				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x26:      /* VCR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100110 |
			// ------------------------------------------------------
			//
			// Vector clip reverse

			//printf("CR ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t ge, le;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vcr(vs, vt_shuffle, vec_zero(), &ge, &le);

			write_vcc_hi(m_flags[RSP_VCC].s, ge);
			write_vcc_lo(m_flags[RSP_VCC].s, le);
			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, vec_zero());
			write_vce(m_flags[RSP_VCE].s, vec_zero());
			write_acc_lo(acc, m_v[VDREG].v);
#else
			CLEAR_CARRY_FLAGS();
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP1_FLAGS();
			CLEAR_ZERO_FLAGS();
			CLEAR_CLIP2_FLAGS();

			for (i=0; i < 8; i++)
			{
				INT16 s1 = VREG_S(VS1REG, i);
				INT16 s2 = VREG_S(VS2REG, VEC_EL_2(EL, i));

				if ((INT16)(s1 ^ s2) < 0)
				{
					if (s2 < 0)
					{
						SET_CLIP2_FLAG(i);
					}
					if ((s1 + s2) <= 0)
					{
						SET_ACCUM_L(~((UINT16)s2), i);
						SET_COMPARE_FLAG(i);
					}
					else
					{
						SET_ACCUM_L(s1, i);
					}
				}
				else
				{
					if (s2 < 0)
					{
						SET_COMPARE_FLAG(i);
					}
					if ((s1 - s2) >= 0)
					{
						SET_ACCUM_L(s2, i);
						SET_CLIP2_FLAG(i);
					}
					else
					{
						SET_ACCUM_L(s1, i);
					}
				}

				m_vres[i] = ACCUM_L(i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x27:      /* VMRG */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100111 |
			// ------------------------------------------------------
			//
			// Merges two vectors according to compare flags

			//printf("MRG ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;
			rsp_vec_t le = read_vcc_lo(m_flags[RSP_VCC].s);

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vmrg(vs, vt_shuffle, le);

			write_vco_hi(m_flags[RSP_VCO].s, vec_zero());
			write_vco_lo(m_flags[RSP_VCO].s, vec_zero());
			write_acc_lo(acc, m_v[VDREG].v);
#else
			for (i = 0; i < 8; i++)
			{
				if (COMPARE_FLAG(i) != 0)
				{
					m_vres[i] = VREG_S(VS1REG, i);
				}
				else
				{
					m_vres[i] = VREG_S(VS2REG, VEC_EL_2(EL, i));
				}

				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}
		case 0x28:      /* VAND */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
			// ------------------------------------------------------
			//
			// Bitwise AND of two vector registers

			//printf("AND ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vand_vnand(op, vs, vt_shuffle);

			write_acc_lo(acc, m_v[VDREG].v);
#else
			for (i = 0; i < 8; i++)
			{
				m_vres[i] = VREG_S(VS1REG, i) & VREG_S(VS2REG, VEC_EL_2(EL, i));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}
		case 0x29:      /* VNAND */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101001 |
			// ------------------------------------------------------
			//
			// Bitwise NOT AND of two vector registers

			//printf("NAND ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vand_vnand(op, vs, vt_shuffle);

			write_acc_lo(acc, m_v[VDREG].v);
#else
			for (i = 0; i < 8; i++)
			{
				m_vres[i] = ~((VREG_S(VS1REG, i) & VREG_S(VS2REG, VEC_EL_2(EL, i))));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}
		case 0x2a:      /* VOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101010 |
			// ------------------------------------------------------
			//
			// Bitwise OR of two vector registers

			//printf("OR ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vor_vnor(op, vs, vt_shuffle);

			write_acc_lo(acc, m_v[VDREG].v);
#else
			for (i = 0; i < 8; i++)
			{
				m_vres[i] = VREG_S(VS1REG, i) | VREG_S(VS2REG, VEC_EL_2(EL, i));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}
		case 0x2b:      /* VNOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101011 |
			// ------------------------------------------------------
			//
			// Bitwise NOT OR of two vector registers

			//printf("NOR ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vor_vnor(op, vs, vt_shuffle);

			write_acc_lo(acc, m_v[VDREG].v);
#else
			for (i=0; i < 8; i++)
			{
				m_vres[i] = ~((VREG_S(VS1REG, i) | VREG_S(VS2REG, VEC_EL_2(EL, i))));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}
		case 0x2c:      /* VXOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101100 |
			// ------------------------------------------------------
			//
			// Bitwise XOR of two vector registers

			//printf("XOR ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vxor_vnxor(op, vs, vt_shuffle);

			write_acc_lo(acc, m_v[VDREG].v);
#else
			for (i=0; i < 8; i++)
			{
				m_vres[i] = VREG_S(VS1REG, i) ^ VREG_S(VS2REG, VEC_EL_2(EL, i));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}
		case 0x2d:      /* VNXOR */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
			// ------------------------------------------------------
			//
			// Bitwise NOT XOR of two vector registers

			//printf("NXOR ");
#if USE_SIMD
			UINT16 *acc = m_acc.s;

			rsp_vec_t vs = vec_load_unshuffled_operand(m_v[VS1REG].s);
			rsp_vec_t vt_shuffle = vec_load_and_shuffle_operand(m_v[VS2REG].s, EL);

			m_v[VDREG].v = vec_vxor_vnxor(op, vs, vt_shuffle);

			write_acc_lo(acc, m_v[VDREG].v);
#else
			for (i=0; i < 8; i++)
			{
				m_vres[i] = ~((VREG_S(VS1REG, i) ^ VREG_S(VS2REG, VEC_EL_2(EL, i))));
				SET_ACCUM_L(m_vres[i], i);
			}
			WRITEBACK_RESULT();
#endif
			//
			break;
		}

		case 0x30:      /* VRCP */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110000 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal

			//printf("RCP ");
#if USE_SIMD
			write_acc_lo(m_acc.s, vec_load_and_shuffle_operand(m_v[VS2REG].s, EL));

			INT32 dp = op & m_dp_flag;
			m_dp_flag = 0;

			m_v[VDREG].v = vec_vrcp_vrsq(op, dp, VS2REG, EL, VDREG, VS1REG);
#else
			INT32 shifter = 0;

			INT32 rec = (INT16)(VREG_S(VS2REG, EL & 7));
			INT32 datainput = (rec < 0) ? (-rec) : rec;
			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))
					{
						shifter = i;
						break;
					}
				}
			}
			else
			{
				shifter = 0x10;
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
			if (rec < 0)
			{
				temp = ~temp;
			}
			if (!rec)
			{
				temp = 0x7fffffff;
			}
			else if (rec == 0xffff8000)
			{
				temp = 0xffff0000;
			}
			rec = temp;

			m_reciprocal_res = rec;
			m_dp_allowed = 0;

			VREG_S(VDREG, VS1REG & 7) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}


#endif
			//
			break;
		}

		case 0x31:      /* VRCPL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110001 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal low part

			//printf("RCPL ");
#if USE_SIMD
			write_acc_lo(m_acc.s, vec_load_and_shuffle_operand(m_v[VS2REG].s, EL));

			INT32 dp = op & m_dp_flag;
			m_dp_flag = 0;

			m_v[VDREG].v = vec_vrcp_vrsq(op, dp, VS2REG, EL, VDREG, VS1REG);
#else
			INT32 shifter = 0;

			INT32 rec = (INT16)VREG_S(VS2REG, EL & 7);
			INT32 datainput = rec;

			if (m_dp_allowed)
			{
				rec = (rec & 0x0000ffff) | m_reciprocal_high;
				datainput = rec;

				if (rec < 0)
				{
					if (rec < -32768)
					{
						datainput = ~datainput;
					}
					else
					{
						datainput = -datainput;
					}
				}
			}
			else if (datainput < 0)
			{
				datainput = -datainput;

				shifter = 0x10;
			}


			for (i = 0; i < 32; i++)
			{
				if (datainput & (1 << ((~i) & 0x1f)))
				{
					shifter = i;
					break;
				}
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
			temp ^= rec >> 31;

			if (!rec)
			{
				temp = 0x7fffffff;
			}
			else if (rec == 0xffff8000)
			{
				temp = 0xffff0000;
			}
			rec = temp;

			m_reciprocal_res = rec;
			m_dp_allowed = 0;

			VREG_S(VDREG, VS1REG & 7) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

#endif
			//
			break;
		}

		case 0x32:      /* VRCPH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110010 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal high part

			//printf("RCPH ");
#if USE_SIMD
			write_acc_lo(m_acc.s, vec_load_and_shuffle_operand(m_v[VS2REG].s, EL));

			m_dp_flag = 1;

			m_v[VDREG].v = vec_vdivh(VS2REG, EL, VDREG, VS1REG);
#else
			m_reciprocal_high = (VREG_S(VS2REG, EL & 7)) << 16;
			m_dp_allowed = 1;

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

			VREG_S(VDREG, VS1REG & 7) = (INT16)(m_reciprocal_res >> 16);

#endif
			//
			break;
		}

		case 0x33:      /* VMOV */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110011 |
			// ------------------------------------------------------
			//
			// Moves element from vector to destination vector

			//printf("MOV ");
#if USE_SIMD
			write_acc_lo(m_acc.s, vec_load_and_shuffle_operand(m_v[VS2REG].s, EL));
			m_v[VDREG].v = vec_vmov(VS2REG, EL, VDREG, VS1REG);
#else
			VREG_S(VDREG, VS1REG & 7) = VREG_S(VS2REG, EL & 7);
			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}
#endif
			//
			break;
		}

		case 0x34:      /* VRSQ */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110100 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal square-root

			//printf("RSQ ");
#if USE_SIMD
			write_acc_lo(m_acc.s, vec_load_and_shuffle_operand(m_v[VS2REG].s, EL));

			INT32 dp = op & m_dp_flag;
			m_dp_flag = 0;

			m_v[VDREG].v = vec_vrcp_vrsq(op, dp, VS2REG, EL, VDREG, VS1REG);
#else
			INT32 shifter = 0;

			INT32 rec = (INT16)(VREG_S(VS2REG, EL & 7));
			INT32 datainput = (rec < 0) ? (-rec) : rec;
			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
					{
						shifter = i;
						break;
					}
				}
			}
			else
			{
				shifter = 0x10;
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
			if (rec < 0)
			{
				temp = ~temp;
			}
			if (!rec)
			{
				temp = 0x7fffffff;
			}
			else if (rec == 0xffff8000)
			{
				temp = 0xffff0000;
			}
			rec = temp;

			m_reciprocal_res = rec;
			m_dp_allowed = 0;

			VREG_S(VDREG, VS1REG & 7) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

#endif
			//
			break;
		}

		case 0x35:      /* VRSQL */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110101 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal square-root low part

			//printf("RSQL ");
#if USE_SIMD
			write_acc_lo(m_acc.s, vec_load_and_shuffle_operand(m_v[VS2REG].s, EL));

			INT32 dp = op & m_dp_flag;
			m_dp_flag = 0;

			m_v[VDREG].v = vec_vrcp_vrsq(op, dp, VS2REG, EL, VDREG, VS1REG);
#else
			INT32 shifter = 0;
			INT32 rec = (INT16)VREG_S(VS2REG, EL & 7);
			INT32 datainput = rec;

			if (m_dp_allowed)
			{
				rec = (rec & 0x0000ffff) | m_reciprocal_high;
				datainput = rec;

				if (rec < 0)
				{
					if (rec < -32768)
					{
						datainput = ~datainput;
					}
					else
					{
						datainput = -datainput;
					}
				}
			}
			else if (datainput < 0)
			{
				datainput = -datainput;

				shifter = 0x10;
			}

			if (datainput)
			{
				for (i = 0; i < 32; i++)
				{
					if (datainput & (1 << ((~i) & 0x1f)))
					{
						shifter = i;
						break;
					}
				}
			}

			INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
			address = ((address | 0x200) & 0x3fe) | (shifter & 1);

			INT32 fetchval = rsp_divtable[address];
			INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
			temp ^= rec >> 31;

			if (!rec)
			{
				temp = 0x7fffffff;
			}
			else if (rec == 0xffff8000)
			{
				temp = 0xffff0000;
			}
			rec = temp;

			m_reciprocal_res = rec;
			m_dp_allowed = 0;

			VREG_S(VDREG, VS1REG & 7) = (UINT16)(rec & 0xffff);

			for (i = 0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

#endif
			//
			break;
		}

		case 0x36:      /* VRSQH */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110110 |
			// ------------------------------------------------------
			//
			// Calculates reciprocal square-root high part

			//printf("RSQH ");
#if USE_SIMD
			write_acc_lo(m_acc.s, vec_load_and_shuffle_operand(m_v[VS2REG].s, EL));

			m_dp_flag = 1;

			m_v[VDREG].v = vec_vdivh(VS2REG, EL, VDREG, VS1REG);
#else
			m_reciprocal_high = (VREG_S(VS2REG, EL & 7)) << 16;
			m_dp_allowed = 1;

			for (i=0; i < 8; i++)
			{
				SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
			}

			VREG_S(VDREG, VS1REG & 7) = (INT16)(m_reciprocal_res >> 16);    // store high part
#endif
			//
			break;
		}

		case 0x37:      /* VNOP */
		{
			// 31       25  24     20      15      10      5        0
			// ------------------------------------------------------
			// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110111 |
			// ------------------------------------------------------
			//
			// Vector null instruction

			//printf("NOP ");
			break;
		}

		default:    m_rsp.unimplemented_opcode(op); break;
	}
}

/***************************************************************************
    Vector Flag Reading/Writing
***************************************************************************/

void rsp_cop2::handle_cop2(UINT32 op)
{
	switch ((op >> 21) & 0x1f)
	{
		case 0x00: /* MFC2 */
		{
			// 31 25 20 15 10 6 0
			// ---------------------------------------------------
			// | 010010 | 00000 | TTTTT | DDDDD | IIII | 0000000 |
			// ---------------------------------------------------
			//
			//printf("MFC2 ");
			int el = (op >> 7) & 0xf;
			UINT16 b1 = VREG_B(RDREG, (el+0) & 0xf);
			UINT16 b2 = VREG_B(RDREG, (el+1) & 0xf);
			if (RTREG) RTVAL = (INT32)(INT16)((b1 << 8) | (b2));
			break;
		}

		case 0x02: /* CFC2 */
		{
			// 31 25 20 15 10 0
			// ------------------------------------------------
			// | 010010 | 00010 | TTTTT | DDDDD | 00000000000 |
			// ------------------------------------------------
			//
			//printf("CFC2 ");
			if (RTREG)
			{
#if USE_SIMD
				INT32 src = RDREG & 3;
				if (src == 3) {
					src = 2;
				}
				RTVAL = get_flags(m_flags[src].s);
#else
				switch(RDREG)
				{
					case 0:
						RTVAL = ((CARRY_FLAG(0) & 1) << 0) |
						((CARRY_FLAG(1) & 1) << 1) |
						((CARRY_FLAG(2) & 1) << 2) |
						((CARRY_FLAG(3) & 1) << 3) |
						((CARRY_FLAG(4) & 1) << 4) |
						((CARRY_FLAG(5) & 1) << 5) |
						((CARRY_FLAG(6) & 1) << 6) |
						((CARRY_FLAG(7) & 1) << 7) |
						((ZERO_FLAG(0) & 1) << 8) |
						((ZERO_FLAG(1) & 1) << 9) |
						((ZERO_FLAG(2) & 1) << 10) |
						((ZERO_FLAG(3) & 1) << 11) |
						((ZERO_FLAG(4) & 1) << 12) |
						((ZERO_FLAG(5) & 1) << 13) |
						((ZERO_FLAG(6) & 1) << 14) |
						((ZERO_FLAG(7) & 1) << 15);
						if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
						break;
					case 1:
						RTVAL = ((COMPARE_FLAG(0) & 1) << 0) |
						((COMPARE_FLAG(1) & 1) << 1) |
						((COMPARE_FLAG(2) & 1) << 2) |
						((COMPARE_FLAG(3) & 1) << 3) |
						((COMPARE_FLAG(4) & 1) << 4) |
						((COMPARE_FLAG(5) & 1) << 5) |
						((COMPARE_FLAG(6) & 1) << 6) |
						((COMPARE_FLAG(7) & 1) << 7) |
						((CLIP2_FLAG(0) & 1) << 8) |
						((CLIP2_FLAG(1) & 1) << 9) |
						((CLIP2_FLAG(2) & 1) << 10) |
						((CLIP2_FLAG(3) & 1) << 11) |
						((CLIP2_FLAG(4) & 1) << 12) |
						((CLIP2_FLAG(5) & 1) << 13) |
						((CLIP2_FLAG(6) & 1) << 14) |
						((CLIP2_FLAG(7) & 1) << 15);
						if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
						break;
					case 2:
						// Anciliary clipping flags
						RTVAL = ((CLIP1_FLAG(0) & 1) << 0) |
						((CLIP1_FLAG(1) & 1) << 1) |
						((CLIP1_FLAG(2) & 1) << 2) |
						((CLIP1_FLAG(3) & 1) << 3) |
						((CLIP1_FLAG(4) & 1) << 4) |
						((CLIP1_FLAG(5) & 1) << 5) |
						((CLIP1_FLAG(6) & 1) << 6) |
						((CLIP1_FLAG(7) & 1) << 7);
				}
#endif
			}
			break;
		}

		case 0x04: /* MTC2 */
		{
			// 31 25 20 15 10 6 0
			// ---------------------------------------------------
			// | 010010 | 00100 | TTTTT | DDDDD | IIII | 0000000 |
			// ---------------------------------------------------
			//
			//printf("MTC2 ");
			int el = (op >> 7) & 0xf;
			W_VREG_B(RDREG, (el+0) & 0xf, (RTVAL >> 8) & 0xff);
			W_VREG_B(RDREG, (el+1) & 0xf, (RTVAL >> 0) & 0xff);
			break;
		}

		case 0x06: /* CTC2 */
		{
			// 31 25 20 15 10 0
			// ------------------------------------------------
			// | 010010 | 00110 | TTTTT | DDDDD | 00000000000 |
			// ------------------------------------------------
			//
			switch(RDREG)
			{
#if USE_SIMD
				case 0:
				case 1:
				case 2:
					UINT16 r0 = (RTVAL & (1 << 0)) ? 0xffff : 0;
					UINT16 r1 = (RTVAL & (1 << 1)) ? 0xffff : 0;
					UINT16 r2 = (RTVAL & (1 << 2)) ? 0xffff : 0;
					UINT16 r3 = (RTVAL & (1 << 3)) ? 0xffff : 0;
					UINT16 r4 = (RTVAL & (1 << 4)) ? 0xffff : 0;
					UINT16 r5 = (RTVAL & (1 << 5)) ? 0xffff : 0;
					UINT16 r6 = (RTVAL & (1 << 6)) ? 0xffff : 0;
					UINT16 r7 = (RTVAL & (1 << 7)) ? 0xffff : 0;
					m_flags[RDREG].__align[0] = _mm_set_epi16(r7, r6, r5, r4, r3, r2, r1, r0);
					r0 = (RTVAL & (1 << 8)) ? 0xffff : 0;
					r1 = (RTVAL & (1 << 9)) ? 0xffff : 0;
					r2 = (RTVAL & (1 << 10)) ? 0xffff : 0;
					r3 = (RTVAL & (1 << 11)) ? 0xffff : 0;
					r4 = (RTVAL & (1 << 12)) ? 0xffff : 0;
					r5 = (RTVAL & (1 << 13)) ? 0xffff : 0;
					r6 = (RTVAL & (1 << 14)) ? 0xffff : 0;
					r7 = (RTVAL & (1 << 15)) ? 0xffff : 0;
					m_flags[RDREG].__align[1] = _mm_set_epi16(r7, r6, r5, r4, r3, r2, r1, r0);
					break;
#else
				case 0:
					CLEAR_CARRY_FLAGS();
					CLEAR_ZERO_FLAGS();
					if (RTVAL & (1 << 0)) { SET_CARRY_FLAG(0); }
					if (RTVAL & (1 << 1)) { SET_CARRY_FLAG(1); }
					if (RTVAL & (1 << 2)) { SET_CARRY_FLAG(2); }
					if (RTVAL & (1 << 3)) { SET_CARRY_FLAG(3); }
					if (RTVAL & (1 << 4)) { SET_CARRY_FLAG(4); }
					if (RTVAL & (1 << 5)) { SET_CARRY_FLAG(5); }
					if (RTVAL & (1 << 6)) { SET_CARRY_FLAG(6); }
					if (RTVAL & (1 << 7)) { SET_CARRY_FLAG(7); }
					if (RTVAL & (1 << 8)) { SET_ZERO_FLAG(0); }
					if (RTVAL & (1 << 9)) { SET_ZERO_FLAG(1); }
					if (RTVAL & (1 << 10)) { SET_ZERO_FLAG(2); }
					if (RTVAL & (1 << 11)) { SET_ZERO_FLAG(3); }
					if (RTVAL & (1 << 12)) { SET_ZERO_FLAG(4); }
					if (RTVAL & (1 << 13)) { SET_ZERO_FLAG(5); }
					if (RTVAL & (1 << 14)) { SET_ZERO_FLAG(6); }
					if (RTVAL & (1 << 15)) { SET_ZERO_FLAG(7); }
					break;

				case 1:
					CLEAR_COMPARE_FLAGS();
					CLEAR_CLIP2_FLAGS();
					if (RTVAL & (1 << 0)) { SET_COMPARE_FLAG(0); }
					if (RTVAL & (1 << 1)) { SET_COMPARE_FLAG(1); }
					if (RTVAL & (1 << 2)) { SET_COMPARE_FLAG(2); }
					if (RTVAL & (1 << 3)) { SET_COMPARE_FLAG(3); }
					if (RTVAL & (1 << 4)) { SET_COMPARE_FLAG(4); }
					if (RTVAL & (1 << 5)) { SET_COMPARE_FLAG(5); }
					if (RTVAL & (1 << 6)) { SET_COMPARE_FLAG(6); }
					if (RTVAL & (1 << 7)) { SET_COMPARE_FLAG(7); }
					if (RTVAL & (1 << 8)) { SET_CLIP2_FLAG(0); }
					if (RTVAL & (1 << 9)) { SET_CLIP2_FLAG(1); }
					if (RTVAL & (1 << 10)) { SET_CLIP2_FLAG(2); }
					if (RTVAL & (1 << 11)) { SET_CLIP2_FLAG(3); }
					if (RTVAL & (1 << 12)) { SET_CLIP2_FLAG(4); }
					if (RTVAL & (1 << 13)) { SET_CLIP2_FLAG(5); }
					if (RTVAL & (1 << 14)) { SET_CLIP2_FLAG(6); }
					if (RTVAL & (1 << 15)) { SET_CLIP2_FLAG(7); }
					break;

				case 2:
					CLEAR_CLIP1_FLAGS();
					if (RTVAL & (1 << 0)) { SET_CLIP1_FLAG(0); }
					if (RTVAL & (1 << 1)) { SET_CLIP1_FLAG(1); }
					if (RTVAL & (1 << 2)) { SET_CLIP1_FLAG(2); }
					if (RTVAL & (1 << 3)) { SET_CLIP1_FLAG(3); }
					if (RTVAL & (1 << 4)) { SET_CLIP1_FLAG(4); }
					if (RTVAL & (1 << 5)) { SET_CLIP1_FLAG(5); }
					if (RTVAL & (1 << 6)) { SET_CLIP1_FLAG(6); }
					if (RTVAL & (1 << 7)) { SET_CLIP1_FLAG(7); }
					break;
#endif
			}
			break;
		}

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		{
			//printf("V");
			handle_vector_ops(op);
			break;
		}

		default:
			m_rsp.unimplemented_opcode(op);
			break;
	}
	//dump(op);
}

inline void rsp_cop2::mfc2()
{
	UINT32 op = m_rspcop2_state->op;
	int el = (op >> 7) & 0xf;

	UINT16 b1 = VREG_B(VS1REG, (el+0) & 0xf);
	UINT16 b2 = VREG_B(VS1REG, (el+1) & 0xf);
	if (RTREG) RTVAL = (INT32)(INT16)((b1 << 8) | (b2));
}

inline void rsp_cop2::cfc2()
{
	UINT32 op = m_rspcop2_state->op;
	if (RTREG)
	{
		switch(RDREG)
		{
			case 0:
				RTVAL = ((CARRY_FLAG(0) & 1) << 0) |
						((CARRY_FLAG(1) & 1) << 1) |
						((CARRY_FLAG(2) & 1) << 2) |
						((CARRY_FLAG(3) & 1) << 3) |
						((CARRY_FLAG(4) & 1) << 4) |
						((CARRY_FLAG(5) & 1) << 5) |
						((CARRY_FLAG(6) & 1) << 6) |
						((CARRY_FLAG(7) & 1) << 7) |
						((ZERO_FLAG(0) & 1) << 8) |
						((ZERO_FLAG(1) & 1) << 9) |
						((ZERO_FLAG(2) & 1) << 10) |
						((ZERO_FLAG(3) & 1) << 11) |
						((ZERO_FLAG(4) & 1) << 12) |
						((ZERO_FLAG(5) & 1) << 13) |
						((ZERO_FLAG(6) & 1) << 14) |
						((ZERO_FLAG(7) & 1) << 15);
				if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
				break;
			case 1:
				RTVAL = ((COMPARE_FLAG(0) & 1) << 0) |
						((COMPARE_FLAG(1) & 1) << 1) |
						((COMPARE_FLAG(2) & 1) << 2) |
						((COMPARE_FLAG(3) & 1) << 3) |
						((COMPARE_FLAG(4) & 1) << 4) |
						((COMPARE_FLAG(5) & 1) << 5) |
						((COMPARE_FLAG(6) & 1) << 6) |
						((COMPARE_FLAG(7) & 1) << 7) |
						((CLIP2_FLAG(0) & 1) << 8) |
						((CLIP2_FLAG(1) & 1) << 9) |
						((CLIP2_FLAG(2) & 1) << 10) |
						((CLIP2_FLAG(3) & 1) << 11) |
						((CLIP2_FLAG(4) & 1) << 12) |
						((CLIP2_FLAG(5) & 1) << 13) |
						((CLIP2_FLAG(6) & 1) << 14) |
						((CLIP2_FLAG(7) & 1) << 15);
				if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
				break;
			case 2:
				RTVAL = ((CLIP1_FLAG(0) & 1) << 0) |
						((CLIP1_FLAG(1) & 1) << 1) |
						((CLIP1_FLAG(2) & 1) << 2) |
						((CLIP1_FLAG(3) & 1) << 3) |
						((CLIP1_FLAG(4) & 1) << 4) |
						((CLIP1_FLAG(5) & 1) << 5) |
						((CLIP1_FLAG(6) & 1) << 6) |
						((CLIP1_FLAG(7) & 1) << 7);
				break;
		}
	}
}

inline void rsp_cop2::mtc2()
{
	UINT32 op = m_rspcop2_state->op;
	int el = (op >> 7) & 0xf;
	VREG_B(VS1REG, (el+0) & 0xf) = (RTVAL >> 8) & 0xff;
	VREG_B(VS1REG, (el+1) & 0xf) = (RTVAL >> 0) & 0xff;
}

inline void rsp_cop2::ctc2()
{
	UINT32 op = m_rspcop2_state->op;
	switch(RDREG)
	{
		case 0:
			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			m_vflag[0][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[0][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[0][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[0][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[0][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[0][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[0][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[0][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0))  { SET_CARRY_FLAG(0); }
			if (RTVAL & (1 << 1))  { SET_CARRY_FLAG(1); }
			if (RTVAL & (1 << 2))  { SET_CARRY_FLAG(2); }
			if (RTVAL & (1 << 3))  { SET_CARRY_FLAG(3); }
			if (RTVAL & (1 << 4))  { SET_CARRY_FLAG(4); }
			if (RTVAL & (1 << 5))  { SET_CARRY_FLAG(5); }
			if (RTVAL & (1 << 6))  { SET_CARRY_FLAG(6); }
			if (RTVAL & (1 << 7))  { SET_CARRY_FLAG(7); }
			m_vflag[3][0] = ((RTVAL >> 8) & 1) ? 0xffff : 0;
			m_vflag[3][1] = ((RTVAL >> 9) & 1) ? 0xffff : 0;
			m_vflag[3][2] = ((RTVAL >> 10) & 1) ? 0xffff : 0;
			m_vflag[3][3] = ((RTVAL >> 11) & 1) ? 0xffff : 0;
			m_vflag[3][4] = ((RTVAL >> 12) & 1) ? 0xffff : 0;
			m_vflag[3][5] = ((RTVAL >> 13) & 1) ? 0xffff : 0;
			m_vflag[3][6] = ((RTVAL >> 14) & 1) ? 0xffff : 0;
			m_vflag[3][7] = ((RTVAL >> 15) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 8))  { SET_ZERO_FLAG(0); }
			if (RTVAL & (1 << 9))  { SET_ZERO_FLAG(1); }
			if (RTVAL & (1 << 10)) { SET_ZERO_FLAG(2); }
			if (RTVAL & (1 << 11)) { SET_ZERO_FLAG(3); }
			if (RTVAL & (1 << 12)) { SET_ZERO_FLAG(4); }
			if (RTVAL & (1 << 13)) { SET_ZERO_FLAG(5); }
			if (RTVAL & (1 << 14)) { SET_ZERO_FLAG(6); }
			if (RTVAL & (1 << 15)) { SET_ZERO_FLAG(7); }
			break;
		case 1:
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();
			m_vflag[1][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[1][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[1][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[1][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[1][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[1][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[1][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[1][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0)) { SET_COMPARE_FLAG(0); }
			if (RTVAL & (1 << 1)) { SET_COMPARE_FLAG(1); }
			if (RTVAL & (1 << 2)) { SET_COMPARE_FLAG(2); }
			if (RTVAL & (1 << 3)) { SET_COMPARE_FLAG(3); }
			if (RTVAL & (1 << 4)) { SET_COMPARE_FLAG(4); }
			if (RTVAL & (1 << 5)) { SET_COMPARE_FLAG(5); }
			if (RTVAL & (1 << 6)) { SET_COMPARE_FLAG(6); }
			if (RTVAL & (1 << 7)) { SET_COMPARE_FLAG(7); }
			m_vflag[4][0] = ((RTVAL >> 8) & 1) ? 0xffff : 0;
			m_vflag[4][1] = ((RTVAL >> 9) & 1) ? 0xffff : 0;
			m_vflag[4][2] = ((RTVAL >> 10) & 1) ? 0xffff : 0;
			m_vflag[4][3] = ((RTVAL >> 11) & 1) ? 0xffff : 0;
			m_vflag[4][4] = ((RTVAL >> 12) & 1) ? 0xffff : 0;
			m_vflag[4][5] = ((RTVAL >> 13) & 1) ? 0xffff : 0;
			m_vflag[4][6] = ((RTVAL >> 14) & 1) ? 0xffff : 0;
			m_vflag[4][7] = ((RTVAL >> 15) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 8))  { SET_CLIP2_FLAG(0); }
			if (RTVAL & (1 << 9))  { SET_CLIP2_FLAG(1); }
			if (RTVAL & (1 << 10)) { SET_CLIP2_FLAG(2); }
			if (RTVAL & (1 << 11)) { SET_CLIP2_FLAG(3); }
			if (RTVAL & (1 << 12)) { SET_CLIP2_FLAG(4); }
			if (RTVAL & (1 << 13)) { SET_CLIP2_FLAG(5); }
			if (RTVAL & (1 << 14)) { SET_CLIP2_FLAG(6); }
			if (RTVAL & (1 << 15)) { SET_CLIP2_FLAG(7); }
			break;
		case 2:
			CLEAR_CLIP1_FLAGS();
			m_vflag[2][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[2][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[2][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[2][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[2][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[2][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[2][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[2][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0)) { SET_CLIP1_FLAG(0); }
			if (RTVAL & (1 << 1)) { SET_CLIP1_FLAG(1); }
			if (RTVAL & (1 << 2)) { SET_CLIP1_FLAG(2); }
			if (RTVAL & (1 << 3)) { SET_CLIP1_FLAG(3); }
			if (RTVAL & (1 << 4)) { SET_CLIP1_FLAG(4); }
			if (RTVAL & (1 << 5)) { SET_CLIP1_FLAG(5); }
			if (RTVAL & (1 << 6)) { SET_CLIP1_FLAG(6); }
			if (RTVAL & (1 << 7)) { SET_CLIP1_FLAG(7); }
			break;
	}
}

void rsp_cop2::log_instruction_execution()
{
	static VECTOR_REG prev_vecs[32];

	for (int i = 0; i < 32; i++)
	{
		if (m_v[i].d[0] != prev_vecs[i].d[0] || m_v[i].d[1] != prev_vecs[i].d[1])
		{
			fprintf(m_rsp.m_exec_output, "V%d: %04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X ", i,
			(UINT16)VREG_S(i,0), (UINT16)VREG_S(i,1), (UINT16)VREG_S(i,2), (UINT16)VREG_S(i,3), (UINT16)VREG_S(i,4), (UINT16)VREG_S(i,5), (UINT16)VREG_S(i,6), (UINT16)VREG_S(i,7));
		}
		prev_vecs[i].d[0] = m_v[i].d[0];
		prev_vecs[i].d[1] = m_v[i].d[1];
	}
}

void rsp_cop2::dump(UINT32 op)
{
	printf("%08x ", op);
	for (int i = 0; i < 32; i++)
	{
		printf("%08x ", m_rsp.m_rsp_state->r[i]);
	}
	printf("\n");

	for (int i = 0; i < 32; i++)
	{
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", VREG_B(i, 0), VREG_B(i, 1), VREG_B(i, 2), VREG_B(i, 3), VREG_B(i, 4), VREG_B(i, 5), VREG_B(i, 6), VREG_B(i, 7), VREG_B(i, 8), VREG_B(i, 9), VREG_B(i, 10), VREG_B(i, 11), VREG_B(i, 12), VREG_B(i, 13), VREG_B(i, 14), VREG_B(i, 15));
	}

#if USE_SIMD
	printf("acc_h: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_acc.s[0], m_acc.s[1], m_acc.s[2], m_acc.s[3], m_acc.s[4], m_acc.s[5], m_acc.s[6], m_acc.s[7]);
	printf("acc_m: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_acc.s[8], m_acc.s[9], m_acc.s[10], m_acc.s[11], m_acc.s[12], m_acc.s[13], m_acc.s[14], m_acc.s[15]);
	printf("acc_l: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_acc.s[16], m_acc.s[17], m_acc.s[18], m_acc.s[19], m_acc.s[20], m_acc.s[21], m_acc.s[22], m_acc.s[23]);
	printf("vcc_hi: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_flags[RSP_VCC].s[0], m_flags[RSP_VCC].s[1], m_flags[RSP_VCC].s[2], m_flags[RSP_VCC].s[3], m_flags[RSP_VCC].s[4], m_flags[RSP_VCC].s[5], m_flags[RSP_VCC].s[6], m_flags[RSP_VCC].s[7]);
	printf("vcc_lo: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_flags[RSP_VCC].s[8], m_flags[RSP_VCC].s[9], m_flags[RSP_VCC].s[10], m_flags[RSP_VCC].s[11], m_flags[RSP_VCC].s[12], m_flags[RSP_VCC].s[13], m_flags[RSP_VCC].s[14], m_flags[RSP_VCC].s[15]);
	printf("vco_hi: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_flags[RSP_VCO].s[0], m_flags[RSP_VCO].s[1], m_flags[RSP_VCO].s[2], m_flags[RSP_VCO].s[3], m_flags[RSP_VCO].s[4], m_flags[RSP_VCO].s[5], m_flags[RSP_VCO].s[6], m_flags[RSP_VCO].s[7]);
	printf("vco_lo: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_flags[RSP_VCO].s[8], m_flags[RSP_VCO].s[9], m_flags[RSP_VCO].s[10], m_flags[RSP_VCO].s[11], m_flags[RSP_VCO].s[12], m_flags[RSP_VCO].s[13], m_flags[RSP_VCO].s[14], m_flags[RSP_VCO].s[15]);
	printf("vce:    %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_flags[RSP_VCE].s[0], m_flags[RSP_VCE].s[1], m_flags[RSP_VCE].s[2], m_flags[RSP_VCE].s[3], m_flags[RSP_VCE].s[4], m_flags[RSP_VCE].s[5], m_flags[RSP_VCE].s[6], m_flags[RSP_VCE].s[7]);
#else
	printf("acc_h: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", ACCUM_H(0), ACCUM_H(1), ACCUM_H(2), ACCUM_H(3), ACCUM_H(4), ACCUM_H(5), ACCUM_H(6), ACCUM_H(7));
	printf("acc_m: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", ACCUM_M(0), ACCUM_M(1), ACCUM_M(2), ACCUM_M(3), ACCUM_M(4), ACCUM_M(5), ACCUM_M(6), ACCUM_M(7));
	printf("acc_l: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", ACCUM_L(0), ACCUM_L(1), ACCUM_L(2), ACCUM_L(3), ACCUM_L(4), ACCUM_L(5), ACCUM_L(6), ACCUM_L(7));
	printf("vcc_hi: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_vflag[4][0], m_vflag[4][1], m_vflag[4][2], m_vflag[4][3], m_vflag[4][4], m_vflag[4][5], m_vflag[4][6], m_vflag[4][7]);
	printf("vcc_lo: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_vflag[1][0], m_vflag[1][1], m_vflag[1][2], m_vflag[1][3], m_vflag[1][4], m_vflag[1][5], m_vflag[1][6], m_vflag[1][7]);
	printf("vco_hi: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_vflag[3][0], m_vflag[3][1], m_vflag[3][2], m_vflag[3][3], m_vflag[3][4], m_vflag[3][5], m_vflag[3][6], m_vflag[3][7]);
	printf("vco_lo: %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_vflag[0][0], m_vflag[0][1], m_vflag[0][2], m_vflag[0][3], m_vflag[0][4], m_vflag[0][5], m_vflag[0][6], m_vflag[0][7]);
	printf("vce:    %04x|%04x|%04x|%04x|%04x|%04x|%04x|%04x\n", m_vflag[2][0], m_vflag[2][1], m_vflag[2][2], m_vflag[2][3], m_vflag[2][4], m_vflag[2][5], m_vflag[2][6], m_vflag[2][7]);
#endif
}

void rsp_cop2::dump_dmem()
{
	UINT8* dmem = m_rsp.get_dmem();
	printf("\n");
	for (int i = 0; i < 0x1000; i += 32)
	{
		printf("%04x: ", i);
		for (int j = 0; j < 32; j++)
		{
			printf("%02x ", dmem[i + j]);
		}
		printf("\n");
	}
	printf("\n");
}
