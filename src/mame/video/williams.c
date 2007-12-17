/***************************************************************************

    Williams 6809 system

****************************************************************************

    The basic video system involves a 4-bit-per-pixel bitmap, oriented
    in inverted X/Y order. That is, pixels (0,0) and (1,0) come from the
    byte at offset 0. Pixels (2,0) and (3,0) come from the byte at offset
    256. Pixels (4,0) and (5,0) come from the byte at offset 512. Etc.

    Defender and Stargate simply draw graphics to the framebuffer directly
    with no extra intervention.

    Later games added a pair of "special chips" (SC-01) to the board which
    are special purpose blitters. During their operation they HALT the
    main CPU so that they can control the busses. The operation of the
    chips is described in detail below.

    The original SC-01 had a bug that forced an XOR of the width and height
    values with 4. This was fixed in the SC-02, which was used on  several
    later games.

    Beginning with Sinistar, additional video tweaks were added.

    In Sinistar, a clipping window can be specified and enabled in order
    to prevent the blitter chip from drawing beyond a certain address.
    This clipping window can be switched on and off at will.

    In Blaster, a number of features were added. First, a fixed window can
    be enabled which cuts off blitter drawing at 0x9700. Second, on a
    per-scanline basis, an "erase behind" feature can be turned on which
    clears the video RAM to 0 after it is refreshed to the screen. Third,
    on a per-scanline basis, an alternate color can be latched as the new
    background color.

    For Mystic Marathon and the 3 other "2nd generation" Williams games,
    a tilemap background layer was added. This layer consisted of 24x16
    tiles and only scrolled in the X direction. In addition, the palette
    was expanded to 1024 entries, some of which were used for the tilemap.
    The traditional foreground bitmap could be configured to use any bank
    of 16 colors from the full palette.

****************************************************************************

    Blitter description from Sean Riddle's page:

    This page contains information about the Williams Special Chips, which
    were 'bit blitters'- block transfer chips that could move data around on
    the screen and in memory faster than the CPU. In fact, I've timed the
    special chips at 16 megs in 18.1 seconds. That's 910K/sec, not bad for
    the early 80s.

    The blitters were not used in Defender and Stargate, but
    were added to the ROM boards of the later games. Splat!, Blaster, Mystic
    Marathon and Joust 2 used Special Chip 2s. The only difference that I've
    seen is that SC1s have a small bug. When you tell the SC1 the size of
    the data to move, you have to exclusive-or the width and height with 2.
    The SC2s eliminate this bug.

    The blitters were accessed at memory location $CA00-CA06.

    CA01 is the mask, usually $FF to move all bits.
    CA02-3 is the source data location.
    CA04-5 is the destination data location.

    Writing to CA00 starts the blit, and the byte written determines how the
    data is blitted.

    Bit 0 indicates that the source data is either laid out linear, one
    pixel after the last, or in screen format, where there are 256 bytes from
    one pair of pixels to the next.

    Bit 1 indicates the same, but for the destination data.

    I'm not sure what bit 2 does. Looking at the image, I can't tell, but
    perhaps it has to do with the mask. My test files only used a mask of $FF.

    Bit 3 tells the blitter only to blit the foreground- that is, everything
    that is not color 0. Also known as transparency mode.

    Bit 4 is 'solid' mode. Only the color indicated by the mask is blitted.
    Note that this just creates a rectangle unless bit 3 is also set, in which
    case it blits the image, but in a solid color.

    Bit 5 shifts the image one pixel to the right. Any data on the far right
    jumps to the far left.

    Bits 6 and 7 only blit every other pixel of the image. Bit 6 says even only,
    while bit 7 says odd only.

***************************************************************************/

#include "driver.h"
#include "video/resnet.h"
#include "williams.h"



/*************************************
 *
 *  Global variables
 *
 *************************************/

/* RAM globals */
UINT8 *williams_videoram;
UINT8 *williams2_tileram;
UINT8 *blaster_palette_0;
UINT8 *blaster_scanline_control;

/* blitter globals */
UINT8 williams_blitter_config;
UINT16 williams_blitter_clip_address;
UINT8 williams_blitter_window_enable;

/* tilemap globals */
UINT8 williams2_tilemap_config;

/* rendering globals */
UINT8 williams_cocktail;



/*************************************
 *
 *  Static global variables
 *
 *************************************/

/* RAM variable */
static UINT8 *williams2_paletteram;

/* palette variables */
static rgb_t *palette_lookup;

