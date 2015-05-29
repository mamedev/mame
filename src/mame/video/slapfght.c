// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/***************************************************************************

  Toaplan Slap Fight hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/slapfght.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(slapfght_state::get_pf_tile_info)
{
	/* For Performan only */
	int tile = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x03) << 8);
	int color = (m_colorram[tile_index] >> 3) & 0x0f;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(slapfght_state::get_pf1_tile_info)
{
	int tile = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x0f) << 8);
	int color = (m_colorram[tile_index] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(slapfght_state::get_fix_tile_info)
{
	int tile = m_fixvideoram[tile_index] | ((m_fixcolorram[tile_index] & 0x03) << 8);
	int color = (m_fixcolorram[tile_index] & 0xfc) >> 2;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(slapfght_state, perfrman)
{
	m_pf1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(slapfght_state::get_pf_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_pf1_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(slapfght_state, slapfight)
{
	m_pf1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(slapfght_state::get_pf1_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(slapfght_state::get_fix_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fix_tilemap->set_scrolldy(0, 15);
	m_pf1_tilemap->set_scrolldy(0, 14);

	m_fix_tilemap->set_transparent_pen(0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(slapfght_state::videoram_w)
{
	m_videoram[offset] = data;
	m_pf1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(slapfght_state::colorram_w)
{
	m_colorram[offset] = data;
	m_pf1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(slapfght_state::fixram_w)
{
	m_fixvideoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(slapfght_state::fixcol_w)
{
	m_fixcolorram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(slapfght_state::scrollx_lo_w)
{
	m_scrollx_lo = data;
}

WRITE8_MEMBER(slapfght_state::scrollx_hi_w)
{
	m_scrollx_hi = data;
}

WRITE8_MEMBER(slapfght_state::scrolly_w)
{
	m_scrolly = data;
}

WRITE8_MEMBER(slapfght_state::flipscreen_w)
{
	flip_screen_set(offset ? 0 : 1);
}

WRITE8_MEMBER(slapfght_state::palette_bank_w)
{
	m_palette_bank = offset;
}



/***************************************************************************

  Render the Screen

***************************************************************************/

void slapfght_state::draw_perfrman_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer)
{
	UINT8 *src = m_spriteram->buffer();

	for (int offs = 0; offs < m_spriteram->bytes(); offs += 4)
	{
		/*
		    0: xxxxxxxx - code
		    1: xxxxxxxx - x
		    2: x....... - priority over backdrop
		       .x...... - sprite-sprite priority (see point-pop sprites)
		       ..x..... - ?
		       ...xx... - no function?
		       .....xxx - color
		    3: xxxxxxxx - y
		*/

		int code = src[offs + 0];
		int sy = src[offs + 3] - 1;
		int sx = src[offs + 1] - 13;
		int pri = src[offs + 2] >> 6 & 3;
		int color = (src[offs + 2] >> 1 & 3) | (src[offs + 2] << 2 & 4) | (m_palette_bank << 3);
		int fx = 0, fy = 0;

		if (flip_screen())
		{
			sy = 256 - sy;
			sx = 240 - sx;
			fx = fy = 1;
		}

		if (layer == pri)
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, fx, fy, sx, sy, 0);
	}
}

UINT32 slapfght_state::screen_update_perfrman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pf1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE);
	draw_perfrman_sprites(bitmap, cliprect, 0);
	draw_perfrman_sprites(bitmap, cliprect, 1);

	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0);
	draw_perfrman_sprites(bitmap, cliprect, 2);
	draw_perfrman_sprites(bitmap, cliprect, 3);

	return 0;
}


void slapfght_state::draw_slapfight_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *src = m_spriteram->buffer();

	for (int offs = 0; offs < m_spriteram->bytes(); offs += 4)
	{
		/*
		    0: xxxxxxxx - code low
		    1: xxxxxxxx - x low
		    2: xx...... - code high
		       ..x..... - no function?
		       ...xxxx. - color
		       .......x - x high
		    3: xxxxxxxx - y
		*/

		int code = src[offs + 0] | ((src[offs + 2] & 0xc0) << 2);
		int sy = src[offs + 3];
		int sx = (src[offs + 1] | (src[offs + 2] << 8 & 0x100)) - 13;
		int color = src[offs + 2] >> 1 & 0xf;
		int fx = 0, fy = 0;

		if (flip_screen())
		{
			sy = (238 - sy) & 0xff;
			sx = 284 - sx;
			fx = fy = 1;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, color, fx, fy, sx, sy, 0);
	}
}

UINT32 slapfght_state::screen_update_slapfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pf1_tilemap->set_scrollx(m_scrollx_hi << 8 | m_scrollx_lo);
	m_pf1_tilemap->set_scrolly(m_scrolly);

	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_slapfight_sprites(bitmap, cliprect);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
