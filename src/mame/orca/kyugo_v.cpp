// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    Kyugo hardware games

***************************************************************************/

#include "emu.h"
#include "kyugo.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(kyugo_state::get_fg_tile_info)
{
	int code = m_fgvideoram[tile_index];
	tileinfo.set(0,
					code,
					2 * m_color_codes[code >> 3] + m_fgcolor,
					0);
}


TILE_GET_INFO_MEMBER(kyugo_state::get_bg_tile_info)
{
	int code = m_bgvideoram[tile_index];
	int attr = m_bgattribram[tile_index];
	tileinfo.set(1,
					code | ((attr & 0x03) << 8),
					(attr >> 4) | (m_bgpalbank << 4),
					TILE_FLIPYX((attr & 0x0c) >> 2));
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

void kyugo_state::video_start()
{
	m_color_codes = memregion("proms")->base() + 0x300;

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kyugo_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kyugo_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(-32, 288+32);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void kyugo_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


void kyugo_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void kyugo_state::bgattribram_w(offs_t offset, uint8_t data)
{
	m_bgattribram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


uint8_t kyugo_state::spriteram_2_r(offs_t offset)
{
	// only the lower nibble is connected
	return m_spriteram[1][offset] | 0xf0;
}


void kyugo_state::scroll_x_lo_w(uint8_t data)
{
	m_scroll_x_lo = data;
}


void kyugo_state::gfxctrl_w(uint8_t data)
{
	// bit 0 is scroll MSB
	m_scroll_x_hi = data & 0x01;

	// bit 5 is front layer color (Son of Phoenix only)
	if (m_fgcolor != ((data & 0x20) >> 5))
	{
		m_fgcolor = (data & 0x20) >> 5;

		m_fg_tilemap->mark_all_dirty();
	}

	// bit 6 is background palette bank
	if (m_bgpalbank != ((data & 0x40) >> 6))
	{
		m_bgpalbank = (data & 0x40) >> 6;
		m_bg_tilemap->mark_all_dirty();
	}

	if (data & 0x9e)
		popmessage("%02x",data);
}


void kyugo_state::scroll_y_w(uint8_t data)
{
	m_scroll_y = data;
}


/*************************************
 *
 *  Video update
 *
 *************************************/

void kyugo_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// sprite information is scattered through memory
	// and uses a portion of the text layer memory (outside the visible area)
	uint8_t *spriteram_area1 = &m_spriteram[0][0x28];
	uint8_t *spriteram_area2 = &m_spriteram[1][0x28];
	uint8_t *spriteram_area3 = &m_fgvideoram[0x28];

	int flip = flip_screen();

	for (int n = 0; n < 12 * 2; n++)
	{
		int offs, sy, sx, color;

		offs = 2 * (n % 12) + 64 * (n / 12);

		sx = spriteram_area3[offs + 1] + 256 * (spriteram_area2[offs + 1] & 1);
		if (sx > 320)
			sx -= 512;

		sy = 255 - spriteram_area1[offs] + 2;
		if (sy > 0xf0)
			sy -= 256;

		if (flip)
			sy = 240 - sy;

		color = spriteram_area1[offs + 1] & 0x1f;

		for (int y = 0; y < 16; y++)
		{
			int code, attr, flipx, flipy;

			code = spriteram_area3[offs + 128 * y];
			attr = spriteram_area2[offs + 128 * y];

			code = code | ((attr & 0x01) << 9) | ((attr & 0x02) << 7);

			flipx =  attr & 0x08;
			flipy =  attr & 0x04;

			if (flip)
			{
				flipx = !flipx;
				flipy = !flipy;
			}


			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
						code,
						color,
						flipx,flipy,
						sx,flip ? sy - 16*y : sy + 16*y, 0 );
		}
	}
}


uint32_t kyugo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (flip_screen())
		m_bg_tilemap->set_scrollx(0, -(m_scroll_x_lo + (m_scroll_x_hi * 256)));
	else
		m_bg_tilemap->set_scrollx(0,   m_scroll_x_lo + (m_scroll_x_hi * 256));

	m_bg_tilemap->set_scrolly(0, m_scroll_y);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