/* blitter variables */
static UINT8 blitterram[8];
static UINT8 blitter_xor;
static UINT8 blitter_remap_index;
static const UINT8 *blitter_remap;
static UINT8 *blitter_remap_lookup;

/* Blaster-specific variables */
static UINT8 blaster_color0;
static UINT8 blaster_video_control;

/* tilemap variables */
static tilemap *bg_tilemap;
static UINT16 tilemap_xscroll;
static UINT8 williams2_fg_color;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void blitter_init(int blitter_config, const UINT8 *remap_prom);
static void create_palette_lookup(void);
static TILE_GET_INFO( get_tile_info );
static int blitter_core(int sstart, int dstart, int w, int h, int data);



/*************************************
 *
 *  Williams video startup
 *
 *************************************/

static void state_save_register(void)
{
	state_save_register_global(williams_blitter_window_enable);
	state_save_register_global(williams_cocktail);
	state_save_register_global_array(blitterram);
	state_save_register_global(blitter_remap_index);
	state_save_register_global(blaster_color0);
	state_save_register_global(blaster_video_control);
	state_save_register_global(tilemap_xscroll);
	state_save_register_global(williams2_fg_color);
}


VIDEO_START( williams )
{
	blitter_init(williams_blitter_config, NULL);
	create_palette_lookup();
	state_save_register();
}


VIDEO_START( blaster )
{
	blitter_init(williams_blitter_config, memory_region(REGION_PROMS));
	create_palette_lookup();
	state_save_register();
}


VIDEO_START( williams2 )
{
	blitter_init(williams_blitter_config, NULL);

	/* allocate paletteram */
	williams2_paletteram = auto_malloc(0x400 * 2);
	state_save_register_global_pointer(williams2_paletteram, 0x400 * 2);

	/* create the tilemap */
	bg_tilemap = tilemap_create(get_tile_info, tilemap_scan_cols, TILEMAP_TYPE_PEN, 24,16, 128,16);
	tilemap_set_scrolldx(bg_tilemap, 2, 0);

	state_save_register();
}



/*************************************
 *
 *  Williams video update
 *
 *************************************/

VIDEO_UPDATE( williams )
{
	int x, y;

	/* loop over rows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT8 *source = &williams_videoram[y];
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);

		/* loop over columns */
		for (x = cliprect->min_x & ~1; x <= cliprect->max_x; x += 2)
		{
			int pix = source[(x/2) * 256];
			dest[x+0] = pix >> 4;
			dest[x+1] = pix & 0x0f;
		}
	}
	return 0;
}


VIDEO_UPDATE( blaster )
{
	int x, y;

	/* if we're blitting from the top, start with a 0 for color 0 */
	if (cliprect->min_y == machine->screen[0].visarea.min_y || !(blaster_video_control & 1))
		blaster_color0 = 0;

	/* loop over rows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int erase_behind = blaster_video_control & blaster_scanline_control[y] & 2;
		UINT8 *source = &williams_videoram[y];
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);

		/* latch a new color0 pen? */
		if (blaster_video_control & blaster_scanline_control[y] & 1)
			blaster_color0 = 16 + y;

		/* loop over columns */
		for (x = cliprect->min_x & ~1; x <= cliprect->max_x; x += 2)
		{
			int pix = source[(x/2) * 256];

			/* clear behind us if requested */
			if (erase_behind)
				source[(x/2) * 256] = 0;

			/* now draw */
			dest[x+0] = (pix & 0xf0) ? (pix >> 4) : blaster_color0;
			dest[x+1] = (pix & 0x0f) ? (pix & 0x0f) : blaster_color0;
		}
	}
	return 0;
}


VIDEO_UPDATE( williams2 )
{
	int x, y, basecolor;

	/* draw the background */
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	/* copy the bitmap data on top of that */
	basecolor = williams2_fg_color * 16;

	/* loop over rows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT8 *source = &williams_videoram[y];
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);

		/* loop over columns */
		for (x = cliprect->min_x & ~1; x <= cliprect->max_x; x += 2)
		{
			int pix = source[(x/2) * 256];

			if (pix & 0xf0)
				dest[x+0] = basecolor + (pix >> 4);
			if (pix & 0x0f)
				dest[x+1] = basecolor + (pix & 0x0f);
		}
	}
	return 0;
}



/*************************************
 *
 *  Williams palette I/O
 *
 *************************************/

