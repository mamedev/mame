// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/***********************************************

    +-------------------------------------+
    |                                     |
    | CAL OMEGA - SYSTEMS 903/904/905/906 |
    |                                     |
    |      Driver by Roberto Fresca.      |
    |                                     |
    +-------------------------------------+

             * Video Hardware *

************************************************/


#include "emu.h"
#include "includes/calomega.h"


WRITE8_MEMBER(calomega_state::calomega_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(calomega_state::calomega_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(calomega_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    --xx xx--   tiles color.
    ---- --x-   tiles bank.
    xx-- ---x   seems unused. */

	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index];
	int bank = (attr & 0x02) >> 1;  /* bit 1 switch the gfx banks */
	int color = (attr & 0x3c) >> 2;  /* bits 2-3-4-5 for color */

	SET_TILE_INFO_MEMBER(bank, code, color, 0);
}

void calomega_state::video_start()
{
	m_gfxdecode->gfx(0)->set_granularity(8);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(calomega_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 31);
}

UINT32 calomega_state::screen_update_calomega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

PALETTE_INIT_MEMBER(calomega_state, calomega)
{
	const UINT8 *color_prom = memregion("proms")->base();

/*  the proms are 256x4 bit, but the games only seem to need the first 128 entries,
    and the rest of the PROM data looks like junk rather than valid colors

    prom bits
    3210
    ---x   red component
    --x-   green component
    -x--   blue component
    x---   foreground (colors with this bit set are full brightness,
           colors with it clear are attenuated by the background color pots)
*/

	// TODO: hook pots up as PORT_ADJUSTERs instead of hard coding them here

	// let's make the BG a little darker than FG blue
	const int r_pot = 0x00;
	const int g_pot = 0x00;
	const int b_pot = 0xc0;

	/* 00000BGR */
	if (color_prom == nullptr) return;

	for (int i = 0;i < palette.entries();i++)
	{
		int nibble = color_prom[i];

		int fg = BIT(nibble, 3);

		/* red component */
		int r = BIT(nibble, 0) * (fg ? 0xff : r_pot);

		/* green component */
		int g = BIT(nibble, 1) * (fg ? 0xff : g_pot);

		/* blue component */
		int b = BIT(nibble, 2) * (fg ? 0xff : b_pot);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}
