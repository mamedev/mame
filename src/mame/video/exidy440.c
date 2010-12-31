/***************************************************************************

    Exidy 440 video system

***************************************************************************/

#include "emu.h"
#include "includes/exidy440.h"


#define PIXEL_CLOCK			(EXIDY440_MASTER_CLOCK / 2)
#define HTOTAL				(0x1a0)
#define HBEND				(0x000)
#define HBSTART				(0x140)
#define HSEND				(0x178)
#define HSSTART				(0x160)
#define VTOTAL				(0x104)
#define VBEND				(0x000)
#define VBSTART				(0x0f0)
#define VSEND				(0x0f8)
#define VSSTART				(0x0f0)

/* Top Secret has a larger VBLANK area */
#define TOPSECEX_VBSTART	(0x0ec)

#define SPRITE_COUNT		(0x28)


/* globals */
UINT8 *exidy440_scanline;
UINT8 *exidy440_imageram;
UINT8  exidy440_firq_vblank;
UINT8  exidy440_firq_beam;
UINT8 *topsecex_yscroll;

/* local allocated storage */
static UINT8 exidy440_latched_x;
static UINT8 *local_videoram;
static UINT8 *local_paletteram;

/* local variables */
static UINT8 firq_enable;
static UINT8 firq_select;
static UINT8 palettebank_io;
static UINT8 palettebank_vis;

/* function prototypes */
static void exidy440_update_firq(running_machine *machine);



/*************************************
 *
 *  Initialize the video system
 *
 *************************************/

static VIDEO_START( exidy440 )
{
	/* reset the system */
	firq_enable = 0;
	firq_select = 0;
	palettebank_io = 0;
	palettebank_vis = 0;
	exidy440_firq_vblank = 0;
	exidy440_firq_beam = 0;

	/* allocate a buffer for VRAM */
	local_videoram = auto_alloc_array(machine, UINT8, 256 * 256 * 2);
	memset(local_videoram, 0, 256 * 256 * 2);

	/* allocate a buffer for palette RAM */
	local_paletteram = auto_alloc_array(machine, UINT8, 512 * 2);
	memset(local_paletteram, 0, 512 * 2);
}


static VIDEO_START( topsecex )
{
	VIDEO_START_CALL(exidy440);

	*topsecex_yscroll = 0;
}



/*************************************
 *
 *  Video RAM read/write
 *
 *************************************/

READ8_HANDLER( exidy440_videoram_r )
{
	UINT8 *base = &local_videoram[(*exidy440_scanline * 256 + offset) * 2];

	/* combine the two pixel values into one byte */
	return (base[0] << 4) | base[1];
}


WRITE8_HANDLER( exidy440_videoram_w )
{
	UINT8 *base = &local_videoram[(*exidy440_scanline * 256 + offset) * 2];

	/* expand the two pixel values into two bytes */
	base[0] = (data >> 4) & 15;
	base[1] = data & 15;
}



/*************************************
 *
 *  Palette RAM read/write
 *
 *************************************/

READ8_HANDLER( exidy440_paletteram_r )
{
	return local_paletteram[palettebank_io * 512 + offset];
}


WRITE8_HANDLER( exidy440_paletteram_w )
{
	/* update palette ram in the I/O bank */
	local_paletteram[palettebank_io * 512 + offset] = data;

	/* if we're modifying the active palette, change the color immediately */
	if (palettebank_io == palettebank_vis)
	{
		int word;

		/* combine two bytes into a word */
		offset = palettebank_vis * 512 + (offset & 0x1fe);
		word = (local_paletteram[offset] << 8) + local_paletteram[offset + 1];

		/* extract the 5-5-5 RGB colors */
		palette_set_color_rgb(space->machine, offset / 2, pal5bit(word >> 10), pal5bit(word >> 5), pal5bit(word >> 0));
	}
}



/*************************************
 *
 *  Horizontal/vertical positions
 *
 *************************************/

READ8_HANDLER( exidy440_horizontal_pos_r )
{
	/* clear the FIRQ on a read here */
	exidy440_firq_beam = 0;
	exidy440_update_firq(space->machine);

	/* according to the schems, this value is only latched on an FIRQ
     * caused by collision or beam */
	return exidy440_latched_x;
}


READ8_HANDLER( exidy440_vertical_pos_r )
{
	int result;

	/* according to the schems, this value is latched on any FIRQ
     * caused by collision or beam, ORed together with CHRCLK,
     * which probably goes off once per scanline; for now, we just
     * always return the current scanline */
	result = space->machine->primary_screen->vpos();
	return (result < 255) ? result : 255;
}



