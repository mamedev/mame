/******************************************************************************

    Markham (c) 1983 Sun Electronics

    Video hardware driver by Uki

    17/Jun/2001 -

******************************************************************************/

#include "emu.h"
#include "includes/markham.h"

PALETTE_INIT( markham )
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

WRITE8_MEMBER(markham_state::markham_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(markham_state::markham_flipscreen_w)
{
	if (flip_screen_get(machine()) != (data & 0x01))
	{
		flip_screen_set(machine(), data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	markham_state *state = machine.driver_data<markham_state>();
	int attr = state->m_videoram[tile_index * 2];
	int code = state->m_videoram[(tile_index * 2) + 1] + ((attr & 0x60) << 3);
	int color = (attr & 0x1f) | ((attr & 0x80) >> 2);

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( markham )
{
	markham_state *state = machine.driver_data<markham_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 8, 8, 32, 32);

	state->m_bg_tilemap->set_scroll_rows(32);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	markham_state *state = machine.driver_data<markham_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0x60; offs < 0x100; offs += 4)
	{
		int chr = spriteram[offs + 1];
		int col = spriteram[offs + 2];

		int fx = flip_screen_get(machine);
		int fy = flip_screen_get(machine);

		int x = spriteram[offs + 3];
		int y = spriteram[offs + 0];
		int px, py;
		col &= 0x3f ;

		if (flip_screen_get(machine) == 0)
		{
			px = x - 2;
			py = 240 - y;
		}
		else
		{
			px = 240 - x;
			py = y;
		}

		px = px & 0xff;

		if (px > 248)
			px = px - 256;

		drawgfx_transmask(bitmap,cliprect,machine.gfx[1],
			chr,
			col,
			fx,fy,
			px,py,
			colortable_get_transpen_mask(machine.colortable, machine.gfx[1], col, 0));
	}
}

SCREEN_UPDATE_IND16( markham )
{
	markham_state *state = screen.machine().driver_data<markham_state>();
	int i;

	for (i = 0; i < 32; i++)
	{
		if ((i > 3) && (i < 16))
			state->m_bg_tilemap->set_scrollx(i, state->m_xscroll[0]);
		if (i >= 16)
			state->m_bg_tilemap->set_scrollx(i, state->m_xscroll[1]);
	}

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
