// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/bombjack.h"

WRITE8_MEMBER(bombjack_state::bombjack_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bombjack_state::bombjack_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bombjack_state::bombjack_background_w)
{
	if (m_background_image != data)
	{
		m_background_image = data;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(bombjack_state::bombjack_flipscreen_w)
{
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(bombjack_state::get_bg_tile_info)
{
	UINT8 *tilerom = memregion("gfx4")->base();

	int offs = (m_background_image & 0x07) * 0x200 + tile_index;
	int code = (m_background_image & 0x10) ? tilerom[offs] : 0;
	int attr = tilerom[offs + 0x100];
	int color = attr & 0x0f;
	int flags = (attr & 0x80) ? TILE_FLIPY : 0;

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(bombjack_state::get_fg_tile_info)
{
	int code = m_videoram[tile_index] + 16 * (m_colorram[tile_index] & 0x10);
	int color = m_colorram[tile_index] & 0x0f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void bombjack_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bombjack_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bombjack_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void bombjack_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
/*
 abbbbbbb cdefgggg hhhhhhhh iiiiiiii

 a        use big sprites (32x32 instead of 16x16)
 bbbbbbb  sprite code
 c        x flip
 d        y flip (used only in death sequence?)
 e        ? (set when big sprites are selected)
 f        ? (set only when the bonus (B) materializes?)
 gggg     color
 hhhhhhhh x position
 iiiiiiii y position
*/
		int sx,sy,flipx,flipy;


		sx = m_spriteram[offs + 3];

		if (m_spriteram[offs] & 0x80)
			sy = 225 - m_spriteram[offs + 2];
		else
			sy = 241 - m_spriteram[offs + 2];

		flipx = m_spriteram[offs + 1] & 0x40;
		flipy = m_spriteram[offs + 1] & 0x80;

		if (flip_screen())
		{
			if (m_spriteram[offs + 1] & 0x20)
			{
				sx = 224 - sx;
				sy = 224 - sy;
			}
			else
			{
				sx = 240 - sx;
				sy = 240 - sy;
			}
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx((m_spriteram[offs] & 0x80) ? 3 : 2)->transpen(bitmap,cliprect,
				m_spriteram[offs] & 0x7f,
				m_spriteram[offs + 1] & 0x0f,
				flipx,flipy,
				sx,sy,0);
	}
}

UINT32 bombjack_state::screen_update_bombjack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
