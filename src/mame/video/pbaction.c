// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/pbaction.h"

WRITE8_MEMBER(pbaction_state::pbaction_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(pbaction_state::pbaction_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(pbaction_state::pbaction_videoram2_w)
{
	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(pbaction_state::pbaction_colorram2_w)
{
	m_colorram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(pbaction_state::pbaction_scroll_w)
{
	m_scroll = data - 3;
	if (flip_screen())
		m_scroll = -m_scroll;

	m_bg_tilemap->set_scrollx(0, m_scroll);
	m_fg_tilemap->set_scrollx(0, m_scroll);
}

WRITE8_MEMBER(pbaction_state::pbaction_flipscreen_w)
{
	flip_screen_set(data & 0x01);
}

TILE_GET_INFO_MEMBER(pbaction_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + 0x10 * (attr & 0x70);
	int color = attr & 0x07;
	int flags = (attr & 0x80) ? TILE_FLIPY : 0;

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(pbaction_state::get_fg_tile_info)
{
	int attr = m_colorram2[tile_index];
	int code = m_videoram2[tile_index] + 0x10 * (attr & 0x30);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void pbaction_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pbaction_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pbaction_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void pbaction_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, flipx, flipy;

		/* if next sprite is double size, skip this one */
		if (offs > 0 && spriteram[offs - 4] & 0x80)
			continue;

		sx = spriteram[offs + 3];

		if (spriteram[offs] & 0x80)
			sy = 225 - spriteram[offs + 2];
		else
			sy = 241 - spriteram[offs + 2];

		flipx = spriteram[offs + 1] & 0x40;
		flipy = spriteram[offs + 1] & 0x80;

		if (flip_screen())
		{
			if (spriteram[offs] & 0x80)
			{
				sx = 224 - sx;
				sy = 225 - sy;
			}
			else
			{
				sx = 240 - sx;
				sy = 241 - sy;
			}
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx((spriteram[offs] & 0x80) ? 3 : 2)->transpen(bitmap,cliprect, /* normal or double size */
				spriteram[offs],
				spriteram[offs + 1] & 0x0f,
				flipx,flipy,
				sx + (flip_screen() ? m_scroll : -m_scroll), sy,0);
	}
}

UINT32 pbaction_state::screen_update_pbaction(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
