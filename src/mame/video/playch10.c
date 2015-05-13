// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
#include "emu.h"
#include "video/ppu2c0x.h"
#include "includes/playch10.h"



WRITE8_MEMBER(playch10_state::playch10_videoram_w)
{
	UINT8 *videoram = m_videoram;
	if (m_pc10_sdcs)
	{
		videoram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset / 2);
	}
}

PALETTE_INIT_MEMBER(playch10_state, playch10)
{
	const UINT8 *color_prom = memregion("proms")->base();

	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		/* red component */

		bit0 = ~(color_prom[0] >> 0) & 0x01;
		bit1 = ~(color_prom[0] >> 1) & 0x01;
		bit2 = ~(color_prom[0] >> 2) & 0x01;
		bit3 = ~(color_prom[0] >> 3) & 0x01;

		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = ~(color_prom[256] >> 0) & 0x01;
		bit1 = ~(color_prom[256] >> 1) & 0x01;
		bit2 = ~(color_prom[256] >> 2) & 0x01;
		bit3 = ~(color_prom[256] >> 3) & 0x01;

		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */

		bit0 = ~(color_prom[2*256] >> 0) & 0x01;
		bit1 = ~(color_prom[2*256] >> 1) & 0x01;
		bit2 = ~(color_prom[2*256] >> 2) & 0x01;
		bit3 = ~(color_prom[2*256] >> 3) & 0x01;

		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i,rgb_t(r,g,b));

		color_prom++;
	}

	m_ppu->init_palette_rgb(palette, 256);
}

void playch10_state::ppu_irq(int *ppu_regs)
{
	machine().device("cart")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE );
	m_pc10_int_detect = 1;
}

TILE_GET_INFO_MEMBER(playch10_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int offs = tile_index * 2;
	int code = videoram[offs] + ((videoram[offs + 1] & 0x07) << 8);
	int color = (videoram[offs + 1] >> 3) & 0x1f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void playch10_state::video_start()
{
	const UINT8 *bios = memregion("maincpu")->base();
	m_pc10_bios = (bios[3] == 0x2a) ? 1 : 2;

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playch10_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}

VIDEO_START_MEMBER(playch10_state,playch10_hboard)
{
	const UINT8 *bios = memregion("maincpu")->base();
	m_pc10_bios = (bios[3] == 0x2a) ? 1 : 2;

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playch10_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}

/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 playch10_state::screen_update_playch10_single(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
		m_ppu->render(bitmap, 0, 0, 0, 0);
	else
	{
		/* When the bios is accessing vram, the video circuitry can't access it */
		if (!m_pc10_sdcs)
			m_bg_tilemap->draw(screen, bitmap, top_monitor, 0, 0);
	}
	return 0;
}

UINT32 playch10_state::screen_update_playch10_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Single Monitor version */
	if (m_pc10_bios != 1)
		return screen_update_playch10_single(screen, bitmap, cliprect);

	if (!m_pc10_dispmask)
		/* render the ppu */
		m_ppu->render(bitmap, 0, 0, 0, 0);
	else
		bitmap.fill(0, cliprect);

	return 0;
}

UINT32 playch10_state::screen_update_playch10_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
