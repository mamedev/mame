// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "emu.h"
#include "includes/pcktgal.h"

void pcktgal_state::pcktgal_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 5);
		bit2 = BIT(color_prom[i], 6);
		bit3 = BIT(color_prom[i], 7);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = BIT(color_prom[i + palette.entries()], 0);
		bit1 = BIT(color_prom[i + palette.entries()], 1);
		bit2 = BIT(color_prom[i + palette.entries()], 2);
		bit3 = BIT(color_prom[i + palette.entries()], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void pcktgal_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bool flip_screen)
{
	for (int offs = 0;offs < m_spriteram.bytes();offs += 4)
	{
		if (m_spriteram[offs] != 0xf8)
		{
			int sx,sy,flipx,flipy;


			sx = 240 - m_spriteram[offs+2];
			sy = 240 - m_spriteram[offs];

			flipx = m_spriteram[offs+1] & 0x04;
			flipy = m_spriteram[offs+1] & 0x02;
			if (flip_screen) {
				sx=240-sx;
				sy=240-sy;
				if (flipx) flipx=0; else flipx=1;
				if (flipy) flipy=0; else flipy=1;
			}

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					m_spriteram[offs+3] + ((m_spriteram[offs+1] & 1) << 8),
					(m_spriteram[offs+1] & 0x70) >> 4,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

uint32_t pcktgal_state::screen_update_pcktgal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen->get_flip_state();
	m_tilegen->set_flip_screen(flip);
	m_tilegen->deco_bac06_pf_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	draw_sprites(bitmap, cliprect, flip);
	return 0;
}

uint32_t pcktgal_state::screen_update_pcktgalb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen->get_flip_state();
	m_tilegen->set_flip_screen(flip);
	// the bootleg doesn't properly set the tilemap registers, because it's on non-original hardware, which probably doesn't have the flexible tilemaps.
	m_tilegen->deco_bac06_pf_draw_bootleg(bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0, 2);
	draw_sprites(bitmap, cliprect, flip);
	return 0;
}
