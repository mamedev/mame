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
    black - completely black palette
-------------------------------------------------*/

PALETTE_INIT( all_black )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		palette_set_color(machine,i,RGB_BLACK); /* black */
	}
}


/*-------------------------------------------------
    black_and_white - basic 2-color black & white
-------------------------------------------------*/

PALETTE_INIT( black_and_white )
{
	palette_set_color(machine,0,RGB_BLACK); /* black */
	palette_set_color(machine,1,RGB_WHITE); /* white */
}


/*-------------------------------------------------
    monochrome_amber - 2-color black & amber
-------------------------------------------------*/

PALETTE_INIT( monochrome_amber )
{
	palette_set_color(machine, 0, RGB_BLACK); /* black */
	palette_set_color_rgb(machine, 1, 0xf7, 0xaa, 0x00); /* amber */
}


/*-------------------------------------------------
    monochrome_green - 2-color black & green
-------------------------------------------------*/

PALETTE_INIT( monochrome_green )
{
	palette_set_color(machine, 0, RGB_BLACK); /* black */
	palette_set_color_rgb(machine, 1, 0x00, 0xff, 0x00); /* green */
}


/*-------------------------------------------------
    RRRR_GGGG_BBBB - standard 4-4-4 palette,
    assuming the commonly used resistor values:

    bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
          -- 470 ohm resistor  -- RED/GREEN/BLUE
          -- 1  kohm resistor  -- RED/GREEN/BLUE
    bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE
-------------------------------------------------*/

PALETTE_INIT( RRRR_GGGG_BBBB )
{
	const UINT8 *color_prom = machine.region("proms")->base();
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[i + machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[i + machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[i + machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[i + machine.total_colors()] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[i + 2*machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*machine.total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}



/*-------------------------------------------------
    RRRRR_GGGGG_BBBBB/BBBBB_GGGGG_RRRRR -
    standard 5-5-5 palette for games using a
    15-bit color space
-------------------------------------------------*/

PALETTE_INIT( RRRRR_GGGGG_BBBBB )
{
	int i;

	for (i = 0; i < 0x8000; i++)
		palette_set_color(machine, i, MAKE_RGB(pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0)));
}


PALETTE_INIT( BBBBB_GGGGG_RRRRR )
{
	int i;

	for (i = 0; i < 0x8000; i++)
		palette_set_color(machine, i, MAKE_RGB(pal5bit(i >> 0), pal5bit(i >> 5), pal5bit(i >> 10)));
}



/*-------------------------------------------------
    RRRRR_GGGGGG_BBBBB -
    standard 5-6-5 palette for games using a
    16-bit color space
-------------------------------------------------*/

PALETTE_INIT( RRRRR_GGGGGG_BBBBB )
{
	int i;

	for (i = 0; i < 0x10000; i++)
		palette_set_color(machine, i, MAKE_RGB(pal5bit(i >> 11), pal6bit(i >> 5), pal5bit(i >> 0)));
}
