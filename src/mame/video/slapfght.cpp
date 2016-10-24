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

void slapfght_state::get_pf_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	/* For Performan only */
	int tile = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x03) << 8);
	int color = (m_colorram[tile_index] >> 3) & 0x0f;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

void slapfght_state::get_pf1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tile = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x0f) << 8);
	int color = (m_colorram[tile_index] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(1, tile, color, 0);
}

void slapfght_state::get_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tile = m_fixvideoram[tile_index] | ((m_fixcolorram[tile_index] & 0x03) << 8);
	int color = (m_fixcolorram[tile_index] & 0xfc) >> 2;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void slapfght_state::video_start_perfrman()
{
	m_pf1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(slapfght_state::get_pf_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_pf1_tilemap->set_transparent_pen(0);
}

void slapfght_state::video_start_slapfight()
{
	m_pf1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(slapfght_state::get_pf1_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(slapfght_state::get_fix_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fix_tilemap->set_scrolldy(0, 15);
	m_pf1_tilemap->set_scrolldy(0, 14);

	m_fix_tilemap->set_transparent_pen(0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void slapfght_state::videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram[offset] = data;
	m_pf1_tilemap->mark_tile_dirty(offset);
}

void slapfght_state::colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_colorram[offset] = data;
	m_pf1_tilemap->mark_tile_dirty(offset);
}

void slapfght_state::fixram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_fixvideoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

void slapfght_state::fixcol_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_fixcolorram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}

void slapfght_state::scrollx_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_scrollx_lo = data;
}

void slapfght_state::scrollx_hi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_scrollx_hi = data;
}

void slapfght_state::scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_scrolly = data;
}

void slapfght_state::flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	flip_screen_set(offset ? 0 : 1);
}

void slapfght_state::palette_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_palette_bank = offset;
}



/***************************************************************************

  Render the Screen

***************************************************************************/

void slapfght_state::draw_perfrman_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer)
{
	uint8_t *src = m_spriteram->buffer();

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

uint32_t slapfght_state::screen_update_perfrman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
	uint8_t *src = m_spriteram->buffer();

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

uint32_t slapfght_state::screen_update_slapfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pf1_tilemap->set_scrollx(m_scrollx_hi << 8 | m_scrollx_lo);
	m_pf1_tilemap->set_scrolly(m_scrolly);

	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_slapfight_sprites(bitmap, cliprect);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
