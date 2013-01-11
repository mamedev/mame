/******************************************************************************

Strength & Skill (c) 1984 Sun Electronics

Video hardware driver by Uki

    19/Jun/2001 -

******************************************************************************/

#include "emu.h"
#include "includes/strnskil.h"


void strnskil_state::palette_init()
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


WRITE8_MEMBER(strnskil_state::strnskil_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(strnskil_state::strnskil_scrl_ctrl_w)
{
	m_scrl_ctrl = data >> 5;

	if (flip_screen() != (data & 0x08))
	{
		flip_screen_set(data & 0x08);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(strnskil_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int attr = videoram[tile_index * 2];
	int code = videoram[(tile_index * 2) + 1] + ((attr & 0x60) << 3);
	int color = (attr & 0x1f) | ((attr & 0x80) >> 2);

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void strnskil_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(strnskil_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS,
			8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	strnskil_state *state = machine.driver_data<strnskil_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0x60; offs < 0x100; offs += 4)
	{
		int code = spriteram[offs + 1];
		int color = spriteram[offs + 2] & 0x3f;
		int flipx = state->flip_screen_x();
		int flipy = state->flip_screen_y();

		int sx = spriteram[offs + 3];
		int sy = spriteram[offs];
		int px, py;

		if (state->flip_screen())
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

UINT32 strnskil_state::screen_update_strnskil(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int row;
	const UINT8 *usr1 = memregion("user1")->base();

	for (row = 0; row < 32; row++)
	{
		if (m_scrl_ctrl != 0x07)
		{
			switch (usr1[m_scrl_ctrl * 32 + row])
			{
			case 2:
				m_bg_tilemap->set_scrollx(row, -~m_xscroll[1]);
				break;
			case 4:
				m_bg_tilemap->set_scrollx(row, -~m_xscroll[0]);
				break;
			}
		}
	}

	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(machine(), bitmap, cliprect);
	return 0;
}
