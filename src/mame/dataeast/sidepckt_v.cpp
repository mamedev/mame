// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/******************************************************************************

    Data East Side Pocket hardware

    Functions to emulate the video hardware

******************************************************************************/

#include "emu.h"
#include "sidepckt.h"


void sidepckt_state::sidepckt_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 5);
		bit2 = BIT(color_prom[i], 6);
		bit3 = BIT(color_prom[i], 7);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = BIT(color_prom[i + palette.entries()], 0);
		bit1 = BIT(color_prom[i + palette.entries()], 1);
		bit2 = BIT(color_prom[i + palette.entries()], 2);
		bit3 = BIT(color_prom[i + palette.entries()], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(sidepckt_state::get_tile_info)
{
	uint8_t attr = m_colorram[tile_index];
	tileinfo.set(0,
			m_videoram[tile_index] + ((attr & 0x07) << 8),
			((attr & 0x10) >> 3) | ((attr & 0x20) >> 5),
			TILE_FLIPX);
	tileinfo.group = (attr & 0x80) >> 7;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void sidepckt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sidepckt_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	m_bg_tilemap->set_transmask(0,0xff,0x00); /* split type 0 is totally transparent in front half */
	m_bg_tilemap->set_transmask(1,0x01,0xfe); /* split type 1 has pen 0 transparent in front half */

	machine().tilemap().set_flip_all(TILEMAP_FLIPX);

	save_item(NAME(m_scroll_y));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void sidepckt_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void sidepckt_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint8_t sidepckt_state::scroll_y_r()
{
	return (m_scroll_y);
}

void sidepckt_state::scroll_y_w(uint8_t data)
{
	// Bits 0-5: Scroll y
	m_scroll_y = data & 0x3F;

	// Other bits: Unknown, but they seem never written
	if (data > 0x3F)
		logerror ("scroll_y_w: Unknown write -> data = 0x%02X\n", data);
}


/***************************************************************************

  Display refresh

***************************************************************************/

void sidepckt_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = 0;offs < m_spriteram.bytes(); offs += 4)
	{
		int attr  = m_spriteram[offs | 1];
		int code  = ((attr & 0x03) << 8) | m_spriteram[offs | 3];
		int color = (attr & 0xf0) >> 4;

		int sx = m_spriteram[offs | 2] - 2;
		int sy = m_spriteram[offs];

		int flipx = attr & 0x08;
		int flipy = attr & 0x04;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,0);

		/* wraparound */
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx-256,sy,0);
	}
}


uint32_t sidepckt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrolly (0, m_scroll_y);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	draw_sprites(bitmap,cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	return 0;
}
