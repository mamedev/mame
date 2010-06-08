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
    paletteram16_le - return a 16-bit value
    assembled from the two bytes of little-endian
    palette RAM referenced by offset
-------------------------------------------------*/

INLINE UINT16 paletteram16_le(running_machine *machine, offs_t offset)
{
	return machine->generic.paletteram.u8[offset & ~1] | (machine->generic.paletteram.u8[offset | 1] << 8);
}


/*-------------------------------------------------
    paletteram16_be - return a 16-bit value
    assembled from the two bytes of big-endian
    palette RAM referenced by offset
-------------------------------------------------*/

INLINE UINT16 paletteram16_be(running_machine *machine, offs_t offset)
{
	return machine->generic.paletteram.u8[offset | 1] | (machine->generic.paletteram.u8[offset & ~1] << 8);
}


/*-------------------------------------------------
    paletteram16_split - return a 16-bit value
    assembled from the two bytes of split palette
    RAM referenced by offset
-------------------------------------------------*/

INLINE UINT16 paletteram16_split(running_machine *machine, offs_t offset)
{
	return machine->generic.paletteram.u8[offset] | (machine->generic.paletteram2.u8[offset] << 8);
}


/*-------------------------------------------------
    paletteram32_be - return a 32-bit value
    assembled from the two words of big-endian
    palette RAM referenced by offset
-------------------------------------------------*/

INLINE UINT32 paletteram32_be(running_machine *machine, offs_t offset)
{
	return machine->generic.paletteram.u16[offset | 1] | (machine->generic.paletteram.u16[offset & ~1] << 16);
}


/*-------------------------------------------------
    set_color_444 - set a 4-4-4 RGB color using
    the 16-bit data provided and the specified
    shift values
-------------------------------------------------*/

INLINE void set_color_444(running_machine *machine, pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	palette_set_color_rgb(machine, color, pal4bit(data >> rshift), pal4bit(data >> gshift), pal4bit(data >> bshift));
}


/*-------------------------------------------------
    set_color_4444 - set a 4-4-4-4 IRGB color using
    the 16-bit data provided and the specified
    shift values
-------------------------------------------------*/

INLINE void set_color_4444(running_machine *machine, pen_t color, int ishift, int rshift, int gshift, int bshift, UINT16 data)
{
	static const UINT8 ztable[16] =
		{ 0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11 };
	int i, r, g, b;

	i = ztable[(data >> ishift) & 15];
	r = ((data >> rshift) & 15) * i;
	g = ((data >> gshift) & 15) * i;
	b = ((data >> bshift) & 15) * i;

	palette_set_color_rgb(machine, color, r, g, b);
}


/*-------------------------------------------------
    set_color_555 - set a 5-5-5 RGB color using
    the 16-bit data provided and the specified
    shift values
-------------------------------------------------*/

