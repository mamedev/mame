/***************************************************************************

    Atari Return of the Jedi hardware

    Return of the Jedi has a peculiar playfield/motion object
    priority system. That is, there is no priority system ;-)
    The color of the pixel which appears on screen depends on
    all three of the foreground, background and motion objects.
    The 1024 colors palette is appropriately set up by the program
    to "emulate" a priority system, but it can also be used to display
    completely different colors (see the palette test in service mode)

***************************************************************************/

#include "driver.h"
#include "jedi.h"


#define NUM_PENS	(0x1000)


/* globals */
UINT8 *jedi_foregroundram;
UINT8 *jedi_backgroundram;
UINT8 *jedi_spriteram;
UINT8 *jedi_paletteram;
UINT8 *jedi_foreground_bank;

/* local variables */
static UINT32 jedi_vscroll;
static UINT32 jedi_hscroll;
static UINT8 video_off;
static UINT8 smoothing_table;



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( jedi )
{
	/* register for saving */
	state_save_register_global(jedi_vscroll);
	state_save_register_global(jedi_hscroll);
	state_save_register_global(video_off);
	state_save_register_global(smoothing_table);
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************
 *
 *  Color RAM format
 *  Color RAM is 1024x12
 *
 *  RAM address: A0..A3 = Playfield color code
 *      A4..A7 = Motion object color code
 *      A8..A9 = Alphanumeric color code
 *
 *  RAM data:
 *      0..2 = Blue
 *      3..5 = Green
 *      6..8 = Blue
 *      9..11 = Intensity
 *
 *  Output resistor values:
 *      bit 0 = 22K
 *      bit 1 = 10K
 *      bit 2 = 4.7K
 *
 *************************************/

static void get_pens(pen_t *pens)
{
	offs_t offs;

	for (offs = 0; offs < NUM_PENS; offs++)
	{
		int r, g, b, bits, intensity;

		UINT16 color = jedi_paletteram[offs] | (jedi_paletteram[offs | 0x400] << 8);

		intensity = (color >> 9) & 7;
		bits = (color >> 6) & 7;
		r = 5 * bits * intensity;
		bits = (color >> 3) & 7;
		g = 5 * bits * intensity;
		bits = (color >> 0) & 7;
		b = 5 * bits * intensity;

		pens[offs] = MAKE_RGB(r, g, b);
	}
}


static void do_pen_lookup(mame_bitmap *bitmap, const rectangle *cliprect)
{
	int y, x;
	pen_t pens[NUM_PENS];

	get_pens(pens);

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		for(x = cliprect->min_x; x <= cliprect->max_x; x++)
			*BITMAP_ADDR32(bitmap, y, x) = pens[*BITMAP_ADDR32(bitmap, y, x)];
}



/*************************************
 *
 *  Scroll offsets
 *
 *************************************/

WRITE8_HANDLER( jedi_vscroll_w )
{
    jedi_vscroll = data | (offset << 8);
}


WRITE8_HANDLER( jedi_hscroll_w )
{
    jedi_hscroll = data | (offset << 8);
}



/*************************************
 *
 *  Video control
 *
 *************************************/

WRITE8_HANDLER( jedi_video_off_w )
{
	video_off = data;
}


WRITE8_HANDLER( jedi_PIXIRAM_w )
{
	/* this should really be 0x07, but the PROMs were
       dumped at half size */
	smoothing_table = data & 0x03;
}



/*************************************
 *
 *  Foreground drawing
 *
 *************************************/

static void draw_foreground(mame_bitmap *bitmap)
{
	offs_t offs;
    UINT32 *dst = BITMAP_ADDR32(bitmap, 0, 0);

	/* draw the bitmap 4 pixels at a time */
    for (offs = 0; offs < 0x7c00; offs++)
	{
		UINT16 code = ((*jedi_foreground_bank & 0x80) << 1) |
					  jedi_foregroundram[((offs & 0x7c00) >> 4) | ((offs & 0x7e) >> 1)];

		UINT8 data = memory_region(REGION_GFX1)[(code << 4) | ((offs & 0x380) >> 6) | (offs & 0x01)];

		/* the background pixel determines pen address bits A8 and A9 */
		dst[0] = (dst[0] & 0xff) | ((data & 0xc0) << 2);
		dst[1] = (dst[1] & 0xff) | ((data & 0x30) << 4);
		dst[2] = (dst[2] & 0xff) | ((data & 0x0c) << 6);
		dst[3] = (dst[3] & 0xff) | ((data & 0x03) << 8);

		dst = dst + 4;
	}
}



/*************************************
 *
 *  Background drawing with smoothing
 *
 *************************************/

static void draw_and_smooth_background(mame_bitmap *bitmap, const rectangle *cliprect)
{
	int y;
	UINT32 background_line_buffer[0x200];	/* RAM chip at 2A */

	UINT8 *prom1 = &memory_region(REGION_PROMS)[0x0000 | (smoothing_table << 8)];
	UINT8 *prom2 = &memory_region(REGION_PROMS)[0x0800 | (smoothing_table << 8)];

	memset(background_line_buffer, 0, 0x200 * sizeof(UINT32));

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int x;
		UINT32 last_col = 0;

		for (x = cliprect->min_x; x <= cliprect->max_x; x += 2)
		{
			UINT32 col;
			UINT32 tempcol;
			offs_t gfx_offs;
			UINT8 data1;
			UINT8 data2;

			int sy = y + jedi_vscroll;
			int sx = x + jedi_hscroll;

			offs_t backgroundram_offs = ((sy & 0x1f0) << 1) | ((sx & 0x1f0) >> 4);

			/* shuffle the bank bits in */
			UINT8 bank = jedi_backgroundram[0x0400 | backgroundram_offs];
			UINT16 code = jedi_backgroundram[0x0000 | backgroundram_offs] |
						  ((bank & 0x01) << 8) |
						  ((bank & 0x08) << 6) |
						  ((bank & 0x02) << 9);

			/* flip X */
			if (bank & 0x04)
				sx = sx ^ 0x0f;

			/* get the pointer to the graphics */
			gfx_offs = (code << 4) | (sy & 0x0e) | (((sx & 0x08) >> 3));

			data1 = memory_region(REGION_GFX2)[0x0000 | gfx_offs];
			data2 = memory_region(REGION_GFX2)[0x8000 | gfx_offs];

			/* the foreground pixel determines pen address bits A0-A3 */
			switch (sx & 0x06)
			{
			case 0x00:	col = ((data1 & 0x80) >> 4) | ((data1 & 0x08) >> 1) | ((data2 & 0x80) >> 6) | ((data2 & 0x08) >> 3); break;
			case 0x02:	col = ((data1 & 0x40) >> 3) | ((data1 & 0x04) >> 0) | ((data2 & 0x40) >> 5) | ((data2 & 0x04) >> 2); break;
			case 0x04:	col = ((data1 & 0x20) >> 2) | ((data1 & 0x02) << 1) | ((data2 & 0x20) >> 4) | ((data2 & 0x02) >> 1); break;
			default:	col = ((data1 & 0x10) >> 1) | ((data1 & 0x01) << 2) | ((data2 & 0x10) >> 3) | ((data2 & 0x01) >> 0); break;
			}

			/* the first pixel is smoothed via a lookup using the current and last pixel value -
               the next pixel just uses the current value directly. After we done with a pixel
               save it for later in the line buffer RAM */
			tempcol = prom1[(last_col << 4) | col];
			*BITMAP_ADDR32(bitmap, y, x + 0) = prom2[(background_line_buffer[x + 0] << 4) | tempcol];
			*BITMAP_ADDR32(bitmap, y, x + 1) = prom2[(background_line_buffer[x + 1] << 4) | col];
			background_line_buffer[x + 0] = tempcol;
			background_line_buffer[x + 1] = col;

			last_col = col;
		}
	}
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

static void draw_sprites(mame_bitmap *bitmap)
{
	offs_t offs;

	for (offs = 0x00; offs < 0x30; offs++)
	{
		int sy;
		int y_size;
		UINT8 *gfx;

		/* coordinates adjustments made to match screenshot */
		UINT8 y = 240 - jedi_spriteram[offs + 0x80] + 1;
		int flip_x = jedi_spriteram[offs + 0x40] & 0x10;
		int flip_y = jedi_spriteram[offs + 0x40] & 0x20;
		int tall = jedi_spriteram[offs + 0x40] & 0x08;

		/* shuffle the bank bits in */
		UINT16 code = jedi_spriteram[offs] |
					  ((jedi_spriteram[offs + 0x40] & 0x04) << 8) |
					  ((jedi_spriteram[offs + 0x40] & 0x40) << 3) |
					  ((jedi_spriteram[offs + 0x40] & 0x02) << 7);

		/* adjust for double-height */
		if (tall)
		{
			code &= ~1;
			y_size = 0x20;
			y = y - 0x10;
		}
		else
			y_size = 0x10;

		gfx = &memory_region(REGION_GFX3)[code << 5];

		if (flip_y)
			y = y + y_size - 1;

		for (sy = 0; sy < y_size; sy++)
		{
			int i;
			UINT16 x = jedi_spriteram[offs + 0x100] + ((jedi_spriteram[offs + 0x40] & 0x01) << 8) - 2;

			if (flip_x)
				x = x + 7;

			for (i = 0; i < 2; i++)
			{
				int sx;
				UINT8 data1 = *(0x00000 + gfx);
				UINT8 data2 = *(0x10000 + gfx);

				for (sx = 0; sx < 4; sx++)
				{
					/* the sprite pixel determines pen address bits A4-A7 */
					UINT32 col = ((data1 & 0x80) >> 0) | ((data1 & 0x08) << 3) | ((data2 & 0x80) >> 2) | ((data2 & 0x08) << 1);

					x = x & 0x1ff;

					if (col)
						*BITMAP_ADDR32(bitmap, y, x) = (*BITMAP_ADDR32(bitmap, y, x) & 0x30f) | col;

					/* next pixel */
					if (flip_x)
						x = x - 1;
					else
						x = x + 1;

					data1 = data1 << 1;
					data2 = data2 << 1;
				}

				gfx = gfx + 1;
			}

			if (flip_y)
				y = y - 1;
			else
				y = y + 1;
		}
	}
}



/*************************************
 *
 *  Core video refresh
 *
 *************************************/

VIDEO_UPDATE( jedi )
{
	/* if no video, clear it all to black */
	if (video_off)
		fillbitmap(bitmap, RGB_BLACK, cliprect);
	else
	{
		/* draw the background - it needs to be done first */
		draw_and_smooth_background(bitmap, cliprect);

		draw_foreground(bitmap);
		draw_sprites(bitmap);

		do_pen_lookup(bitmap, cliprect);
	}

	return 0;
}
