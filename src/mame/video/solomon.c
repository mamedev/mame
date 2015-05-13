// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
#include "emu.h"
#include "includes/solomon.h"

WRITE8_MEMBER(solomon_state::solomon_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(solomon_state::solomon_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(solomon_state::solomon_videoram2_w)
{
	m_videoram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(solomon_state::solomon_colorram2_w)
{
	m_colorram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(solomon_state::solomon_flipscreen_w)
{
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(solomon_state::get_bg_tile_info)
{
	int attr = m_colorram2[tile_index];
	int code = m_videoram2[tile_index] + 256 * (attr & 0x07);
	int color = ((attr & 0x70) >> 4);
	int flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x08) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(solomon_state::get_fg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + 256 * (attr & 0x07);
	int color = (attr & 0x70) >> 4;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void solomon_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(solomon_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(solomon_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void solomon_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int code = spriteram[offs] + 16 * (spriteram[offs + 1] & 0x10);
		int color = (spriteram[offs + 1] & 0x0e) >> 1;
		int flipx = spriteram[offs + 1] & 0x40;
		int flipy = spriteram[offs + 1] & 0x80;
		int sx = spriteram[offs + 3];
		int sy = 241 - spriteram[offs + 2];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 242 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

UINT32 solomon_state::screen_update_solomon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