INLINE void set_color_555(running_machine *machine, pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	palette_set_color_rgb(machine, color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}


/*-------------------------------------------------
    set_color_8888 - set a 8-8-8 RGB color using
    the 32-bit data provided and the specified
    shift values
-------------------------------------------------*/

INLINE void set_color_888(running_machine *machine, pen_t color, int rshift, int gshift, int bshift, UINT32 data)
{
	palette_set_color_rgb(machine, color, (data >> rshift) & 0xff, (data >> gshift) & 0xff, (data >> bshift) & 0xff);
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    generic_video_init - initialize globals and
    register for save states
-------------------------------------------------*/

void generic_video_init(running_machine *machine)
{
	generic_video_private *state;

	state = machine->generic_video_data = auto_alloc_clear(machine, generic_video_private);

	state_save_register_item(machine, "video", NULL, 0, state->flip_screen_x);
	state_save_register_item(machine, "video", NULL, 0, state->flip_screen_y);
}



/***************************************************************************
    GENERIC VIDEO START/UPDATE
***************************************************************************/

/*-------------------------------------------------
    VIDEO_START( generic_bitmapped ) - general video
    system with a bitmap
-------------------------------------------------*/

VIDEO_START( generic_bitmapped )
{
	/* allocate the temporary bitmap */
	machine->generic.tmpbitmap = machine->primary_screen->alloc_compatible_bitmap();

	/* ensure the contents of the bitmap are saved */
	state_save_register_global_bitmap(machine, machine->generic.tmpbitmap);
}


/*-------------------------------------------------
    VIDEO_UPDATE( generic_bitmapped ) - blast the
    generic bitmap to the screen
-------------------------------------------------*/

VIDEO_UPDATE( generic_bitmapped )
{
	copybitmap(bitmap, screen->machine->generic.tmpbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}



/***************************************************************************
    GENERIC SPRITE BUFFERING
***************************************************************************/

/* Mish:  171099

    'Buffered spriteram' is where the graphics hardware draws the sprites
from private ram that the main CPU cannot access.  The main CPU typically
prepares sprites for the next frame in it's own sprite ram as the graphics
hardware renders sprites for the current frame from private ram.  Main CPU
sprite ram is usually copied across to private ram by setting some flag
in the VBL interrupt routine.

    The reason for this is to avoid sprite flicker or lag - if a game
is unable to prepare sprite ram within a frame (for example, lots of sprites
on screen) then it doesn't trigger the buffering hardware - instead the
graphics hardware will use the sprites from the last frame. An example is
Dark Seal - the buffer flag is only written to if the CPU is idle at the time
of the VBL interrupt.  If the buffering is not emulated the sprites flicker
at busy scenes.

    Some games seem to use buffering because of hardware constraints -
Capcom games (Cps1, Last Duel, etc) render spriteram _1 frame ahead_ and
buffer this spriteram at the end of a frame, so the _next_ frame must be drawn
from the buffer.  Presumably the graphics hardware and the main cpu cannot
share the same spriteram for whatever reason.

    Sprite buffering & Mame:

    To use sprite buffering in a driver use VIDEO_BUFFERS_SPRITERAM in the
machine driver.  This will automatically create an area for buffered spriteram
equal to the size of normal spriteram.

    Spriteram size _must_ be declared in the memory map:

    AM_RANGE(0x120000, 0x1207ff) AM_RAMBANK("sprites") AM_BASE_SIZE_GENERIC(spriteram)

    Then the video driver must draw the sprites from the buffered_spriteram
pointer.  The function buffer_spriteram_w() is used to simulate hardware
which buffers the spriteram from a memory location write.  The function
buffer_spriteram(UINT8 *ptr, int length) can be used where
more control is needed over what is buffered.

    For examples see darkseal.c, contra.c, lastduel.c, bionicc.c etc.

*/


/*-------------------------------------------------
    buffer_spriteram_w - triggered writes to
    buffer spriteram
-------------------------------------------------*/

WRITE8_HANDLER( buffer_spriteram_w )
{
	memcpy(space->machine->generic.buffered_spriteram.u8, space->machine->generic.spriteram.u8, space->machine->generic.spriteram_size);
}

WRITE16_HANDLER( buffer_spriteram16_w )
{
	memcpy(space->machine->generic.buffered_spriteram.u16, space->machine->generic.spriteram.u16, space->machine->generic.spriteram_size);
}

WRITE32_HANDLER( buffer_spriteram32_w )
{
	memcpy(space->machine->generic.buffered_spriteram.u32, space->machine->generic.spriteram.u32, space->machine->generic.spriteram_size);
}


/*-------------------------------------------------
    buffer_spriteram_2_w - triggered writes to
    buffer spriteram_2
-------------------------------------------------*/

WRITE8_HANDLER( buffer_spriteram_2_w )
{
	memcpy(space->machine->generic.buffered_spriteram2.u8, space->machine->generic.spriteram2.u8, space->machine->generic.spriteram2_size);
}

WRITE16_HANDLER( buffer_spriteram16_2_w )
{
	memcpy(space->machine->generic.buffered_spriteram2.u16, space->machine->generic.spriteram2.u16, space->machine->generic.spriteram2_size);
}

WRITE32_HANDLER( buffer_spriteram32_2_w )
{
	memcpy(space->machine->generic.buffered_spriteram2.u32, space->machine->generic.spriteram2.u32, space->machine->generic.spriteram2_size);
}


/*-------------------------------------------------
    buffer_spriteram - for manually buffering
    spriteram
-------------------------------------------------*/

void buffer_spriteram(running_machine *machine, UINT8 *ptr, int length)
{
	memcpy(machine->generic.buffered_spriteram.u8, ptr, length);
}

void buffer_spriteram_2(running_machine *machine, UINT8 *ptr, int length)
{
	memcpy(machine->generic.buffered_spriteram2.u8, ptr, length);
}



/***************************************************************************
    GLOBAL VIDEO ATTRIBUTES
***************************************************************************/

/*-------------------------------------------------
    updateflip - handle global flipping
-------------------------------------------------*/

static void updateflip(running_machine *machine)
{
	generic_video_private *state = machine->generic_video_data;
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();
	attoseconds_t period = machine->primary_screen->frame_period().attoseconds;
	rectangle visarea = machine->primary_screen->visible_area();

	tilemap_set_flip_all(machine,(TILEMAP_FLIPX & state->flip_screen_x) | (TILEMAP_FLIPY & state->flip_screen_y));

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

	machine->primary_screen->configure(width, height, visarea, period);
}


/*-------------------------------------------------
    flip_screen_set - set global flip
-------------------------------------------------*/

void flip_screen_set(running_machine *machine, int on)
{
	flip_screen_x_set(machine, on);
	flip_screen_y_set(machine, on);
}


/*-------------------------------------------------
    flip_screen_set_no_update - set global flip
       do not call update_flip.
-------------------------------------------------*/

void flip_screen_set_no_update(running_machine *machine, int on)
{
	/* flip_screen_y is not updated on purpose
     * this function is for drivers which
     * where writing to flip_screen_x to
     * bypass update_flip
     */
	generic_video_private *state = machine->generic_video_data;
	state->flip_screen_x = on;
}


/*-------------------------------------------------
    flip_screen_x_set - set global horizontal flip
-------------------------------------------------*/

void flip_screen_x_set(running_machine *machine, int on)
{
	generic_video_private *state = machine->generic_video_data;
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

void flip_screen_y_set(running_machine *machine, int on)
{
	generic_video_private *state = machine->generic_video_data;
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

int flip_screen_get(running_machine *machine)
{
	generic_video_private *state = machine->generic_video_data;
	return state->flip_screen_x;
}


/*-------------------------------------------------
    flip_screen_x_get - get global x flip
-------------------------------------------------*/

int flip_screen_x_get(running_machine *machine)
{
	generic_video_private *state = machine->generic_video_data;
	return state->flip_screen_x;
}


/*-------------------------------------------------
    flip_screen_get - get global y flip
-------------------------------------------------*/

int flip_screen_y_get(running_machine *machine)
{
	generic_video_private *state = machine->generic_video_data;
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

	for (i = 0; i < machine->config->total_colors; i++)
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

	for (i = 0; i < machine->config->total_colors; i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[i + machine->config->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + machine->config->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + machine->config->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + machine->config->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[i + 2*machine->config->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*machine->config->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*machine->config->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*machine->config->total_colors] >> 3) & 0x01;
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

WRITE8_HANDLER( paletteram_RRRGGGBB_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	palette_set_color_rgb(space->machine, offset, pal3bit(data >> 5), pal3bit(data >> 2), pal2bit(data >> 0));
}


/*-------------------------------------------------
    BB-GGG-RR writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_BBGGGRRR_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	palette_set_color_rgb(space->machine, offset, pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
}


/*-------------------------------------------------
    BB-GG-RR-II writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_BBGGRRII_w )
{
	int i = (data >> 0) & 3;

	space->machine->generic.paletteram.u8[offset] = data;
	palette_set_color_rgb(space->machine, offset, pal4bit(((data >> 0) & 0x0c) | i),
	                                   pal4bit(((data >> 2) & 0x0c) | i),
	                                   pal4bit(((data >> 4) & 0x0c) | i));
}

/*-------------------------------------------------
    II-BB-GG-RR writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_IIBBGGRR_w )
{
	int i = (data >> 6) & 3;

	space->machine->generic.paletteram.u8[offset] = data;
	palette_set_color_rgb(space->machine, offset, pal4bit(((data << 2) & 0x0c) | i),
	                                   pal4bit(((data >> 0) & 0x0c) | i),
	                                   pal4bit(((data >> 2) & 0x0c) | i));
}



/***************************************************************************
    4-4-4 RGB PALETTE WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    xxxx-BBBB-GGGG-RRRR writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_le_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset / 2, 0, 4, 8, paletteram16_le(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_be_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset / 2, 0, 4, 8, paletteram16_be(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_split1_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset, 0, 4, 8, paletteram16_split(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxBBBBGGGGRRRR_split2_w )
{
	space->machine->generic.paletteram2.u8[offset] = data;
	set_color_444(space->machine, offset, 0, 4, 8, paletteram16_split(space->machine, offset));
}

WRITE16_HANDLER( paletteram16_xxxxBBBBGGGGRRRR_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_444(space->machine, offset, 0, 4, 8, space->machine->generic.paletteram.u16[offset]);
}


/*-------------------------------------------------
    xxxx-BBBB-RRRR-GGGG writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_le_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset / 2, 4, 0, 8, paletteram16_le(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_be_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset / 2, 4, 0, 8, paletteram16_be(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_split1_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset, 4, 0, 8, paletteram16_split(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxBBBBRRRRGGGG_split2_w )
{
	space->machine->generic.paletteram2.u8[offset] = data;
	set_color_444(space->machine, offset, 4, 0, 8, paletteram16_split(space->machine, offset));
}

WRITE16_HANDLER( paletteram16_xxxxBBBBRRRRGGGG_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_444(space->machine, offset, 4, 0, 8, space->machine->generic.paletteram.u16[offset]);
}


/*-------------------------------------------------
    xxxx-RRRR-BBBB-GGGG writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_xxxxRRRRBBBBGGGG_split1_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset, 8, 0, 4, paletteram16_split(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxRRRRBBBBGGGG_split2_w )
{
	space->machine->generic.paletteram2.u8[offset] = data;
	set_color_444(space->machine, offset, 8, 0, 4, paletteram16_split(space->machine, offset));
}


/*-------------------------------------------------
    xxxx-RRRR-GGGG-BBBB writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_le_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset / 2, 8, 4, 0, paletteram16_le(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_be_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset / 2, 8, 4, 0, paletteram16_be(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_split1_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset, 8, 4, 0, paletteram16_split(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xxxxRRRRGGGGBBBB_split2_w )
{
	space->machine->generic.paletteram2.u8[offset] = data;
	set_color_444(space->machine, offset, 8, 4, 0, paletteram16_split(space->machine, offset));
}

WRITE16_HANDLER( paletteram16_xxxxRRRRGGGGBBBB_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_444(space->machine, offset, 8, 4, 0, space->machine->generic.paletteram.u16[offset]);
}


/*-------------------------------------------------
    RRRR-GGGG-BBBB-xxxx writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_RRRRGGGGBBBBxxxx_be_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset / 2, 12, 8, 4, paletteram16_be(space->machine, offset));
}

WRITE8_HANDLER( paletteram_RRRRGGGGBBBBxxxx_split1_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_444(space->machine, offset, 12, 8, 4, paletteram16_split(space->machine, offset));
}

WRITE8_HANDLER( paletteram_RRRRGGGGBBBBxxxx_split2_w )
{
	space->machine->generic.paletteram2.u8[offset] = data;
	set_color_444(space->machine, offset, 12, 8, 4, paletteram16_split(space->machine, offset));
}

WRITE16_HANDLER( paletteram16_RRRRGGGGBBBBxxxx_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_444(space->machine, offset, 12, 8, 4, space->machine->generic.paletteram.u16[offset]);
}



/***************************************************************************
    5-5-5 RGB PALETTE WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    x-BBBBB-GGGGG-RRRRR writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_le_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_555(space->machine, offset / 2, 0, 5, 10, paletteram16_le(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_be_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_555(space->machine, offset / 2, 0, 5, 10, paletteram16_be(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_split1_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_555(space->machine, offset, 0, 5, 10, paletteram16_split(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xBBBBBGGGGGRRRRR_split2_w )
{
	space->machine->generic.paletteram2.u8[offset] = data;
	set_color_555(space->machine, offset, 0, 5, 10, paletteram16_split(space->machine, offset));
}

WRITE16_HANDLER( paletteram16_xBBBBBGGGGGRRRRR_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_555(space->machine, offset, 0, 5, 10, space->machine->generic.paletteram.u16[offset]);
}


/*-------------------------------------------------
    x-BBBBB-RRRRR-GGGGG writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_xBBBBBRRRRRGGGGG_split1_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_555(space->machine, offset, 5, 0, 10, paletteram16_split(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xBBBBBRRRRRGGGGG_split2_w )
{
	space->machine->generic.paletteram2.u8[offset] = data;
	set_color_555(space->machine, offset, 5, 0, 10, paletteram16_split(space->machine, offset));
}


/*-------------------------------------------------
    x-RRRRR-GGGGG-BBBBB writes
-------------------------------------------------*/

WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_le_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_555(space->machine, offset / 2, 10, 5, 0, paletteram16_le(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_be_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_555(space->machine, offset / 2, 10, 5, 0, paletteram16_be(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_split1_w )
{
	space->machine->generic.paletteram.u8[offset] = data;
	set_color_555(space->machine, offset, 10, 5, 0, paletteram16_split(space->machine, offset));
}

WRITE8_HANDLER( paletteram_xRRRRRGGGGGBBBBB_split2_w )
{
	space->machine->generic.paletteram2.u8[offset] = data;
	set_color_555(space->machine, offset, 10, 5, 0, paletteram16_split(space->machine, offset));
}

WRITE16_HANDLER( paletteram16_xRRRRRGGGGGBBBBB_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_555(space->machine, offset, 10, 5, 0, space->machine->generic.paletteram.u16[offset]);
}


/*-------------------------------------------------
    x-GGGGG-RRRRR-BBBBB writes
-------------------------------------------------*/

WRITE16_HANDLER( paletteram16_xGGGGGRRRRRBBBBB_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_555(space->machine, offset, 5, 10, 0, space->machine->generic.paletteram.u16[offset]);
}


/*-------------------------------------------------
    x-GGGGG-BBBBB-RRRRR writes
-------------------------------------------------*/

WRITE16_HANDLER( paletteram16_xGGGGGBBBBBRRRRR_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_555(space->machine, offset, 0, 10, 5, space->machine->generic.paletteram.u16[offset]);
}


/*-------------------------------------------------
    GGGGG-RRRRR-BBBBB-x writes
-------------------------------------------------*/

WRITE16_HANDLER( paletteram16_GGGGGRRRRRBBBBBx_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_555(space->machine, offset, 6, 11, 1, space->machine->generic.paletteram.u16[offset]);
}

/*-------------------------------------------------
    RRRRR-GGGGG-BBBBB-x writes
-------------------------------------------------*/

WRITE16_HANDLER( paletteram16_RRRRRGGGGGBBBBBx_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_555(space->machine, offset, 11, 6, 1, space->machine->generic.paletteram.u16[offset]);
}


/*-------------------------------------------------
    RRRR-GGGG-BBBB-RGBx writes
-------------------------------------------------*/

WRITE16_HANDLER( paletteram16_RRRRGGGGBBBBRGBx_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	data = space->machine->generic.paletteram.u16[offset];
	palette_set_color_rgb(space->machine, offset, pal5bit(((data >> 11) & 0x1e) | ((data >> 3) & 0x01)),
	                                       pal5bit(((data >>  7) & 0x1e) | ((data >> 2) & 0x01)),
	                                       pal5bit(((data >>  3) & 0x1e) | ((data >> 1) & 0x01)));
}



/***************************************************************************
    4-4-4-4 RGBI PALETTE WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    IIII-RRRR-GGGG-BBBB writes
-------------------------------------------------*/

WRITE16_HANDLER( paletteram16_IIIIRRRRGGGGBBBB_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_4444(space->machine, offset, 12, 8, 4, 0, space->machine->generic.paletteram.u16[offset]);
}


/*-------------------------------------------------
    RRRR-GGGG-BBBB-IIII writes
-------------------------------------------------*/

WRITE16_HANDLER( paletteram16_RRRRGGGGBBBBIIII_word_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_4444(space->machine, offset, 0, 12, 8, 4, space->machine->generic.paletteram.u16[offset]);
}



/***************************************************************************
    8-8-8 RGB PALETTE WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    xxxxxxxx-RRRRRRRR-GGGGGGGG-BBBBBBBB writes
-------------------------------------------------*/

WRITE16_HANDLER( paletteram16_xrgb_word_be_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_888(space->machine, offset / 2, 16, 8, 0, paletteram32_be(space->machine, offset));
}


/*-------------------------------------------------
    xxxxxxxx-BBBBBBBB-GGGGGGGG-RRRRRRRR writes
-------------------------------------------------*/

WRITE16_HANDLER( paletteram16_xbgr_word_be_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	set_color_888(space->machine, offset / 2, 0, 8, 16, paletteram32_be(space->machine, offset));
}
