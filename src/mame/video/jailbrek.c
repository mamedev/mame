// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "includes/jailbrek.h"

PALETTE_INIT_MEMBER(jailbrek_state, jailbrek)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int r = pal4bit(color_prom[i + 0x00] >> 0);
		int g = pal4bit(color_prom[i + 0x00] >> 4);
		int b = pal4bit(color_prom[i + 0x20] >> 0);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x40;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}

	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

WRITE8_MEMBER(jailbrek_state::jailbrek_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(jailbrek_state::jailbrek_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(jailbrek_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void jailbrek_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(jailbrek_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap->set_scrolldx(0, 396 - 256);
}

void jailbrek_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int i;

	for (i = 0; i < m_spriteram.bytes(); i += 4)
	{
		int attr = spriteram[i + 1];    // attributes = ?tyxcccc
		int code = spriteram[i] + ((attr & 0x40) << 2);
		int color = attr & 0x0f;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[i + 2] - ((attr & 0x80) << 1);
		int sy = spriteram[i + 3];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transmask(bitmap,cliprect, code, color, flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
	}
}

UINT32 jailbrek_state::screen_update_jailbrek(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	// added support for vertical scrolling (credits).  23/1/2002  -BR
	// bit 2 appears to be horizontal/vertical scroll control
	if (m_scroll_dir[0] & 0x04)
	{
		m_bg_tilemap->set_scroll_cols(32);
		m_bg_tilemap->set_scroll_rows(1);
		m_bg_tilemap->set_scrollx(0, 0);

		for (i = 0; i < 32; i++)
			m_bg_tilemap->set_scrolly(i, ((m_scroll_x[i + 32] << 8) + m_scroll_x[i]));
	}
	else
	{
		m_bg_tilemap->set_scroll_rows(32);
		m_bg_tilemap->set_scroll_cols(1);
		m_bg_tilemap->set_scrolly(0, 0);

		for (i = 0; i < 32; i++)
			m_bg_tilemap->set_scrollx(i, ((m_scroll_x[i + 32] << 8) + m_scroll_x[i]));
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
