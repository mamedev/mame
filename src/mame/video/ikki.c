/******************************************************************************

Ikki (c) 1985 Sun Electronics

Video hardware driver by Uki

    20/Jun/2001 -

******************************************************************************/

#include "emu.h"
#include "includes/ikki.h"

PALETTE_INIT( ikki )
{
	ikki_state *state = machine.driver_data<ikki_state>();
	int i;

	/* allocate the colortable - extra pen for the punch through pen */
	machine.colortable = colortable_alloc(machine, 0x101);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* sprites lookup table */
	for (i = 0; i < 0x200; i++)
	{
		UINT16 ctabentry = color_prom[i] ^ 0xff;

		if (((i & 0x07) == 0x07) && (ctabentry == 0))
		{
			/* punch through */
			state->m_punch_through_pen = i;
			ctabentry = 0x100;
		}

		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	/* bg lookup table */
	for (i = 0x200; i < 0x400; i++)
	{
		UINT8 ctabentry = color_prom[i];
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}

WRITE8_MEMBER(ikki_state::ikki_scrn_ctrl_w)
{
	m_flipscreen = (data >> 2) & 1;
}


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	ikki_state *state = machine.driver_data<ikki_state>();
	UINT8 *spriteram = state->m_spriteram;
	int y;
	offs_t offs;

	state->m_sprite_bitmap.fill(state->m_punch_through_pen, cliprect);

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int code = (spriteram[offs + 2] & 0x80) | (spriteram[offs + 1] >> 1);
		int color = spriteram[offs + 2] & 0x3f;

		int x = spriteram[offs + 3];
		    y = spriteram[offs + 0];

		if (state->m_flipscreen)
			x = 240 - x;
		else
			y = 224 - y;

		x = x & 0xff;
		y = y & 0xff;

		if (x > 248)
			x = x - 256;

		if (y > 240)
			y = y - 256;

		drawgfx_transmask(state->m_sprite_bitmap,cliprect, machine.gfx[1],
				code, color,
				state->m_flipscreen,state->m_flipscreen,
				x,y,
				colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, 0));
	}

	/* copy the sprite bitmap into the main bitmap, skipping the transparent pixels */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int x;

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 pen = state->m_sprite_bitmap.pix16(y, x);

			if (colortable_entry_get_value(machine.colortable, pen) != 0x100)
				bitmap.pix16(y, x) = pen;
		}
	}
}


VIDEO_START( ikki )
{
	ikki_state *state = machine.driver_data<ikki_state>();
	machine.primary_screen->register_screen_bitmap(state->m_sprite_bitmap);
	state->save_item(NAME(state->m_sprite_bitmap));
}


SCREEN_UPDATE_IND16( ikki )
{
	ikki_state *state = screen.machine().driver_data<ikki_state>();
	offs_t offs;
	UINT8 *VIDEOATTR = screen.machine().region("user1")->base();

	/* draw bg layer */

	for (offs = 0; offs < (state->m_videoram_size / 2); offs++)
	{
		int color, bank;

		int sx = offs / 32;
		int sy = offs % 32;
		int y = sy*8;
		int x = sx*8;

		int d = VIDEOATTR[sx];

		switch (d)
		{
			case 0x02: /* scroll area */
				x = sx * 8 - state->m_scroll[1];
				if (x < 0)
					x += 8 * 22;
				y = (sy * 8 + ~state->m_scroll[0]) & 0xff;
				break;

			case 0x03: /* non-scroll area */
				break;

			case 0x00: /* sprite disable? */
				break;

			case 0x0d: /* sprite disable? */
				break;

			case 0x0b: /* non-scroll area (?) */
				break;

			case 0x0e: /* unknown */
				break;
		}

		if (state->m_flipscreen)
		{
			x = 248 - x;
			y = 248 - y;
		}

		color = state->m_videoram[offs * 2];
		bank = (color & 0xe0) << 3;
		color = ((color & 0x1f)<<0) | ((color & 0x80) >> 2);

		drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],
			state->m_videoram[offs * 2 + 1] + bank,
			color,
			state->m_flipscreen,state->m_flipscreen,
			x,y);
	}

	draw_sprites(screen.machine(), bitmap, cliprect);

	/* mask sprites */

	for (offs = 0; offs < (state->m_videoram_size / 2); offs++)
	{
		int sx = offs / 32;
		int sy = offs % 32;

		int d = VIDEOATTR[sx];

		if ((d == 0) || (d == 0x0d))
		{
			int color, bank;

			int y = sy * 8;
			int x = sx * 8;

			if (state->m_flipscreen)
			{
				x = 248 - x;
				y = 248 - y;
			}

			color = state->m_videoram[offs * 2];
			bank = (color & 0xe0) << 3;
			color = ((color & 0x1f)<<0) | ((color & 0x80) >> 2);

			drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],
				state->m_videoram[offs * 2 + 1] + bank,
				color,
				state->m_flipscreen,state->m_flipscreen,
				x,y);
		}
	}

	return 0;
}