static void create_palette_lookup(void)
{
	static const int resistances_rg[3] = { 1200, 560, 330 };
	static const int resistances_b[2]  = { 560, 330 };
	double weights_r[3], weights_g[3], weights_b[2];
	int i;

	/* compute palette information */
	/* note that there really are pullup/pulldown resistors, but this situation is complicated */
	/* by the use of transistors, so we ignore that and just use the realtive resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, resistances_rg, weights_r, 0, 0,
			3, resistances_rg, weights_g, 0, 0,
			2, resistances_b,  weights_b, 0, 0);

	/* build a palette lookup */
	palette_lookup = auto_malloc(256 * sizeof(*palette_lookup));
	for (i = 0; i < 256; i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (i >> 0) & 0x01;
		bit1 = (i >> 1) & 0x01;
		bit2 = (i >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);

		/* green component */
		bit0 = (i >> 3) & 0x01;
		bit1 = (i >> 4) & 0x01;
		bit2 = (i >> 5) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);

		/* blue component */
		bit0 = (i >> 6) & 0x01;
		bit1 = (i >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_lookup[i] = MAKE_RGB(r, g, b);
	}
}


WRITE8_HANDLER( williams_paletteram_w )
{
	paletteram[offset] = data;
	palette_set_color(Machine, offset, palette_lookup[data]);
}


READ8_HANDLER( williams2_paletteram_r )
{
	return williams2_paletteram[offset];
}


WRITE8_HANDLER( williams2_paletteram_w )
{
	static const UINT8 ztable[16] =
	{
		0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,  0x9,
		0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11
	};
	UINT8 entry_lo, entry_hi, i, r, g, b;

	/* set the new value */
	williams2_paletteram[offset] = data;

	/* pull the associated low/high bytes */
	entry_lo = williams2_paletteram[offset & ~1];
	entry_hi = williams2_paletteram[offset |  1];

	/* update the palette entry */
	i = ztable[(entry_hi >> 4) & 15];
	b = ((entry_hi >> 0) & 15) * i;
	g = ((entry_lo >> 4) & 15) * i;
	r = ((entry_lo >> 0) & 15) * i;
	palette_set_color(Machine, offset / 2, MAKE_RGB(r, g, b));
}


WRITE8_HANDLER( williams2_fg_select_w )
{
	video_screen_update_partial(0, video_screen_get_vpos(0));
	williams2_fg_color = data & 0x3f;
}



/*************************************
 *
 *  Video position readout
 *
 *************************************/

READ8_HANDLER( williams_video_counter_r )
{
	return video_screen_get_vpos(0) & 0xfc;
}


READ8_HANDLER( williams2_video_counter_r )
{
	return video_screen_get_vpos(0);
}



/*************************************
 *
 *  Tilemap handling
 *
 *************************************/

static TILE_GET_INFO( get_tile_info )
{
	int mask = machine->gfx[0]->total_elements - 1;
	int data = williams2_tileram[tile_index];
	int y = (tile_index >> 1) & 7;
	int color = 0;

	switch (williams2_tilemap_config)
	{
		case WILLIAMS_TILEMAP_MYSTICM:
		{
			/* IC79 is a 74LS85 comparator that controls the low bit */
			int a = 1 | ((color & 1) << 2) | ((color & 1) << 3);
			int b = ((y & 6) >> 1);
			int casc = (y & 1);
			color = (a > b) || ((a == b) && !casc);
			break;
		}

		case WILLIAMS_TILEMAP_TSHOOT:
			/* IC79 is a 74LS157 selector jumpered to be enabled */
			color = y;
			break;

		case WILLIAMS_TILEMAP_JOUST2:
			/* IC79 is a 74LS157 selector jumpered to be disabled */
			color = 0;
			break;
	}

	SET_TILE_INFO(0, data & mask, color, (data & ~mask) ? TILE_FLIPX : 0);
}


WRITE8_HANDLER( williams2_bg_select_w )
{
	video_screen_update_partial(0, video_screen_get_vpos(0));

	/* based on the tilemap config, only certain bits are used */
	/* the rest are determined by other factors */
	switch (williams2_tilemap_config)
	{
		case WILLIAMS_TILEMAP_MYSTICM:
			/* IC79 is a 74LS85 comparator that controls the low bit */
			data &= 0x3e;
			break;

		case WILLIAMS_TILEMAP_TSHOOT:
			/* IC79 is a 74LS157 selector jumpered to be enabled */
			data &= 0x38;
			break;

		case WILLIAMS_TILEMAP_JOUST2:
			/* IC79 is a 74LS157 selector jumpered to be disabled */
			data &= 0x3f;
			break;
	}
	tilemap_set_palette_offset(bg_tilemap, data * 16);
}


