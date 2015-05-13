// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "includes/exprraid.h"


WRITE8_MEMBER(exprraid_state::exprraid_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(exprraid_state::exprraid_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(exprraid_state::exprraid_flipscreen_w)
{
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(exprraid_state::exprraid_bgselect_w)
{
	if (m_bg_index[offset] != data)
	{
		m_bg_index[offset] = data;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(exprraid_state::exprraid_scrollx_w)
{
	m_bg_tilemap->set_scrollx(offset, data);
}

WRITE8_MEMBER(exprraid_state::exprraid_scrolly_w)
{
	m_bg_tilemap->set_scrolly(0, data);
}

TILE_GET_INFO_MEMBER(exprraid_state::get_bg_tile_info)
{
	UINT8 *tilerom = memregion("gfx4")->base();

	int data, attr, bank, code, color, flags;
	int quadrant = 0, offs;

	int sx = tile_index % 32;
	int sy = tile_index / 32;

	if (sx >= 16) quadrant++;
	if (sy >= 16) quadrant += 2;

	offs = (sy % 16) * 16 + (sx % 16) + (m_bg_index[quadrant] & 0x3f) * 0x100;

	data = tilerom[offs];
	attr = tilerom[offs + 0x4000];
	bank = (2 * (attr & 0x03) + ((data & 0x80) >> 7)) + 2;
	code = data & 0x7f;
	color = (attr & 0x18) >> 3;
	flags = (attr & 0x04) ? TILE_FLIPX : 0;

	tileinfo.category = ((attr & 0x80) ? 1 : 0);

	SET_TILE_INFO_MEMBER(bank, code, color, flags);
}

TILE_GET_INFO_MEMBER(exprraid_state::get_fg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0x07) << 8);
	int color = (attr & 0x10) >> 4;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void exprraid_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(exprraid_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(exprraid_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(2);
	m_fg_tilemap->set_transparent_pen(0);
}

void exprraid_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int attr = m_spriteram[offs + 1];
		int code = m_spriteram[offs + 3] + ((attr & 0xe0) << 3);
		int color = (attr & 0x03) + ((attr & 0x08) >> 1);
		int flipx = (attr & 0x04);
		int flipy = 0;
		int sx = ((248 - m_spriteram[offs + 2]) & 0xff) - 8;
		int sy = m_spriteram[offs];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);

		/* double height */

		if (attr & 0x10)
		{
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code + 1, color,
				flipx, flipy,
				sx, sy + (flip_screen() ? -16 : 16), 0);
		}
	}
}

UINT32 exprraid_state::screen_update_exprraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
