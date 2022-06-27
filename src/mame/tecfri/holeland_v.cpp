// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/***************************************************************************

  holeland.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "holeland.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(holeland_state::holeland_get_tile_info)
{
	/*
	x--- ---- priority (1) behind sprites
	xxxx ---- color
	---- xx-- flip yx
	---- --xx tile upper bits
	*/

	int attr = m_colorram[tile_index];
	int tile_number = m_videoram[tile_index] | ((attr & 0x03) << 8);

	tileinfo.set(0,
			tile_number,
			m_palette_offset + ((attr >> 4) & 0x0f),
			TILE_FLIPYX((attr >> 2) & 0x03));
	tileinfo.group = (attr >> 7) & 1;
}

TILE_GET_INFO_MEMBER(holeland_state::crzrally_get_tile_info)
{
	int attr = m_colorram[tile_index];
	int tile_number = m_videoram[tile_index] | ((attr & 0x03) << 8);

	tileinfo.set(0,
			tile_number,
			m_palette_offset + ((attr >> 4) & 0x0f),
			TILE_FLIPYX((attr >> 2) & 0x03));
	tileinfo.group = (attr >> 5) & 1;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(holeland_state,holeland)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(holeland_state::holeland_get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_bg_tilemap->set_transmask(0, 0xff, 0x00); /* split type 0 is totally transparent in front half */
	m_bg_tilemap->set_transmask(1, 0x01, 0xfe); /* split type 1 has pen 0? transparent in front half */

	save_item(NAME(m_palette_offset));
}

VIDEO_START_MEMBER(holeland_state,crzrally)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(holeland_state::crzrally_get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	save_item(NAME(m_palette_offset));
}

void holeland_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void holeland_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void holeland_state::pal_offs_w(uint8_t data)
{
	if ((m_palette_offset >> 4) != (data & 3))
	{
		m_palette_offset = (data & 3) << 4;
		machine().tilemap().mark_all_dirty();
	}
}

void holeland_state::scroll_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0, data);
}

WRITE_LINE_MEMBER(holeland_state::flipscreen_x_w)
{
	flip_screen_x_set(state);
}

WRITE_LINE_MEMBER(holeland_state::flipscreen_y_w)
{
	flip_screen_y_set(state);
}


void holeland_state::holeland_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t *spriteram = m_spriteram;
	int offs, code, sx, sy, color, flipx, flipy;

	/* Weird, sprites entries don't start on DWORD boundary */
	for (offs = 3; offs < m_spriteram.bytes() - 1; offs += 4)
	{
		sy = 236 - spriteram[offs];
		sx = spriteram[offs + 2];

		/* Bit 7 unknown */
		code = spriteram[offs + 1] & 0x7f;
		color = m_palette_offset + (spriteram[offs + 3] >> 4);

		/* Bit 0, 1 unknown */
		flipx = spriteram[offs + 3] & 0x04;
		flipy = spriteram[offs + 3] & 0x08;

		if (flip_screen_x())
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y())
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				2*sx,2*sy,0);
	}
}

void holeland_state::crzrally_draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	uint8_t *spriteram = m_spriteram;
	int offs, code, sx, sy, color, flipx, flipy;

	/* Weird, sprites entries don't start on DWORD boundary */
	for (offs = 3; offs < m_spriteram.bytes() - 1; offs += 4)
	{
		sy = 236 - spriteram[offs];
		sx = spriteram[offs + 2];

		code = spriteram[offs + 1] + ((spriteram[offs + 3] & 0x01) << 8);
		color = (spriteram[offs + 3] >> 4) + ((spriteram[offs + 3] & 0x01) << 4);

		/* Bit 1 unknown but somehow related to X offset (clipping range?) */
		flipx = spriteram[offs + 3] & 0x04;
		flipy = spriteram[offs + 3] & 0x08;

		if (flip_screen_x())
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y())
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

uint32_t holeland_state::screen_update_holeland(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	holeland_draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}

uint32_t holeland_state::screen_update_crzrally(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	crzrally_draw_sprites(bitmap, cliprect);
	return 0;
}
