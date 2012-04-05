/*********************************************************************

    generic.c

    Generic simple video functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _generic_video_private
{
	int 		flip_screen_x;
	int			flip_screen_y;
};



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
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    set_color_332 - set a 3-3-2 RGB color using
    the 16-bit data provided and the specified
    shift values
-------------------------------------------------*/

void driver_device::set_color_332(pen_t color, int rshift, int gshift, int bshift, UINT32 data)
{
	palette_set_color(machine(), color, pal332(data, rshift, gshift, bshift));
}


/*-------------------------------------------------
    set_color_444 - set a 4-4-4 RGB color using
    the 16-bit data provided and the specified
    shift values
-------------------------------------------------*/

void driver_device::set_color_444(pen_t color, int rshift, int gshift, int bshift, UINT32 data)
{
	palette_set_color(machine(), color, pal444(data, rshift, gshift, bshift));
}


/*-------------------------------------------------
    set_color_4444 - set a 4-4-4-4 IRGB color using
    the 16-bit data provided and the specified
    shift values
-------------------------------------------------*/

void driver_device::set_color_4444(pen_t color, int ishift, int rshift, int gshift, int bshift, UINT16 data)
{
	static const UINT8 ztable[16] =
		{ 0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11 };
	int i, r, g, b;

	i = ztable[(data >> ishift) & 15];
	r = ((data >> rshift) & 15) * i;
	g = ((data >> gshift) & 15) * i;
	b = ((data >> bshift) & 15) * i;

	palette_set_color_rgb(machine(), color, r, g, b);
}


/*-------------------------------------------------
    set_color_555 - set a 5-5-5 RGB color using
    the 16-bit data provided and the specified
    shift values
-------------------------------------------------*/

void driver_device::set_color_555(pen_t color, int rshift, int gshift, int bshift, UINT32 data)
{
	palette_set_color(machine(), color, pal555(data, rshift, gshift, bshift));
}


/*-------------------------------------------------
    set_color_565 - set a 5-6-5 RGB color using
    the 16-bit data provided and the specified
    shift values
-------------------------------------------------*/

void driver_device::set_color_565(pen_t color, int rshift, int gshift, int bshift, UINT32 data)
{
	palette_set_color(machine(), color, pal565(data, rshift, gshift, bshift));
}


/*-------------------------------------------------
    set_color_8888 - set a 8-8-8 RGB color using
    the 32-bit data provided and the specified
    shift values
-------------------------------------------------*/

