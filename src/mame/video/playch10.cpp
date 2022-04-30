// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
#include "emu.h"
#include "includes/playch10.h"
#include "video/ppu2c0x.h"

#include "screen.h"


void playch10_state::playch10_videoram_w(offs_t offset, uint8_t data)
{
	if (m_pc10_sdcs)
	{
		m_videoram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset / 2);
	}
}

void playch10_state::playch10_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(~color_prom[0], 0);
		bit1 = BIT(~color_prom[0], 1);
		bit2 = BIT(~color_prom[0], 2);
		bit3 = BIT(~color_prom[0], 3);

		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = BIT(~color_prom[256], 0);
		bit1 = BIT(~color_prom[256], 1);
		bit2 = BIT(~color_prom[256], 2);
		bit3 = BIT(~color_prom[256], 3);

		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = BIT(~color_prom[2 * 256], 0);
		bit1 = BIT(~color_prom[2 * 256], 1);
		bit2 = BIT(~color_prom[2 * 256], 2);
		bit3 = BIT(~color_prom[2 * 256], 3);

		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));

		color_prom++;
	}
}

WRITE_LINE_MEMBER(playch10_state::int_detect_w)
{
	if (state)
		m_pc10_int_detect = 1;
}

TILE_GET_INFO_MEMBER(playch10_state::get_bg_tile_info)
{
	uint8_t *videoram = m_videoram;
	int offs = tile_index * 2;
	int code = videoram[offs] + ((videoram[offs + 1] & 0x07) << 8);
	int color = (videoram[offs + 1] >> 3) & 0x1f;

	tileinfo.set(0, code, color, 0);
}

void playch10_state::video_start()
{
	const uint8_t *bios = memregion("maincpu")->base();
	m_pc10_bios = (bios[3] == 0x2a) ? 1 : 2;

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playch10_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}

/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t playch10_state::screen_update_playch10_single(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle top_monitor = screen.visible_area();

	top_monitor.max_y = (top_monitor.max_y - top_monitor.min_y) / 2;

	if (m_pc10_dispmask_old != m_pc10_dispmask)
	{
		m_pc10_dispmask_old = m_pc10_dispmask;

		if (m_pc10_dispmask)
			m_pc10_game_mode ^= 1;
	}

	if (m_pc10_game_mode)
		/* render the ppu */
		m_ppu->render(bitmap, 0, 0, 0, 0, cliprect);
	else
	{
		/* When the bios is accessing vram, the video circuitry can't access it */
		if (!m_pc10_sdcs)
			m_bg_tilemap->draw(screen, bitmap, top_monitor, 0, 0);
	}
	return 0;
}

uint32_t playch10_state::screen_update_playch10_top(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* Single Monitor version */
	if (m_pc10_bios != 1)
		return screen_update_playch10_single(screen, bitmap, cliprect);

	/* When the bios is accessing vram, the video circuitry can't access it */
	if (!m_pc10_sdcs)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(0, cliprect);

	return 0;
}

uint32_t playch10_state::screen_update_playch10_bottom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* Single Monitor version */
	if (m_pc10_bios != 1)
		return screen_update_playch10_single(screen, bitmap, cliprect);

	if (!m_pc10_dispmask)
		/* render the ppu */
		m_ppu->render(bitmap, 0, 0, 0, 0, cliprect);
	else
		bitmap.fill(0, cliprect);

	return 0;
}
