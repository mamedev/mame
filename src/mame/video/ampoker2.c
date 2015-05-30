// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/********************************************************************************


    AMERICAN POKER 2 - VIDEO HARDWARE
    ---------------------------------

    Company:    Novomatic.
    Year:       1990.

    Driver by Roberto Fresca, with a lot of help of Grull Osgo.
    Based on a preliminary work of Curt Coder.


    --- Supported Sets ---

    Set Name | Relation | Description
    ---------+----------+------------------------------------
    ampoker2 |  parent  |  American Poker II.
    ampkr2b1 |  clone   |  American Poker II (bootleg, set 1).
    ampkr2b2 |  clone   |  American Poker II (bootleg, set 2).
    ampkr2b3 |  clone   |  American Poker II (bootleg, set 3).
    ampkr228 |  clone   |  American Poker II (iamp2 v28).
    pkrdewin |  clone   |  Poker De Win.
    ampkr95  |  clone   |  American Poker 95.
    videomat |  clone   |  Videomat (polish bootleg).
    rabbitpk |  clone   |  Rabbit Poker / Arizona Poker 1.1? (with PIC)
    sigmapkr |  parent  |  Sigma Poker.
    sigma2k  |  parent  |  Sigma Poker 2000.


*********************************************************************************


    Resistor Network
    ----------------

    The following diagram is related to taiwanese and argentine PCBs.


    82S147AN
   +---------+
   |         |    470
   | O1-Pin06|---/\/\/\----+---> BLUE
   |         |    220      |
   | O2-Pin07|---/\/\/\----+
   |         |    1K
   | O3-Pin08|---/\/\/\----+---> GREEN
   |         |    470      |
   | O4-Pin09|---/\/\/\----+
   |         |    220      |
   | O5-Pin11|---/\/\/\----+
   |         |    1K
   | O6-Pin12|---/\/\/\----+---> RED
   |         |    470      |
   | O7-Pin13|---/\/\/\----+
   |         |    220      |
   | O8-Pin14|---/\/\/\----+
   |         |
   +---------+


   All colors are directly routed to the edge connector.
   There are not pull-up or pull-down resistors.


********************************************************************************/


#include "emu.h"
#include "video/resnet.h"
#include "includes/ampoker2.h"


PALETTE_INIT_MEMBER(ampoker2_state, ampoker2)
{
	const UINT8 *color_prom = memregion("proms")->base();
/*    - bits -
      76543210
      RRRGGGBB
*/
	int i;
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[2];

	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  0,  0,
			3,  resistances_rg, weights_g,  0,  0,
			2,  resistances_b,  weights_b,  0,  0);


	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* blue component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);
		/* green component */
		bit0 = (color_prom[i] >> 2) & 0x01;
		bit1 = (color_prom[i] >> 3) & 0x01;
		bit2 = (color_prom[i] >> 4) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);
		/* red component */
		bit0 = (color_prom[i] >> 5) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}

WRITE8_MEMBER(ampoker2_state::ampoker2_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(ampoker2_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int offs = tile_index * 2;
	int attr = videoram[offs + 1];
	int code = videoram[offs];
	int color = attr;
	code = code + (256 * (color & 0x03));   /* code = color.bit1 + color.bit0 + code */
	color = color >> 1;                     /* color = color - bit0 (bit1..bit7) */

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(ampoker2_state::s2k_get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int offs = tile_index * 2;
	int attr = videoram[offs + 1];
	int code = videoram[offs];
	int color = attr;
	code = code + (256 * (color & 0x0f));   /* the game uses 2 extra bits */
	color = color >> 1;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void ampoker2_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ampoker2_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 64, 32);
}

VIDEO_START_MEMBER(ampoker2_state,sigma2k)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ampoker2_state::s2k_get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 64, 32);
}

UINT32 ampoker2_state::screen_update_ampoker2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
