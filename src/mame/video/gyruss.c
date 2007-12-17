/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


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
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* color_prom now points to the beginning of the sprite lookup table */

	/* sprites */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = *(color_prom++) & 0x0f;

	/* characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = (*(color_prom++) & 0x0f) + 0x10;
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
				drawgfx(bitmap,machine->gfx[1 + (sr[offs + 1] & 1)],
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

		drawgfx(bitmap,machine->gfx[0],
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
			drawgfx(bitmap,machine->gfx[0],
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
		irq0_line_hold();
}
