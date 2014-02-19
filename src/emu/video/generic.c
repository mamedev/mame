/*********************************************************************

    generic.c

    Generic simple video functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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



/***************************************************************************
    COMMON PALETTE INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    black_and_white - basic 2-color black & white
-------------------------------------------------*/

const rgb_t RGB_MONOCHROME_WHITE[] =
{
	rgb_t::black,
	rgb_t::white
};

const rgb_t RGB_MONOCHROME_WHITE_HIGHLIGHT[] =
{
	rgb_t::black,
	rgb_t(0xc0, 0xc0, 0xc0),
	rgb_t::white
};


/*-------------------------------------------------
    monochrome_amber - 2-color black & amber
-------------------------------------------------*/

const rgb_t RGB_MONOCHROME_AMBER[] =
{
	rgb_t::black,
	rgb_t(0xf7, 0xaa, 0x00)
};


/*-------------------------------------------------
    monochrome_green - 2-color black & green
-------------------------------------------------*/

const rgb_t RGB_MONOCHROME_GREEN[] =
{
	rgb_t::black,
	rgb_t(0x00, 0xff, 0x00)
};

const rgb_t RGB_MONOCHROME_GREEN_HIGHLIGHT[] =
{
	rgb_t::black,
	rgb_t(0x00, 0xc0, 0x00),
	rgb_t(0x00, 0xff, 0x00)
};

/*-------------------------------------------------
    monochrome_yellow - 2-color black & yellow
-------------------------------------------------*/

const rgb_t RGB_MONOCHROME_YELLOW[] =
{
	rgb_t::black,
	rgb_t(0xff, 0xff, 0x00)
};
