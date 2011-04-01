/******************************************************************************

Strength & Skill (c) 1984 Sun Electronics

Video hardware driver by Uki

    19/Jun/2001 -

******************************************************************************/

#include "emu.h"
#include "includes/strnskil.h"


PALETTE_INIT( strnskil )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

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
	for (i = 0; i < 0x400; i++)
	{
		UINT8 ctabentry = color_prom[i];
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}


WRITE8_HANDLER( strnskil_videoram_w )
{
	strnskil_state *state = space->machine().driver_data<strnskil_state>();
	UINT8 *videoram = state->m_videoram;
	videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset / 2);
}

WRITE8_HANDLER( strnskil_scrl_ctrl_w )
{
	strnskil_state *state = space->machine().driver_data<strnskil_state>();
	state->m_scrl_ctrl = data >> 5;

	if (flip_screen_get(space->machine()) != (data & 0x08))
	{
		flip_screen_set(space->machine(), data & 0x08);
		tilemap_mark_all_tiles_dirty_all(space->machine());
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	strnskil_state *state = machine.driver_data<strnskil_state>();
	UINT8 *videoram = state->m_videoram;
	int attr = videoram[tile_index * 2];
	int code = videoram[(tile_index * 2) + 1] + ((attr & 0x60) << 3);
	int color = (attr & 0x1f) | ((attr & 0x80) >> 2);

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( strnskil )
{
	strnskil_state *state = machine.driver_data<strnskil_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,
		 8, 8, 32, 32);

	tilemap_set_scroll_rows(state->m_bg_tilemap, 32);
}

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	strnskil_state *state = machine.driver_data<strnskil_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0x60; offs < 0x100; offs += 4)
	{
		int code = spriteram[offs + 1];
		int color = spriteram[offs + 2] & 0x3f;
		int flipx = flip_screen_x_get(machine);
		int flipy = flip_screen_y_get(machine);

		int sx = spriteram[offs + 3];
		int sy = spriteram[offs];
		int px, py;

		if (flip_screen_get(machine))
		{
			px = 240 - sx + 0; /* +2 or +0 ? */
			py = sy;
		}
		else
		{
			px = sx - 2;
			py = 240 - sy;
		}

		sx = sx & 0xff;

		if (sx > 248)
			sx = sx - 256;

		drawgfx_transmask(bitmap, cliprect,
			machine.gfx[1],
			code, color,
			flipx, flipy,
			px, py,
			colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, 0));
	}
}

SCREEN_UPDATE( strnskil )
{
	strnskil_state *state = screen->machine().driver_data<strnskil_state>();
	int row;
	const UINT8 *usr1 = screen->machine().region("user1")->base();

	for (row = 0; row < 32; row++)
	{
		if (state->m_scrl_ctrl != 0x07)
		{
			switch (usr1[state->m_scrl_ctrl * 32 + row])
			{
			case 2:
				tilemap_set_scrollx(state->m_bg_tilemap, row, -~state->m_xscroll[1]);
				break;
			case 4:
				tilemap_set_scrollx(state->m_bg_tilemap, row, -~state->m_xscroll[0]);
				break;
			}
		}
	}

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
