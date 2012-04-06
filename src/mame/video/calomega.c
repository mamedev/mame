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

static TILE_GET_INFO( get_bg_tile_info )
{
	calomega_state *state = machine.driver_data<calomega_state>();
/*  - bits -
    7654 3210
    --xx xx--   tiles color.
    ---- --x-   tiles bank.
    xx-- ---x   seems unused. */

	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index];
	int bank = (attr & 0x02) >> 1;	/* bit 1 switch the gfx banks */
	int color = (attr & 0x3c);	/* bits 2-3-4-5 for color */

	if (attr == 0x3a)	/* Is the palette wrong? */
		color = 0x3b;	/* 0x3b is the best match */

	if (attr == 0x36)	/* Is the palette wrong? */
		color = 0x3a;	/* 0x3a is the best match */

	if (attr == 0x32)	/* Is the palette wrong? */
		color = 0x39;	/* 0x39 is the best match */

	SET_TILE_INFO(bank, code, color, 0);
}

VIDEO_START( calomega )
{
	calomega_state *state = machine.driver_data<calomega_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 31);
}

SCREEN_UPDATE_IND16( calomega )
{
	calomega_state *state = screen.machine().driver_data<calomega_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

PALETTE_INIT( calomega )
{
/*  prom bits
    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    xxxx x---   unused.
*/
	int i;

	/* 00000BGR */
	if (color_prom == 0) return;

	for (i = 0;i < machine.total_colors();i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		r = bit0 * 0xff;

		/* green component */
		bit1 = (color_prom[i] >> 1) & 0x01;
		g = bit1 * 0xff;

		/* blue component */
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = bit2 * 0xff;


		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}