void driver_device::set_color_888(pen_t color, int rshift, int gshift, int bshift, UINT32 data)
{
	palette_set_color(machine(), color, pal888(data, rshift, gshift, bshift));
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    generic_video_init - initialize globals and
    register for save states
-------------------------------------------------*/

void generic_video_init(running_machine &machine)
{
	generic_video_private *state;

	state = machine.generic_video_data = auto_alloc_clear(machine, generic_video_private);

	machine.save().save_item(NAME(state->flip_screen_x));
	machine.save().save_item(NAME(state->flip_screen_y));
}



/***************************************************************************
    GLOBAL VIDEO ATTRIBUTES
***************************************************************************/

/*-------------------------------------------------
    updateflip - handle global flipping
-------------------------------------------------*/

static void updateflip(running_machine &machine)
{
	generic_video_private *state = machine.generic_video_data;
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();
	attoseconds_t period = machine.primary_screen->frame_period().attoseconds;
	rectangle visarea = machine.primary_screen->visible_area();

	machine.tilemap().set_flip_all((TILEMAP_FLIPX & state->flip_screen_x) | (TILEMAP_FLIPY & state->flip_screen_y));

	if (state->flip_screen_x)
	{
		int temp;

		temp = width - visarea.min_x - 1;
		visarea.min_x = width - visarea.max_x - 1;
		visarea.max_x = temp;
	}
	if (state->flip_screen_y)
	{
		int temp;

		temp = height - visarea.min_y - 1;
		visarea.min_y = height - visarea.max_y - 1;
		visarea.max_y = temp;
	}

	machine.primary_screen->configure(width, height, visarea, period);
}


/*-------------------------------------------------
    flip_screen_set - set global flip
-------------------------------------------------*/

void flip_screen_set(running_machine &machine, int on)
{
	generic_video_private *state = machine.generic_video_data;
	if (on) on = ~0;
	if (state->flip_screen_x != on || state->flip_screen_y != on)
	{
		if (!on) updateflip(machine); // flip visarea back
		state->flip_screen_x = state->flip_screen_y = on;
		updateflip(machine);
	}
}


/*-------------------------------------------------
    flip_screen_set_no_update - set global flip
       do not call update_flip.
-------------------------------------------------*/

void flip_screen_set_no_update(running_machine &machine, int on)
{
	/* flip_screen_y is not updated on purpose
     * this function is for drivers which
     * where writing to flip_screen_x to
     * bypass update_flip
     */
	generic_video_private *state = machine.generic_video_data;
	if (on) on = ~0;
	state->flip_screen_x = on;
}


/*-------------------------------------------------
    flip_screen_x_set - set global horizontal flip
-------------------------------------------------*/

void flip_screen_x_set(running_machine &machine, int on)
{
	generic_video_private *state = machine.generic_video_data;
	if (on) on = ~0;
	if (state->flip_screen_x != on)
	{
		state->flip_screen_x = on;
		updateflip(machine);
	}
}


/*-------------------------------------------------
    flip_screen_y_set - set global vertical flip
-------------------------------------------------*/

void flip_screen_y_set(running_machine &machine, int on)
{
	generic_video_private *state = machine.generic_video_data;
	if (on) on = ~0;
	if (state->flip_screen_y != on)
	{
		state->flip_screen_y = on;
		updateflip(machine);
	}
}


/*-------------------------------------------------
    flip_screen_get - get global flip
-------------------------------------------------*/

int flip_screen_get(running_machine &machine)
{
	generic_video_private *state = machine.generic_video_data;
	return state->flip_screen_x;
}


/*-------------------------------------------------
    flip_screen_x_get - get global x flip
-------------------------------------------------*/

int flip_screen_x_get(running_machine &machine)
{
	generic_video_private *state = machine.generic_video_data;
	return state->flip_screen_x;
}


/*-------------------------------------------------
    flip_screen_get - get global y flip
-------------------------------------------------*/

int flip_screen_y_get(running_machine &machine)
{
	generic_video_private *state = machine.generic_video_data;
	return state->flip_screen_y;
}



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



/***************************************************************************
    3-3-2 RGB PALETTE WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    RRR-GGG-BB writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_RRRGGGBB_w )
{
	m_generic_paletteram_8[offset] = data;
	palette_set_color_rgb(machine(), offset, pal3bit(data >> 5), pal3bit(data >> 2), pal2bit(data >> 0));
}


/*-------------------------------------------------
    BB-GGG-RR writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_BBGGGRRR_w )
{
	m_generic_paletteram_8[offset] = data;
	palette_set_color_rgb(machine(), offset, pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
}


/*-------------------------------------------------
    BB-GG-RR-II writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_BBGGRRII_w )
{
	int i = (data >> 0) & 3;

	m_generic_paletteram_8[offset] = data;
	palette_set_color_rgb(machine(), offset, pal4bit(((data >> 0) & 0x0c) | i),
	                                   pal4bit(((data >> 2) & 0x0c) | i),
	                                   pal4bit(((data >> 4) & 0x0c) | i));
}

/*-------------------------------------------------
    II-BB-GG-RR writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_IIBBGGRR_w )
{
	int i = (data >> 6) & 3;

	m_generic_paletteram_8[offset] = data;
	palette_set_color_rgb(machine(), offset, pal4bit(((data << 2) & 0x0c) | i),
	                                   pal4bit(((data >> 0) & 0x0c) | i),
	                                   pal4bit(((data >> 2) & 0x0c) | i));
}



/***************************************************************************
    4-4-4 RGB PALETTE WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    xxxx-BBBB-GGGG-RRRR writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBGGGGRRRR_le_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset / 2, 0, 4, 8, paletteram16_le(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBGGGGRRRR_be_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset / 2, 0, 4, 8, paletteram16_be(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBGGGGRRRR_split1_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset, 0, 4, 8, paletteram16_split(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBGGGGRRRR_split2_w )
{
	m_generic_paletteram2_8[offset] = data;
	set_color_444(offset, 0, 4, 8, paletteram16_split(offset));
}

WRITE16_MEMBER( driver_device::paletteram16_xxxxBBBBGGGGRRRR_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_444(offset, 0, 4, 8, m_generic_paletteram_16[offset]);
}


/*-------------------------------------------------
    xxxx-BBBB-RRRR-GGGG writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBRRRRGGGG_le_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset / 2, 4, 0, 8, paletteram16_le(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBRRRRGGGG_be_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset / 2, 4, 0, 8, paletteram16_be(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBRRRRGGGG_split1_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset, 4, 0, 8, paletteram16_split(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxBBBBRRRRGGGG_split2_w )
{
	m_generic_paletteram2_8[offset] = data;
	set_color_444(offset, 4, 0, 8, paletteram16_split(offset));
}

WRITE16_MEMBER( driver_device::paletteram16_xxxxBBBBRRRRGGGG_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_444(offset, 4, 0, 8, m_generic_paletteram_16[offset]);
}


/*-------------------------------------------------
    xxxx-RRRR-BBBB-GGGG writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRBBBBGGGG_split1_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset, 8, 0, 4, paletteram16_split(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRBBBBGGGG_split2_w )
{
	m_generic_paletteram2_8[offset] = data;
	set_color_444(offset, 8, 0, 4, paletteram16_split(offset));
}


/*-------------------------------------------------
    xxxx-RRRR-GGGG-BBBB writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRGGGGBBBB_le_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset / 2, 8, 4, 0, paletteram16_le(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRGGGGBBBB_be_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset / 2, 8, 4, 0, paletteram16_be(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRGGGGBBBB_split1_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset, 8, 4, 0, paletteram16_split(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xxxxRRRRGGGGBBBB_split2_w )
{
	m_generic_paletteram2_8[offset] = data;
	set_color_444(offset, 8, 4, 0, paletteram16_split(offset));
}

WRITE16_MEMBER( driver_device::paletteram16_xxxxRRRRGGGGBBBB_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_444(offset, 8, 4, 0, m_generic_paletteram_16[offset]);
}


/*-------------------------------------------------
    RRRR-GGGG-BBBB-xxxx writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_RRRRGGGGBBBBxxxx_be_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset / 2, 12, 8, 4, paletteram16_be(offset));
}

WRITE8_MEMBER( driver_device::paletteram_RRRRGGGGBBBBxxxx_split1_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_444(offset, 12, 8, 4, paletteram16_split(offset));
}

WRITE8_MEMBER( driver_device::paletteram_RRRRGGGGBBBBxxxx_split2_w )
{
	m_generic_paletteram2_8[offset] = data;
	set_color_444(offset, 12, 8, 4, paletteram16_split(offset));
}

WRITE16_MEMBER( driver_device::paletteram16_RRRRGGGGBBBBxxxx_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_444(offset, 12, 8, 4, m_generic_paletteram_16[offset]);
}



/***************************************************************************
    5-5-5 RGB PALETTE WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    x-BBBBB-GGGGG-RRRRR writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_xBBBBBGGGGGRRRRR_le_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_555(offset / 2, 0, 5, 10, paletteram16_le(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xBBBBBGGGGGRRRRR_be_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_555(offset / 2, 0, 5, 10, paletteram16_be(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xBBBBBGGGGGRRRRR_split1_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_555(offset, 0, 5, 10, paletteram16_split(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xBBBBBGGGGGRRRRR_split2_w )
{
	m_generic_paletteram2_8[offset] = data;
	set_color_555(offset, 0, 5, 10, paletteram16_split(offset));
}

WRITE16_MEMBER( driver_device::paletteram16_xBBBBBGGGGGRRRRR_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_555(offset, 0, 5, 10, m_generic_paletteram_16[offset]);
}


/*-------------------------------------------------
    x-BBBBB-RRRRR-GGGGG writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_xBBBBBRRRRRGGGGG_split1_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_555(offset, 5, 0, 10, paletteram16_split(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xBBBBBRRRRRGGGGG_split2_w )
{
	m_generic_paletteram2_8[offset] = data;
	set_color_555(offset, 5, 0, 10, paletteram16_split(offset));
}


/*-------------------------------------------------
    x-RRRRR-GGGGG-BBBBB writes
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_le_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_555(offset / 2, 10, 5, 0, paletteram16_le(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_be_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_555(offset / 2, 10, 5, 0, paletteram16_be(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_split1_w )
{
	m_generic_paletteram_8[offset] = data;
	set_color_555(offset, 10, 5, 0, paletteram16_split(offset));
}

WRITE8_MEMBER( driver_device::paletteram_xRRRRRGGGGGBBBBB_split2_w )
{
	m_generic_paletteram2_8[offset] = data;
	set_color_555(offset, 10, 5, 0, paletteram16_split(offset));
}

WRITE16_MEMBER( driver_device::paletteram16_xRRRRRGGGGGBBBBB_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_555(offset, 10, 5, 0, m_generic_paletteram_16[offset]);
}


/*-------------------------------------------------
    x-GGGGG-RRRRR-BBBBB writes
-------------------------------------------------*/

WRITE16_MEMBER( driver_device::paletteram16_xGGGGGRRRRRBBBBB_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_555(offset, 5, 10, 0, m_generic_paletteram_16[offset]);
}


/*-------------------------------------------------
    x-GGGGG-BBBBB-RRRRR writes
-------------------------------------------------*/

WRITE16_MEMBER( driver_device::paletteram16_xGGGGGBBBBBRRRRR_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_555(offset, 0, 10, 5, m_generic_paletteram_16[offset]);
}


/*-------------------------------------------------
    GGGGG-RRRRR-BBBBB-x writes
-------------------------------------------------*/

WRITE16_MEMBER( driver_device::paletteram16_GGGGGRRRRRBBBBBx_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_555(offset, 6, 11, 1, m_generic_paletteram_16[offset]);
}

/*-------------------------------------------------
    RRRRR-GGGGG-BBBBB-x writes
-------------------------------------------------*/

WRITE16_MEMBER( driver_device::paletteram16_RRRRRGGGGGBBBBBx_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_555(offset, 11, 6, 1, m_generic_paletteram_16[offset]);
}


/*-------------------------------------------------
    RRRR-GGGG-BBBB-RGBx writes
-------------------------------------------------*/

WRITE16_MEMBER( driver_device::paletteram16_RRRRGGGGBBBBRGBx_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	data = m_generic_paletteram_16[offset];
	palette_set_color_rgb(machine(), offset, pal5bit(((data >> 11) & 0x1e) | ((data >> 3) & 0x01)),
	                                       pal5bit(((data >>  7) & 0x1e) | ((data >> 2) & 0x01)),
	                                       pal5bit(((data >>  3) & 0x1e) | ((data >> 1) & 0x01)));
}



/***************************************************************************
    4-4-4-4 RGBI PALETTE WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    IIII-RRRR-GGGG-BBBB writes
-------------------------------------------------*/

WRITE16_MEMBER( driver_device::paletteram16_IIIIRRRRGGGGBBBB_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_4444(offset, 12, 8, 4, 0, m_generic_paletteram_16[offset]);
}


/*-------------------------------------------------
    RRRR-GGGG-BBBB-IIII writes
-------------------------------------------------*/

WRITE16_MEMBER( driver_device::paletteram16_RRRRGGGGBBBBIIII_word_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_4444(offset, 0, 12, 8, 4, m_generic_paletteram_16[offset]);
}



/***************************************************************************
    8-8-8 RGB PALETTE WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    xxxxxxxx-RRRRRRRR-GGGGGGGG-BBBBBBBB writes
-------------------------------------------------*/

WRITE16_MEMBER( driver_device::paletteram16_xrgb_word_be_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_888(offset / 2, 16, 8, 0, paletteram32_be(offset));
}


/*-------------------------------------------------
    xxxxxxxx-BBBBBBBB-GGGGGGGG-RRRRRRRR writes
-------------------------------------------------*/

WRITE16_MEMBER( driver_device::paletteram16_xbgr_word_be_w )
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	set_color_888(offset / 2, 0, 8, 16, paletteram32_be(offset));
}