WRITE8_HANDLER( williams2_tileram_w )
{
	williams2_tileram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( williams2_xscroll_low_w )
{
	video_screen_update_partial(0, video_screen_get_vpos(0));
	tilemap_xscroll = (tilemap_xscroll & ~0x00f) | ((data & 0x80) >> 4) | (data & 0x07);
	tilemap_set_scrollx(bg_tilemap, 0, (tilemap_xscroll & 7) + ((tilemap_xscroll >> 3) * 6));
}


WRITE8_HANDLER( williams2_xscroll_high_w )
{
	video_screen_update_partial(0, video_screen_get_vpos(0));
	tilemap_xscroll = (tilemap_xscroll & 0x00f) | (data << 4);
	tilemap_set_scrollx(bg_tilemap, 0, (tilemap_xscroll & 7) + ((tilemap_xscroll >> 3) * 6));
}



/*************************************
 *
 *  Blaster-specific enhancements
 *
 *************************************/

WRITE8_HANDLER( blaster_remap_select_w )
{
	video_screen_update_partial(0, video_screen_get_vpos(0));
	blitter_remap_index = data;
	blitter_remap = blitter_remap_lookup + data * 256;
}


WRITE8_HANDLER( blaster_video_control_w )
{
	video_screen_update_partial(0, video_screen_get_vpos(0));
	blaster_video_control = data;
}


WRITE8_HANDLER( blaster_scanline_control_w )
{
	video_screen_update_partial(0, video_screen_get_vpos(0));
	blaster_scanline_control[offset] = data;
}


WRITE8_HANDLER( blaster_palette_0_w )
{
	video_screen_update_partial(0, video_screen_get_vpos(0));
	blaster_palette_0[offset] = data;
	palette_set_color(Machine, 16 + offset, palette_lookup[data ^ 0xff]);
}



/*************************************
 *
 *  Blitter setup and control
 *
 *************************************/

static void blitter_init(int blitter_config, const UINT8 *remap_prom)
{
	static const UINT8 dummy_table[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
	int i,j;

	/* by default, there is no clipping window - this will be touched only by games that have one */
	williams_blitter_window_enable = 0;

	/* switch off the video config */
	blitter_xor = (blitter_config == WILLIAMS_BLITTER_SC01) ? 4 : 0;

	/* create the remap table; if no PROM, make an identity remap table */
	blitter_remap_lookup = auto_malloc(256 * 256);
	blitter_remap_index = 0;
	blitter_remap = blitter_remap_lookup;
	for (i = 0; i < 256; i++)
	{
		const UINT8 *table = remap_prom ? (remap_prom + (i & 0x7f) * 16) : dummy_table;
		for (j = 0; j < 256; j++)
			blitter_remap_lookup[i * 256 + j] = (table[j >> 4] << 4) | table[j & 0x0f];
	}
}


WRITE8_HANDLER( williams_blitter_w )
{
	int sstart, dstart, w, h, accesses;
	int estimated_clocks_at_4MHz;

	/* store the data */
	blitterram[offset] = data;

	/* only writes to location 0 trigger the blit */
	if (offset != 0)
		return;

	/* compute the starting locations */
	sstart = (blitterram[2] << 8) + blitterram[3];
	dstart = (blitterram[4] << 8) + blitterram[5];

	/* compute the width and height */
	w = blitterram[6] ^ blitter_xor;
	h = blitterram[7] ^ blitter_xor;

	/* adjust the width and height */
	if (w == 0) w = 1;
	if (h == 0) h = 1;
	if (w == 255) w = 256;
	if (h == 255) h = 256;

	/* do the actual blit */
	accesses = blitter_core(sstart, dstart, w, h, data);

	/* based on the number of memory accesses needed to do the blit, compute how long the blit will take */
	/* this is just a guess */
	estimated_clocks_at_4MHz = 20 + 2 * accesses;
	activecpu_adjust_icount(-((estimated_clocks_at_4MHz + 3) / 4));

	/* Log blits */
	logerror("%04X:Blit @ %3d : %02X%02X -> %02X%02X, %3dx%3d, mask=%02X, flags=%02X, icount=%d, win=%d\n",
			activecpu_get_pc(), video_screen_get_vpos(0),
			blitterram[2], blitterram[3],
			blitterram[4], blitterram[5],
			blitterram[6], blitterram[7],
			blitterram[1], blitterram[0],
			((estimated_clocks_at_4MHz + 3) / 4), williams_blitter_window_enable);
}


WRITE8_HANDLER( williams2_blit_window_enable_w )
{
	williams_blitter_window_enable = data & 0x01;
}



/*************************************
 *
 *  Blitter core
 *
 *************************************/

INLINE void blit_pixel(int offset, int srcdata, int data, int mask, int solid)
{
	/* always read from video RAM regardless of the bank setting */
	int pix = (offset < 0xc000) ? williams_videoram[offset] : program_read_byte(offset);

	/* handle transparency */
	if (data & 0x08)
	{
		if (!(srcdata & 0xf0)) mask |= 0xf0;
		if (!(srcdata & 0x0f)) mask |= 0x0f;
	}

	/* handle solid versus source data */
	pix &= mask;
	if (data & 0x10)
		pix |= solid & ~mask;
	else
		pix |= srcdata & ~mask;

	/* if the window is enabled, only blit to videoram below the clipping address */
	/* note that we have to allow blits to non-video RAM (e.g. tileram) because those */
	/* are not blocked by the window enable */
	if (!williams_blitter_window_enable || offset < williams_blitter_clip_address || offset >= 0xc000)
		program_write_byte(offset, pix);
}


static int blitter_core(int sstart, int dstart, int w, int h, int data)
{
	int source, sxadv, syadv;
	int dest, dxadv, dyadv;
	int i, j, solid;
	int accesses = 0;
	int keepmask;

	/* compute how much to advance in the x and y loops */
	sxadv = (data & 0x01) ? 0x100 : 1;
	syadv = (data & 0x01) ? 1 : w;
	dxadv = (data & 0x02) ? 0x100 : 1;
	dyadv = (data & 0x02) ? 1 : w;

	/* determine the common mask */
	keepmask = 0x00;
	if (data & 0x80) keepmask |= 0xf0;
	if (data & 0x40) keepmask |= 0x0f;
	if (keepmask == 0xff)
		return accesses;

	/* set the solid pixel value to the mask value */
	solid = blitterram[1];

	/* first case: no shifting */
	if (!(data & 0x20))
	{
		/* loop over the height */
		for (i = 0; i < h; i++)
		{
			source = sstart & 0xffff;
			dest = dstart & 0xffff;

			/* loop over the width */
			for (j = w; j > 0; j--)
			{
				blit_pixel(dest, blitter_remap[program_read_byte(source)], data, keepmask, solid);
				accesses += 2;

				/* advance */
				source = (source + sxadv) & 0xffff;
				dest   = (dest + dxadv) & 0xffff;
			}

			sstart += syadv;

			/* note that PlayBall! indicates the X coordinate doesn't wrap */
			if (data & 0x02)
				dstart = (dstart & 0xff00) | ((dstart + dyadv) & 0xff);
			else
				dstart += dyadv;
		}
	}

	/* second case: shifted one pixel */
	else
	{
		/* swap halves of the keep mask and the solid color */
		keepmask = ((keepmask & 0xf0) >> 4) | ((keepmask & 0x0f) << 4);
		solid = ((solid & 0xf0) >> 4) | ((solid & 0x0f) << 4);

		/* loop over the height */
		for (i = 0; i < h; i++)
		{
			int pixdata;

			source = sstart & 0xffff;
			dest = dstart & 0xffff;

			/* left edge case */
			pixdata = blitter_remap[program_read_byte(source)];
			blit_pixel(dest, (pixdata >> 4) & 0x0f, data, keepmask | 0xf0, solid);
			accesses += 2;

			source = (source + sxadv) & 0xffff;
			dest   = (dest + dxadv) & 0xffff;

			/* loop over the width */
			for (j = w - 1; j > 0; j--)
			{
				pixdata = (pixdata << 8) | blitter_remap[program_read_byte(source)];
				blit_pixel(dest, (pixdata >> 4) & 0xff, data, keepmask, solid);
				accesses += 2;

				source = (source + sxadv) & 0xffff;
				dest   = (dest + dxadv) & 0xffff;
			}

			/* right edge case */
			blit_pixel(dest, (pixdata << 4) & 0xf0, data, keepmask | 0x0f, solid);
			accesses++;

			sstart += syadv;

			/* note that PlayBall! indicates the X coordinate doesn't wrap */
			if (data & 0x02)
				dstart = (dstart & 0xff00) | ((dstart + dyadv) & 0xff);
			else
				dstart += dyadv;
		}
	}

	return accesses;
}
