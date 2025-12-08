// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/***************************************************************************

  Toaplan Slap Fight hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "slapfght.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(perfrman_state::get_pf_tile_info)
{
	/* For Performan only */
	const int tile = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x03) << 8);
	const int color = (m_colorram[tile_index] >> 3) & 0x0f;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(tigerh_state::get_pf1_tile_info)
{
	const int tile = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x0f) << 8);
	const int color = (m_colorram[tile_index] & 0xf0) >> 4;

	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(tigerh_state::get_fix_tile_info)
{
	const int tile = m_fixvideoram[tile_index] | ((m_fixcolorram[tile_index] & 0x03) << 8);
	const int color = (m_fixcolorram[tile_index] & 0xfc) >> 2;

	tileinfo.set(0, tile, color, 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void perfrman_state::video_start()
{
	m_pf1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(perfrman_state::get_pf_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_pf1_tilemap->set_scrolldy(-16, 0);

	m_pf1_tilemap->set_transparent_pen(0);
}

void tigerh_state::video_start()
{
	m_pf1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tigerh_state::get_pf1_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tigerh_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fix_tilemap->set_scrolldy(-16, 0);
	m_pf1_tilemap->set_scrolldy(-17, -1);

	m_fix_tilemap->set_transparent_pen(0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void perfrman_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_pf1_tilemap->mark_tile_dirty(offset);
}

void perfrman_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_pf1_tilemap->mark_tile_dirty(offset);
}

void tigerh_state::fixram_w(offs_t offset, uint8_t data)
{
	m_fixvideoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

void tigerh_state::fixcol_w(offs_t offset, uint8_t data)
{
	m_fixcolorram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

void tigerh_state::scrollx_lo_w(uint8_t data)
{
	m_scrollx_lo = data;
}

void tigerh_state::scrollx_hi_w(uint8_t data)
{
	m_scrollx_hi = data;
}

void tigerh_state::scrolly_w(uint8_t data)
{
	m_scrolly = data;
}

void perfrman_state::flipscreen_w(int state)
{
	flip_screen_set(state ? 0 : 1);
}

void perfrman_state::palette_bank_w(int state)
{
	m_palette_bank = state;
}



/***************************************************************************

  Render the Screen

***************************************************************************/

void perfrman_state::draw_perfrman_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer)
{
	const uint8_t *src = m_spriteram->buffer();

	for (int offs = 0; offs < 0x800; offs += 4)
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

		const int code = src[offs + 0];
		int sy = src[offs + 3] - 17;
		int sx = src[offs + 1] - 13;
		const int pri = src[offs + 2] >> 6 & 3;
		const int color = (src[offs + 2] >> 1 & 3) | (src[offs + 2] << 2 & 4) | (m_palette_bank << 3);
		bool fx = false, fy = false;

		if (flip_screen())
		{
			sy = (206 - sy) & 0xff;
			sx = 284 - sx;
			fx = fy = true;
		}

		if (layer == pri)
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, fx, fy, sx, sy, 0);
	}
}

uint32_t perfrman_state::screen_update_perfrman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pf1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE);
	draw_perfrman_sprites(bitmap, cliprect, 0);
	draw_perfrman_sprites(bitmap, cliprect, 1);

	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0);
	draw_perfrman_sprites(bitmap, cliprect, 2);
	draw_perfrman_sprites(bitmap, cliprect, 3);

	return 0;
}


void tigerh_state::draw_slapfight_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint8_t *src = m_spriteram->buffer();

	for (int offs = 0; offs < 0x800; offs += 4)
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

		const int code = src[offs + 0] | ((src[offs + 2] & 0xc0) << 2);
		int sy = src[offs + 3] - 17;
		int sx = (src[offs + 1] | (src[offs + 2] << 8 & 0x100)) - 13;
		const int color = src[offs + 2] >> 1 & 0xf;
		bool fx = false, fy = false;

		if (flip_screen())
		{
			sy = (206 - sy) & 0xff;
			sx = 284 - sx;
			fx = fy = true;
		}

		if (sy > 256-8)
			sy -= 256;

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, color, fx, fy, sx, sy, 0);
	}
}

uint32_t tigerh_state::screen_update_slapfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pf1_tilemap->set_scrollx(m_scrollx_hi << 8 | m_scrollx_lo);
	m_pf1_tilemap->set_scrolly(m_scrolly);

	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_slapfight_sprites(bitmap, cliprect);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