/*************************************
 *
 *  Sprite RAM handler
 *
 *************************************/

WRITE8_HANDLER( exidy440_spriteram_w )
{
	space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
	space->machine->generic.spriteram.u8[offset] = data;
}



/*************************************
 *
 *  Interrupt and I/O control regs
 *
 *************************************/

WRITE8_HANDLER( exidy440_control_w )
{
	int oldvis = palettebank_vis;

	/* extract the various bits */
	exidy440_bank_select(space->machine, data >> 4);
	firq_enable = (data >> 3) & 1;
	firq_select = (data >> 2) & 1;
	palettebank_io = (data >> 1) & 1;
	palettebank_vis = data & 1;

	/* update the FIRQ in case we enabled something */
	exidy440_update_firq(space->machine);

	/* if we're swapping palettes, change all the colors */
	if (oldvis != palettebank_vis)
	{
		int i;

		/* pick colors from the visible bank */
		offset = palettebank_vis * 512;
		for (i = 0; i < 256; i++, offset += 2)
		{
			/* extract a word and the 5-5-5 RGB components */
			int word = (local_paletteram[offset] << 8) + local_paletteram[offset + 1];
			palette_set_color_rgb(space->machine, i, pal5bit(word >> 10), pal5bit(word >> 5), pal5bit(word >> 0));
		}
	}
}


WRITE8_HANDLER( exidy440_interrupt_clear_w )
{
	/* clear the VBLANK FIRQ on a write here */
	exidy440_firq_vblank = 0;
	exidy440_update_firq(space->machine);
}



/*************************************
 *
 *  Interrupt generators
 *
 *************************************/

static void exidy440_update_firq(running_machine *machine)
{
	if (exidy440_firq_vblank || (firq_enable && exidy440_firq_beam))
		cputag_set_input_line(machine, "maincpu", 1, ASSERT_LINE);
	else
		cputag_set_input_line(machine, "maincpu", 1, CLEAR_LINE);
}


INTERRUPT_GEN( exidy440_vblank_interrupt )
{
	/* set the FIRQ line on a VBLANK */
	exidy440_firq_vblank = 1;
	exidy440_update_firq(device->machine);
}



/*************************************
 *
 *  IRQ callback handlers
 *
 *************************************/

static TIMER_CALLBACK( beam_firq_callback )
{
	/* generate the interrupt, if we're selected */
	if (firq_select && firq_enable)
	{
		exidy440_firq_beam = 1;
		exidy440_update_firq(machine);
	}

	/* round the x value to the nearest byte */
	param = (param + 1) / 2;

	/* latch the x value; this convolution comes from the read routine */
	exidy440_latched_x = (param + 3) ^ 2;
}


