// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "includes/xyonix.h"

void xyonix_state::xyonix_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 5);
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}


TILE_GET_INFO_MEMBER(xyonix_state::get_tile_info)
{
	int tileno;
	int attr = m_vidram[tile_index+0x1000+1];

	tileno = (m_vidram[tile_index+1] << 0) | ((attr & 0x0f) << 8);

	SET_TILE_INFO_MEMBER(0,tileno,attr >> 4,0);
}

WRITE8_MEMBER(xyonix_state::vidram_w)
{
	m_vidram[offset] = data;
	m_tilemap->mark_tile_dirty((offset-1)&0x0fff);
}

void xyonix_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xyonix_state::get_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 80, 32);
}

uint32_t xyonix_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
