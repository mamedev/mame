/***************************************************************************

  video/balsente.c

  Functions to emulate the video hardware of the machine.

****************************************************************************/

#include "driver.h"
#include "includes/balsente.h"


/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( balsente )
{
	balsente_state *state = (balsente_state *)machine->driver_data;

	/* reset the system */
	state->palettebank_vis = 0;
	state->sprite_bank[0] = memory_region(machine, "gfx1");
	state->sprite_bank[1] = memory_region(machine, "gfx1") + 0x10000;

	/* determine sprite size */
	state->sprite_data = memory_region(machine, "gfx1");
	state->sprite_mask = memory_region_length(machine, "gfx1") - 1;

	/* register for saving */
	state_save_register_global_array(machine, state->videoram);
	state_save_register_global(machine, state->palettebank_vis);
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

WRITE8_HANDLER( balsente_videoram_w )
{
	balsente_state *state = (balsente_state *)space->machine->driver_data;

	space->machine->generic.videoram.u8[offset] = data;

	/* expand the two pixel values into two bytes */
	state->videoram[offset * 2 + 0] = data >> 4;
	state->videoram[offset * 2 + 1] = data & 15;
}



/*************************************
 *
 *  Palette banking
 *
 *************************************/

WRITE8_HANDLER( balsente_palette_select_w )
{
	balsente_state *state = (balsente_state *)space->machine->driver_data;

	/* only update if changed */
	if (state->palettebank_vis != (data & 3))
	{
		/* update the scanline palette */
		video_screen_update_partial(space->machine->primary_screen, video_screen_get_vpos(space->machine->primary_screen) - 1 + BALSENTE_VBEND);
		state->palettebank_vis = data & 3;
	}

	logerror("balsente_palette_select_w(%d) scanline=%d\n", data & 3, video_screen_get_vpos(space->machine->primary_screen));
}



/*************************************
 *
 *  Palette RAM write
 *
 *************************************/

WRITE8_HANDLER( balsente_paletteram_w )
{
	int r, g, b;

	space->machine->generic.paletteram.u8[offset] = data & 0x0f;

	r = space->machine->generic.paletteram.u8[(offset & ~3) + 0];
	g = space->machine->generic.paletteram.u8[(offset & ~3) + 1];
	b = space->machine->generic.paletteram.u8[(offset & ~3) + 2];

	palette_set_color_rgb(space->machine, offset / 4, pal4bit(r), pal4bit(g), pal4bit(b));
}



/*************************************
 *
 *  Sprite banking
 *
 *************************************/

WRITE8_HANDLER( shrike_sprite_select_w )
{
	balsente_state *state = (balsente_state *)space->machine->driver_data;
	if( state->sprite_data != state->sprite_bank[(data & 0x80 >> 7) ^ 1 ])
	{
		logerror( "shrike_sprite_select_w( 0x%02x )\n", data );
		video_screen_update_partial(space->machine->primary_screen, video_screen_get_vpos(space->machine->primary_screen) - 1 + BALSENTE_VBEND);
		state->sprite_data = state->sprite_bank[(data & 0x80 >> 7) ^ 1];
	}

	shrike_shared_6809_w( space, 1, data );
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

static void draw_one_sprite(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *sprite)
{
	balsente_state *state = (balsente_state *)machine->driver_data;
	int flags = sprite[0];
	int image = sprite[1] | ((flags & 7) << 8);
	int ypos = sprite[2] + 17 + BALSENTE_VBEND;
	int xpos = sprite[3];
	UINT8 *src;
	int x, y;

	/* get a pointer to the source image */
	src = &state->sprite_data[(64 * image) & state->sprite_mask];
	if (flags & 0x80) src += 4 * 15;

	/* loop over y */
	for (y = 0; y < 16; y++, ypos = (ypos + 1) & 255)
	{
		if (ypos >= (16 + BALSENTE_VBEND) && ypos >= cliprect->min_y && ypos <= cliprect->max_y)
		{
			const pen_t *pens = &machine->pens[state->palettebank_vis * 256];
			UINT8 *old = &state->videoram[(ypos - BALSENTE_VBEND) * 256 + xpos];
			int currx = xpos;

			/* standard case */
			if (!(flags & 0x40))
			{
				/* loop over x */
				for (x = 0; x < 4; x++, old += 2)
				{
					int ipixel = *src++;
					int left = ipixel & 0xf0;
					int right = (ipixel << 4) & 0xf0;

					/* left pixel, combine with the background */
					if (left && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx) = pens[left | old[0]];
					currx++;

					/* right pixel, combine with the background */
					if (right && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx) = pens[right | old[1]];
					currx++;
				}
			}

			/* hflip case */
			else
			{
				src += 4;

				/* loop over x */
				for (x = 0; x < 4; x++, old += 2)
				{
					int ipixel = *--src;
					int left = (ipixel << 4) & 0xf0;
					int right = ipixel & 0xf0;

					/* left pixel, combine with the background */
					if (left && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx) = pens[left | old[0]];
					currx++;

					/* right pixel, combine with the background */
					if (right && currx >= 0 && currx < 256)
						*BITMAP_ADDR16(bitmap, ypos, currx) = pens[right | old[1]];
					currx++;
				}
				src += 4;
			}
		}
		else
			src += 4;
		if (flags & 0x80) src -= 2 * 4;
	}
}



/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

VIDEO_UPDATE( balsente )
{
	balsente_state *state = (balsente_state *)screen->machine->driver_data;
	const pen_t *pens = &screen->machine->pens[state->palettebank_vis * 256];
	int y, i;

	/* draw scanlines from the VRAM directly */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		draw_scanline8(bitmap, 0, y, 256, &state->videoram[(y - BALSENTE_VBEND) * 256], pens);

	/* draw the sprite images */
	for (i = 0; i < 40; i++)
		draw_one_sprite(screen->machine, bitmap, cliprect, &screen->machine->generic.spriteram.u8[(0xe0 + i * 4) & 0xff]);

	return 0;
}
