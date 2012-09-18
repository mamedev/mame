/******************************************************************************

    Markham (c) 1983 Sun Electronics

    Video hardware driver by Uki

    17/Jun/2001 -

******************************************************************************/

#include "emu.h"
#include "includes/markham.h"

void markham_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine().colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* sprites lookup table */
	for (i = 0; i < 0x400; i++)
	{
		UINT8 ctabentry = color_prom[i];
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}
}

WRITE8_MEMBER(markham_state::markham_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(markham_state::markham_flipscreen_w)
{
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(markham_state::get_bg_tile_info)
{
	int attr = m_videoram[tile_index * 2];
	int code = m_videoram[(tile_index * 2) + 1] + ((attr & 0x60) << 3);
	int color = (attr & 0x1f) | ((attr & 0x80) >> 2);

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void markham_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(markham_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);
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

		int fx = state->flip_screen();
		int fy = state->flip_screen();

		int x = spriteram[offs + 3];
		int y = spriteram[offs + 0];
		int px, py;
		col &= 0x3f ;

		if (state->flip_screen() == 0)
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

UINT32 markham_state::screen_update_markham(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if ((i > 3) && (i < 16))
			m_bg_tilemap->set_scrollx(i, m_xscroll[0]);
		if (i >= 16)
			m_bg_tilemap->set_scrollx(i, m_xscroll[1]);
	}

	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(machine(), bitmap, cliprect);
	return 0;
}
