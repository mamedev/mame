/**********************************************************************************


    SNOOKER 10 / SANDII'

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Snooker 10 (Ver 1.11), Sandii', 1998.
    * Apple 10 (Ver 1.21),   Sandii', 1998.
    * Ten Balls (Ver 1.05),  unknown, 1997.


***********************************************************************************


    Resistor Network for all PCBs:


     74LS373
    +-------+
    |     02|--> 1  KOhms resistor --> \
    |     05|--> 470 Ohms resistor -->  > 100 Ohms pull-down resistor --> RED
    |     06|--> 220 Ohms resistor --> /
    |       |
    |     09|--> 1  KOhms resistor --> \
    |     12|--> 470 Ohms resistor -->  > 100 Ohms pull-down resistor --> BLUE
    |     15|--> 220 Ohms resistor --> /
    |       |
    |     16|--> 470 Ohms resistor --> \  100 Ohms pull-down resistor --> GREEN
    |     19|--> 220 Ohms resistor --> /
    +-------+


**********************************************************************************/


#include "emu.h"
#include "video/resnet.h"
#include "includes/snookr10.h"


WRITE8_MEMBER(snookr10_state::snookr10_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(snookr10_state::snookr10_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


PALETTE_INIT( snookr10 )
{
	/* GGBBBRRR */

	int i;
	static const int resistances_rb[3] = { 1000, 470, 220 };
	static const int resistances_g [2] = { 470, 220 };
	double weights_r[3], weights_b[3], weights_g[2];

	compute_resistor_weights(0,	255,	-1.0,
			3,	resistances_rb,	weights_r,	100,	0,
			3,	resistances_rb,	weights_b,	100,	0,
			2,	resistances_g,	weights_g,	100,	0);


	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);
		/* blue component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		b = combine_3_weights(weights_b, bit0, bit1, bit2);
		/* green component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		g = combine_2_weights(weights_g, bit0, bit1);

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	snookr10_state *state = machine.driver_data<snookr10_state>();
/*  - bits -
    7654 3210
    xxxx ----   tiles color.
    ---- xxxx   seems unused.
*/
	int offs = tile_index;
	int attr = state->m_videoram[offs] + (state->m_colorram[offs] << 8);
	int code = attr & 0xfff;
	int color = state->m_colorram[offs] >> 4;

	SET_TILE_INFO(0, code, color, 0);
}


/**********************************************************
* Apple10 colors and tile matrix are encrypted/scrambled. *
*     For more information, see the driver notes.         *
**********************************************************/

PALETTE_INIT( apple10 )
{
	/* GGBBBRRR */

	int i, cn;
	static const int resistances_rb[3] = { 1000, 470, 220 };
	static const int resistances_g [2] = { 470, 220 };
	double weights_r[3], weights_b[3], weights_g[2];

	compute_resistor_weights(0,	255,	-1.0,
			3,	resistances_rb,	weights_r,	100,	0,
			3,	resistances_rb,	weights_b,	100,	0,
			2,	resistances_g,	weights_g,	100,	0);


	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);
		/* blue component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		b = combine_3_weights(weights_b, bit0, bit1, bit2);
		/* green component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		g = combine_2_weights(weights_g, bit0, bit1);

		/* encrypted color matrix */
		cn = BITSWAP8(i,4,5,6,7,2,3,0,1);

		palette_set_color(machine, cn, MAKE_RGB(r,g,b));
	}
}

static TILE_GET_INFO( apple10_get_bg_tile_info )
{
	snookr10_state *state = machine.driver_data<snookr10_state>();
/*  - bits -
    7654 3210
    xxxx ----   tiles color.
    ---- xxxx   seems unused.
*/
	int offs = tile_index;
	int attr = state->m_videoram[offs] + (state->m_colorram[offs] << 8);
	int code = BITSWAP16((attr & 0xfff),15,14,13,12,8,9,10,11,0,1,2,3,4,5,6,7);	/* encrypted tile matrix */
	int color = state->m_colorram[offs] >> 4;

	SET_TILE_INFO(0, code, color, 0);
}


VIDEO_START( snookr10 )
{
	snookr10_state *state = machine.driver_data<snookr10_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 4, 8, 128, 30);
}

VIDEO_START( apple10 )
{
	snookr10_state *state = machine.driver_data<snookr10_state>();
	state->m_bg_tilemap = tilemap_create(machine, apple10_get_bg_tile_info, tilemap_scan_rows, 4, 8, 128, 30);
}

SCREEN_UPDATE_IND16( snookr10 )
{
	snookr10_state *state = screen.machine().driver_data<snookr10_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
