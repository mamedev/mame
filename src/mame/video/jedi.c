/***************************************************************************

    Atari Return of the Jedi hardware

***************************************************************************/

#include "driver.h"
#include "jedi.h"


/* globals */
UINT8 *jedi_backgroundram;
size_t jedi_backgroundram_size;
UINT8 *jedi_PIXIRAM;


/* local variables */
static UINT32 jedi_vscroll;
static UINT32 jedi_hscroll;
static UINT32 jedi_alpha_bank;
static UINT8 video_off, smooth_table;
static UINT8 *fgdirty, *bgdirty;
static mame_bitmap *fgbitmap, *mobitmap, *bgbitmap, *bgexbitmap;



/*************************************
 *
 *  Video startup
 *
 *************************************/

static void jedi_postload(void)
{
	memset(fgdirty, 1, videoram_size);
	memset(bgdirty, 1, jedi_backgroundram_size);
}


VIDEO_START( jedi )
{
	/* allocate dirty buffer for the foreground characters */
	fgdirty = dirtybuffer = auto_malloc(videoram_size);
	memset(fgdirty, 1, videoram_size);

	/* allocate an 8bpp bitmap for the raw foreground characters */
	fgbitmap = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);

	/* allocate an 8bpp bitmap for the motion objects */
	mobitmap = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
	fillbitmap(mobitmap, 0, &machine->screen[0].visarea);

	/* allocate dirty buffer for the background characters */
	bgdirty = auto_malloc(jedi_backgroundram_size);
	memset(bgdirty, 1, jedi_backgroundram_size);

	/* the background area is 256x256, doubled by the hardware*/
	bgbitmap = auto_bitmap_alloc(256, 256, machine->screen[0].format);

	/* the expanded background area is 512x512 */
	bgexbitmap = auto_bitmap_alloc(512, 512, machine->screen[0].format);

	/* reserve color 1024 for black (disabled display) */
	palette_set_color(machine, 1024, MAKE_RGB(0, 0, 0));

	/* register for saving */
	state_save_register_global(jedi_vscroll);
	state_save_register_global(jedi_hscroll);
	state_save_register_global(jedi_alpha_bank);
	state_save_register_global(video_off);
	state_save_register_global(smooth_table);
	state_save_register_func_postload(jedi_postload);
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

WRITE8_HANDLER( jedi_paletteram_w )
{
    int r, g, b, bits, intensity;
    UINT32 color;

	paletteram[offset] = data;
	color = paletteram[offset & 0x3FF] | (paletteram[offset | 0x400] << 8);

	intensity = (color >> 9) & 7;
	bits = (color >> 6) & 7;
	r = 5 * bits * intensity;
	bits = (color >> 3) & 7;
	g = 5 * bits * intensity;
	bits = (color >> 0) & 7;
	b = 5 * bits * intensity;

	palette_set_color(Machine, offset & 0x3ff, MAKE_RGB(r, g, b));
}



/*************************************
 *
 *  Background access
 *
 *************************************/

WRITE8_HANDLER( jedi_backgroundram_w )
{
	if (jedi_backgroundram[offset] != data)
	{
		bgdirty[offset] = 1;
		jedi_backgroundram[offset] = data;
	}
}



/*************************************
 *
 *  Foreground banking
 *
 *************************************/

WRITE8_HANDLER( jedi_alpha_banksel_w )
{
	if (jedi_alpha_bank != 2 * (data & 0x80))
	{
		jedi_alpha_bank = 2 * (data & 0x80);
		memset(fgdirty, 1, videoram_size);
	}
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
	smooth_table = data & 0x03;
	memset(bgdirty, 1, jedi_backgroundram_size);
}



/*************************************
 *
 *  Background smoothing
 *
 *************************************/

