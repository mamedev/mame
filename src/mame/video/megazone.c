/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/megazone.h"

/***************************************************************************
Based on driver from MAME 0.55
Changes by Martin M. (pfloyd@gmx.net) 14.10.2001:

 - Added support for screen flip in cocktail mode (tricky!) */


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Megazone has one 32x8 palette PROM and two 256x4 lookup table PROMs
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

PALETTE_INIT( megazone )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances_rg[0], rweights, 1000, 0,
			3, &resistances_rg[0], gweights, 1000, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* sprites */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	/* characters */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}

WRITE8_MEMBER(megazone_state::megazone_flipscreen_w)
{
	m_flipscreen = data & 1;
}

VIDEO_START( megazone )
{
	megazone_state *state = machine.driver_data<megazone_state>();
	state->m_tmpbitmap = auto_bitmap_ind16_alloc(machine, 256, 256);

	state->save_item(NAME(*state->m_tmpbitmap));
}


SCREEN_UPDATE_IND16( megazone )
{
	megazone_state *state = screen.machine().driver_data<megazone_state>();
	int offs;
	int x, y;

	/* for every character in the Video RAM */
	for (offs = state->m_videoram_size - 1; offs >= 0; offs--)
	{
		int sx, sy, flipx, flipy;

		sx = offs % 32;
		sy = offs / 32;
		flipx = state->m_colorram[offs] & (1 << 6);
		flipy = state->m_colorram[offs] & (1 << 5);

		if (state->m_flipscreen)
		{
			sx = 31 - sx;
			sy = 31 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_opaque(*state->m_tmpbitmap, state->m_tmpbitmap->cliprect(), screen.machine().gfx[1],
				((int)state->m_videoram[offs]) + ((state->m_colorram[offs] & (1 << 7) ? 256 : 0) ),
				(state->m_colorram[offs] & 0x0f) + 0x10,
				flipx,flipy,
				8*sx,8*sy);
	}

	/* copy the temporary bitmap to the screen */
	{
		int scrollx;
		int scrolly;

		if (state->m_flipscreen)
		{
			scrollx = *state->m_scrolly;
			scrolly = *state->m_scrollx;
		}
		else
		{
			scrollx = - *state->m_scrolly + 4 * 8; // leave space for credit&score overlay
			scrolly = - *state->m_scrollx;
		}


		copyscrollbitmap(bitmap, *state->m_tmpbitmap, 1, &scrollx, 1, &scrolly, cliprect);
	}


	/* Draw the sprites. */
	{
		UINT8 *spriteram = state->m_spriteram;
		for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
		{
			int sx = spriteram[offs + 3];
			int sy = 255 - ((spriteram[offs + 1] + 16) & 0xff);
			int color =  spriteram[offs + 0] & 0x0f;
			int flipx = ~spriteram[offs + 0] & 0x40;
			int flipy =  spriteram[offs + 0] & 0x80;

			if (state->m_flipscreen)
			{
				sx = sx - 11;
				sy = sy + 2;
			}
			else
				sx = sx + 32;

			drawgfx_transmask(bitmap,cliprect,screen.machine().gfx[0],
					spriteram[offs + 2],
					color,
					flipx,flipy,
					sx,sy,
					colortable_get_transpen_mask(screen.machine().colortable, screen.machine().gfx[0], color, 0));
		}
	}

	for (y = 0; y < 32;y++)
	{
		offs = y * 32;
		for (x = 0; x < 6; x++)
		{
			int sx, sy, flipx, flipy;

			sx = x;
			sy = y;

			flipx = state->m_colorram2[offs] & (1 << 6);
			flipy = state->m_colorram2[offs] & (1 << 5);

			if (state->m_flipscreen)
			{
				sx = 35 - sx;
				sy = 31 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}




			drawgfx_opaque(bitmap, cliprect, screen.machine().gfx[1],
					((int)state->m_videoram2[offs]) + ((state->m_colorram2[offs] & (1 << 7) ? 256 : 0) ),
					(state->m_colorram2[offs] & 0x0f) + 0x10,
					flipx,flipy,
					8*sx,8*sy);
			offs++;
		}
	}
	return 0;
}
