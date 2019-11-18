// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*********************************************************************

    generic.c

    Generic simple video functions.

*********************************************************************/

#include "emu.h"



/***************************************************************************
    COMMON GRAPHICS LAYOUTS
***************************************************************************/

const gfx_layout gfx_8x8x1 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

const gfx_layout gfx_8x8x2_planar =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

const gfx_layout gfx_8x8x3_planar =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

const gfx_layout gfx_8x8x4_planar =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

const gfx_layout gfx_8x8x5_planar =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5), RGN_FRAC(3,5), RGN_FRAC(2,5), RGN_FRAC(1,5), RGN_FRAC(0,5) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

const gfx_layout gfx_8x8x6_planar =
{
	8,8,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(5,6), RGN_FRAC(4,6), RGN_FRAC(3,6), RGN_FRAC(2,6), RGN_FRAC(1,6), RGN_FRAC(0,6) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

const gfx_layout gfx_16x16x4_planar =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

// packed gfxs; msb and lsb is start nibble of packed pixel byte
const gfx_layout gfx_8x8x4_packed_msb =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) }, // x order : hi nibble first, low nibble second
	{ STEP8(0,4*8) },
	8*8*4
};

const gfx_layout gfx_8x8x4_packed_lsb =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 }, // x order : low nibble first, hi nibble second
	{ STEP8(0,4*8) },
	8*8*4
};

GFXLAYOUT_RAW(gfx_8x8x8_raw, 8, 8, 8*8, 8*8*8);

const gfx_layout gfx_16x16x4_packed_msb =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP16(0,4) }, // x order : hi nibble first, low nibble second
	{ STEP16(0,4*16) },
	16*16*4
};

const gfx_layout gfx_16x16x4_packed_lsb =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4, 9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 }, // x order : low nibble first, hi nibble second
	{ STEP16(0,4*16) },
	16*16*4
};

GFXLAYOUT_RAW(gfx_16x16x8_raw, 16, 16, 16*8, 16*16*8);

/*
    16x16; grouped of 4 8x8 tiles (row align)
    0 1
    2 3
*/
const gfx_layout gfx_8x8x4_row_2x2_group_packed_msb =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4), STEP8(4*8*8,4) }, // x order : hi nibble first, low nibble second
	{ STEP8(0,4*8), STEP8(4*8*8*2,4*8) },
	16*16*4
};

const gfx_layout gfx_8x8x4_row_2x2_group_packed_lsb =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ // x order : low nibble first, hi nibble second
		1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
		4*8*8+1*4, 4*8*8+0*4, 4*8*8+3*4, 4*8*8+2*4, 4*8*8+5*4, 4*8*8+4*4, 4*8*8+7*4, 4*8*8+6*4
	},
	{ STEP8(0,4*8), STEP8(4*8*8*2,4*8) },
	16*16*4
};

/*
    16x16; grouped of 4 8x8 tiles (col align)
    0 2
    1 3
*/
const gfx_layout gfx_8x8x4_col_2x2_group_packed_msb =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4), STEP8(4*8*16,4) }, // x order : hi nibble first, low nibble second
	{ STEP16(0,4*8) },
	16*16*4
};

const gfx_layout gfx_8x8x4_col_2x2_group_packed_lsb =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ // x order : low nibble first, hi nibble second
		1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
		4*8*16+1*4, 4*8*16+0*4, 4*8*16+3*4, 4*8*16+2*4, 4*8*16+5*4, 4*8*16+4*4, 4*8*16+7*4, 4*8*16+6*4
	},
	{ STEP16(0,4*8) },
	16*16*4
};