static TIMER_CALLBACK( collide_firq_callback )
{
	/* generate the interrupt, if we're selected */
	if (!firq_select && firq_enable)
	{
		exidy440_firq_beam = 1;
		exidy440_update_firq(machine);
	}

	/* round the x value to the nearest byte */
	param = (param + 1) / 2;

	/* latch the x value; this convolution comes from the read routine */
	exidy440_latched_x = (param + 3) ^ 2;
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

static void draw_sprites(screen_device &screen, bitmap_t *bitmap, const rectangle *cliprect,
						 int scroll_offset, int check_collision)
{
	int i;

	/* get a pointer to the palette to look for collision flags */
	UINT8 *palette = &local_paletteram[palettebank_vis * 512];
	int count = 0;

	/* draw the sprite images, checking for collisions along the way */
	UINT8 *sprite = screen.machine->generic.spriteram.u8 + (SPRITE_COUNT - 1) * 4;

	for (i = 0; i < SPRITE_COUNT; i++, sprite -= 4)
	{
		int image = (~sprite[3] & 0x3f);
		int xoffs = (~((sprite[1] << 8) | sprite[2]) & 0x1ff);
		int yoffs = (~sprite[0] & 0xff) + 1;
		int x, y, sy;
		UINT8 *src;

		/* skip if out of range */
		if (yoffs < cliprect->min_y || yoffs >= cliprect->max_y + 16)
			continue;

		/* get a pointer to the source image */
		src = &exidy440_imageram[image * 128];

		/* account for large positive offsets meaning small negative values */
		if (xoffs >= 0x1ff - 16)
			xoffs -= 0x1ff;

		/* loop over y */
		sy = yoffs + scroll_offset;
		for (y = 0; y < 16; y++, yoffs--, sy--)
		{
			/* wrap at the top and bottom of the screen */
			if (sy >= VBSTART)
				sy -= (VBSTART - VBEND);
			else if (sy < VBEND)
				sy += (VBSTART - VBEND);

			/* stop if we get before the current scanline */
			if (yoffs < cliprect->min_y)
				break;

			/* only draw scanlines that are in this cliprect */
			if (yoffs <= cliprect->max_y)
			{
				UINT8 *old = &local_videoram[sy * 512 + xoffs];
				int currx = xoffs;

				/* loop over x */
				for (x = 0; x < 8; x++, old += 2)
				{
					int ipixel = *src++;
					int left = ipixel & 0xf0;
					int right = (ipixel << 4) & 0xf0;
					int pen;

					/* left pixel */
					if (left && currx >= HBEND && currx < HBSTART)
					{
						/* combine with the background */
						pen = left | old[0];
						*BITMAP_ADDR16(bitmap, yoffs, currx) = pen;

						/* check the collisions bit */
						if (check_collision && (palette[2 * pen] & 0x80) && (count++ < 128))
							timer_set(screen.machine, screen.time_until_pos(yoffs, currx), NULL, currx, collide_firq_callback);
					}
					currx++;

					/* right pixel */
					if (right && currx >= HBEND && currx < HBSTART)
					{
						/* combine with the background */
						pen = right | old[1];
						*BITMAP_ADDR16(bitmap, yoffs, currx) = pen;

						/* check the collisions bit */
						if (check_collision && (palette[2 * pen] & 0x80) && (count++ < 128))
							timer_set(screen.machine, screen.time_until_pos(yoffs, currx), NULL, currx, collide_firq_callback);
					}
					currx++;
				}
			}
			else
				src += 8;
		}
	}
}



/*************************************
 *
 *  Core refresh routine
 *
 *************************************/

static void update_screen(screen_device &screen, bitmap_t *bitmap, const rectangle *cliprect,
						  int scroll_offset, int check_collision)
{
	int y, sy;

	/* draw any dirty scanlines from the VRAM directly */
	sy = scroll_offset + cliprect->min_y;
	for (y = cliprect->min_y; y <= cliprect->max_y; y++, sy++)
	{
		/* wrap at the bottom of the screen */
		if (sy >= VBSTART)
			sy -= (VBSTART - VBEND);

		/* draw line */
		draw_scanline8(bitmap, 0, y, (HBSTART - HBEND), &local_videoram[sy * 512], NULL);
	}

	/* draw the sprites */
	draw_sprites(screen, bitmap, cliprect, scroll_offset, check_collision);
}



/*************************************
 *
 *  Standard screen refresh callback
 *
 *************************************/

static VIDEO_UPDATE( exidy440 )
{
	/* redraw the screen */
	update_screen(*screen, bitmap, cliprect, 0, TRUE);

	/* generate an interrupt once/frame for the beam */
	if (cliprect->max_y == screen->visible_area().max_y)
	{
		int i;

		int beamx = ((input_port_read(screen->machine, "AN0") & 0xff) * (HBSTART - HBEND)) >> 8;
		int beamy = ((input_port_read(screen->machine, "AN1") & 0xff) * (VBSTART - VBEND)) >> 8;

		/* The timing of this FIRQ is very important. The games look for an FIRQ
            and then wait about 650 cycles, clear the old FIRQ, and wait a
            very short period of time (~130 cycles) for another one to come in.
            From this, it appears that they are expecting to get beams over
            a 12 scanline period, and trying to pick roughly the middle one.
            This is how it is implemented. */
		attoseconds_t increment = attotime_to_attoseconds(screen->scan_period());
		attotime time = attotime_sub(screen->time_until_pos(beamy, beamx), attotime_make(0, increment * 6));
		for (i = 0; i <= 12; i++)
		{
			timer_set(screen->machine, time, NULL, beamx, beam_firq_callback);
			time = attotime_add(time, attotime_make(0, increment));
		}
	}

	return 0;
}


static VIDEO_UPDATE( topsecex )
{
	/* redraw the screen */
	update_screen(*screen, bitmap, cliprect, *topsecex_yscroll, FALSE);

	return 0;
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( exidy440_video )
	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_VIDEO_START(exidy440)
	MCFG_VIDEO_UPDATE(exidy440)
	MCFG_PALETTE_LENGTH(256)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( topsecex_video )
	MCFG_VIDEO_ATTRIBUTES(0)
	MCFG_VIDEO_START(topsecex)
	MCFG_VIDEO_UPDATE(topsecex)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, TOPSECEX_VBSTART)
MACHINE_CONFIG_END
