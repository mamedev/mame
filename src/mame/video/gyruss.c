/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "video/resnet.h"


static UINT8 flipscreen;

/*
sprites are multiplexed, so we have to buffer the spriteram
scanline by scanline.
*/
static UINT8 *sprite_mux_buffer;
static UINT32 scanline;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Gyruss has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( gyruss )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances_rg[0], rweights, 470, 0,
			3, &resistances_rg[0], gweights, 470, 0,
			2, &resistances_b[0],  bweights, 470, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 32);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 32;

	/* sprites map to the lower 16 palette entries */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* characters map to the upper 16 palette entries */
	for (i = 0x100; i < 0x140; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry + 0x10);
	}
}



VIDEO_START( gyruss )
{
	sprite_mux_buffer = auto_malloc(256 * spriteram_size);

	state_save_register_global(flipscreen);
	state_save_register_global(scanline);
	state_save_register_global_pointer(sprite_mux_buffer,256*spriteram_size);
}



WRITE8_HANDLER( gyruss_flipscreen_w )
{
	flipscreen = data & 1;
}



/* Return the current video scan line */
READ8_HANDLER( gyruss_scanline_r )
{
	return scanline;
}



static void draw_sprites(running_machine *machine, mame_bitmap *bitmap)
{
	rectangle clip = machine->screen[0].visarea;
	int offs;
	int line;


	for (line = 0;line < 256;line++)
	{
		UINT8 *sr;

		sr = sprite_mux_buffer + line * spriteram_size;
		clip.min_y = clip.max_y = line;

		for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
		{
			int sx,sy;

			sx = sr[offs];
			sy = 241 - sr[offs + 3];
			if (sy > line-16 && sy <= line)
			{
				drawgfx(bitmap,machine->gfx[sr[offs + 1] & 1],
						sr[offs + 1]/2 + 4*(sr[offs + 2] & 0x20),
						sr[offs + 2] & 0x0f,
						!(sr[offs + 2] & 0x40),sr[offs + 2] & 0x80,
						sx,sy,
						&clip,TRANSPARENCY_PEN,0);
			}
		}
	}
}


VIDEO_UPDATE( gyruss )
{
	int offs;


	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy,flipx,flipy;

		sx = offs % 32;
		sy = offs / 32;
		flipx = colorram[offs] & 0x40;
		flipy = colorram[offs] & 0x80;
		if (flipscreen)
		{
			sx = 31 - sx;
			sy = 31 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[2],
				videoram[offs] + 8 * (colorram[offs] & 0x20),
				colorram[offs] & 0x0f,
				flipx,flipy,
				8*sx,8*sy,
				cliprect,TRANSPARENCY_NONE,0);
	}


	draw_sprites(machine, bitmap);


	/* redraw the characters which have priority over sprites */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy,flipx,flipy;


		sx = offs % 32;
		sy = offs / 32;
		flipx = colorram[offs] & 0x40;
		flipy = colorram[offs] & 0x80;
		if (flipscreen)
		{
			sx = 31 - sx;
			sy = 31 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if ((colorram[offs] & 0x10) != 0)
			drawgfx(bitmap,machine->gfx[2],
					videoram[offs] + 8 * (colorram[offs] & 0x20),
					colorram[offs] & 0x0f,
					flipx,flipy,
					8*sx,8*sy,
					cliprect,TRANSPARENCY_NONE,0);
	}
	return 0;
}


INTERRUPT_GEN( gyruss_6809_interrupt )
{
	scanline = 255 - cpu_getiloops();

	memcpy(sprite_mux_buffer + scanline * spriteram_size,spriteram,spriteram_size);

	if (scanline == 255)
		irq0_line_hold(machine, cpunum);
}