static void update_smoothing(int bgtilerow, int first, int last)
{
	UINT8 *prom = memory_region(REGION_PROMS) + smooth_table * 0x100;
	UINT8 bgscan[2][256];
	UINT8 *bgcurr = bgscan[0], *bglast = bgscan[1];
	int xstart, xstop, x, y;

	/*
        smoothing notes:
            * even scanlines blend the previous (Y-1) and current (Y) line
            * odd scanlines are just taken from the current line (Y)
            * therefore, if we modify source scanlines 8-15, we must update dest scanlines 16-32

            * even pixels are just taken from the current pixel (X)
            * odd pixels blend the current (X) and next (X+1) pixels
            * therefore, if we modify source pixels 8-15, we must update dest pixels 15-31
    */

	/* compute x start/stop in destination coordinates */
	xstart = first * 16 - 1;
	xstop = last * 16 + 15;

	/* extract the previous bg scanline */
	extract_scanline8(bgbitmap, 0, ((bgtilerow * 16 - 1) & 0x1ff) / 2, 256, bgcurr);

	/* loop over height */
	for (y = 0; y <= 16; y++)
	{
		int curry = (bgtilerow * 16 + y) & 0x1ff;

		/* swap background buffers */
		UINT8 *bgtemp = bgcurr;
		bgcurr = bglast;
		bglast = bgtemp;

		/* extract current bg scanline */
		extract_scanline8(bgbitmap, 0, curry / 2, 256, bgcurr);

		/* loop over columns */
		for (x = xstart; x <= xstop; x++)
		{
			int tr = bglast[((x + 1) & 0x1ff) / 2];
			int br = bgcurr[((x + 1) & 0x1ff) / 2];

			/* smooth pixels */
			if (x & 1)
			{
				int tl = bglast[(x & 0x1ff) / 2];
				int bl = bgcurr[(x & 0x1ff) / 2];
				int mixt = prom[16 * tl + tr];
				int mixb = prom[16 * bl + br];
				*BITMAP_ADDR16(bgexbitmap, curry, x & 0x1ff) = prom[0x400 + 16 * mixt + mixb];
			}
			else
				*BITMAP_ADDR16(bgexbitmap, curry, x & 0x1ff) = prom[0x400 + 16 * tr + br];
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
	int bgexdirty[32][2];
	int offs;


	/* if no video, clear it all to black */
	if (video_off)
	{
		fillbitmap(bitmap, machine->pens[1024], &machine->screen[0].visarea);
		return 0;
	}

	/* Return of the Jedi has a peculiar playfield/motion object priority system. That */
	/* is, there is no priority system ;-) The color of the pixel which appears on */
	/* screen depends on all three of the foreground, background and motion objects. The */
	/* 1024 colors palette is appropriately set up by the program to "emulate" a */
	/* priority system, but it can also be used to display completely different */
	/* colors (see the palette test in service mode) */

    /* update foreground bitmap as a raw bitmap*/
    for (offs = videoram_size - 1; offs >= 0; offs--)
		if (fgdirty[offs])
		{
			int sx = offs % 64;
			int sy = offs / 64;

			fgdirty[offs] = 0;

			drawgfx(fgbitmap, machine->gfx[0], videoram[offs] + jedi_alpha_bank,
					0, 0, 0, 8*sx, 8*sy, &machine->screen[0].visarea, TRANSPARENCY_NONE_RAW, 0);
		}

	/* reset the expanded dirty array */
	for (offs = 0; offs < 32; offs++)
		bgexdirty[offs][0] = bgexdirty[offs][1] = -1;

    /* update background bitmap as a raw bitmap */
	for (offs = jedi_backgroundram_size / 2 - 1; offs >= 0; offs--)
		if (bgdirty[offs] || bgdirty[offs + 0x400])
		{
			int sx = offs % 32;
			int sy = offs / 32;
			int code = (jedi_backgroundram[offs] & 0xFF);
			int bank = (jedi_backgroundram[offs + 0x400] & 0x0F);

			/* shuffle the bank bits in */
			code |= (bank & 0x01) << 8;
			code |= (bank & 0x08) << 6;
			code |= (bank & 0x02) << 9;

			bgdirty[offs] = bgdirty[offs + 0x400] = 0;

			/* update expanded dirty status (assumes we go right-to-left) */
			if (bgexdirty[sy][1] == -1)
				bgexdirty[sy][1] = sx;
			bgexdirty[sy][0] = sx;

			drawgfx(bgbitmap, machine->gfx[1], code,
					0, bank & 0x04, 0, 8*sx, 8*sy, 0, TRANSPARENCY_NONE_RAW, 0);
		}

	/* update smoothed version of background */
	for (offs = 0; offs < 32; offs++)
		if (bgexdirty[offs][1] != -1)
			update_smoothing(offs, bgexdirty[offs][0], bgexdirty[offs][1]);

	/* draw the motion objects */
    for (offs = 0; offs < 0x30; offs++)
	{
		/* coordinates adjustments made to match screenshot */
		int x = spriteram[offs + 0x100] + ((spriteram[offs + 0x40] & 0x01) << 8) - 2;
		int y = 240 - spriteram[offs + 0x80] + 1;
		int flipx = spriteram[offs + 0x40] & 0x10;
		int flipy = spriteram[offs + 0x40] & 0x20;
		int tall = spriteram[offs + 0x40] & 0x08;
		int code, bank;

		/* shuffle the bank bits in */
		bank  = ((spriteram[offs + 0x40] & 0x02) >> 1);
		bank |= ((spriteram[offs + 0x40] & 0x40) >> 5);
		bank |=  (spriteram[offs + 0x40] & 0x04);
		code = spriteram[offs] + (bank * 256);

		/* adjust for double-height */
		if (tall)
			code |= 1;

		/* draw motion object */
		drawgfx(mobitmap, machine->gfx[2], code,
				0, flipx, flipy, x, y, &machine->screen[0].visarea, TRANSPARENCY_PEN_RAW, 0);

		/* handle double-height */
		if (tall)
			drawgfx(mobitmap, machine->gfx[2], code - 1,
					0, flipx, flipy, x, y - 16, &machine->screen[0].visarea, TRANSPARENCY_PEN_RAW, 0);
    }

	/* compose the three layers */
	{
		int xscroll = -jedi_hscroll;
		int yscroll = -jedi_vscroll;
		copyscrollbitmap(bitmap, bgexbitmap, 1, &xscroll, 1, &yscroll, &machine->screen[0].visarea, TRANSPARENCY_NONE, 0);
		copybitmap(bitmap, mobitmap, 0, 0, 0, 0, &machine->screen[0].visarea, TRANSPARENCY_BLEND_RAW, 4);
		copybitmap(bitmap, fgbitmap, 0, 0, 0, 0, &machine->screen[0].visarea, TRANSPARENCY_BLEND, 8);
	}

	/* erase the motion objects */
    for (offs = 0; offs < 0x30; offs++)
	{
		/* coordinates adjustments made to match screenshot */
		int x = spriteram[offs + 0x100] + ((spriteram[offs + 0x40] & 0x01) << 8) - 2;
		int y = 240 - spriteram[offs + 0x80] + 1;
		int tall = spriteram[offs + 0x40] & 0x08;
		rectangle bounds;

		/* compute the bounds */
		bounds.min_x = x;
		bounds.max_x = x + 15;
		bounds.min_y = tall ? (y - 16) : y;
		bounds.max_y = y + (tall ? 31 : 15);
		fillbitmap(mobitmap, 0, &bounds);
    }
	return 0;
}
