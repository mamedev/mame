/****************************************************************************************

    LUCKY 74 - WING CO.,LTD.
    ------------------------

    *** VIDEO HARDWARE ***


    Written by Roberto Fresca.


    Games running on this hardware:

    * Lucky 74 (bootleg, set 1), 1988, Wing Co.,Ltd.
    * Lucky 74 (bootleg, set 2), 1988, Wing Co.,Ltd.


*****************************************************************************************


    Color Circuitry
    ---------------

    Here is a little diagram showing how the color PROMs are connected to a 74174
    and therefore to a resistor network that derives to the RGB connector.


                                  220
    (E6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (E7)24s10-12 _|                        |
                                  470      |
    (E6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (E7)24s10-11 _|                        |
                                   1K      |
    (E6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (E7)24s10-10 _|                        |
                                   2K      |
    (E6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Red
    (E7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _

                                  220
    (D6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (D7)24s10-12 _|                        |
                                  470      |
    (D6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (D7)24s10-11 _|                        |
                                   1K      |
    (D6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (D7)24s10-10 _|                        |
                                   2K      |
    (D6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Green
    (D7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _

                                  220
    (C6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (C7)24s10-12 _|                        |
                                  470      |
    (C6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (C7)24s10-11 _|                        |
                                   1K      |
    (C6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (C7)24s10-10 _|                        |
                                   2K      |
    (C6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Blue
    (C7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _


    Regarding the abobe diagram, there are 2 different states controlled by both 06B53P.
    Each state arrange a different palette that will be assigned to each graphics bank.

    As we can see here, same pin of different PROMs are connected together in parallel.

    To reproduce the states, we need to create a double-sized palette and fill the first
    half with the values created through state 1, then fill the second half with proper
    values from state 2.


****************************************************************************************/


#include "emu.h"
#include "video/resnet.h"
#include "includes/lucky74.h"


WRITE8_MEMBER(lucky74_state::lucky74_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(lucky74_state::lucky74_fg_colorram_w)
{
	m_fg_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(lucky74_state::lucky74_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(lucky74_state::lucky74_bg_colorram_w)
{
	m_bg_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


PALETTE_INIT( lucky74 )
/*
   There are 2 states (see the technical notes).
   We're constructing a double-sized palette with one half for each state.
*/
{
	int i;
	static const int resistances_rgb[4] = { 2000, 1000, 470, 220 };
	double weights_r[4], weights_g[4], weights_b[4];

	compute_resistor_weights(0,	255,	-1.0,
			4,	resistances_rgb,	weights_r,	1000,	0,
			4,	resistances_rgb,	weights_g,	1000,	0,
			4,	resistances_rgb,	weights_b,	1000,	0);


	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3, r1, g1, b1, r2, g2, b2;

        /* red component (state 1, PROM E6) */
		bit0 = (color_prom[0x000 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x000 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x000 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x000 + i] >> 3) & 0x01;
		r1 = combine_4_weights(weights_r, bit0, bit1, bit2, bit3);

        /* red component (state 2, PROM E7) */
		bit0 = (color_prom[0x100 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x100 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x100 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x100 + i] >> 3) & 0x01;
		r2 = combine_4_weights(weights_r, bit0, bit1, bit2, bit3);

        /* green component (state 1, PROM D6) */
		bit0 = (color_prom[0x200 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x200 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x200 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x200 + i] >> 3) & 0x01;
		g1 = combine_4_weights(weights_g, bit0, bit1, bit2, bit3);

        /* green component (state 2, PROM D7) */
		bit0 = (color_prom[0x300 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x300 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x300 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x300 + i] >> 3) & 0x01;
		g2 = combine_4_weights(weights_g, bit0, bit1, bit2, bit3);

        /* blue component (state 1, PROM C6) */
		bit0 = (color_prom[0x400 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x400 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x400 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x400 + i] >> 3) & 0x01;
		b1 = combine_4_weights(weights_b, bit0, bit1, bit2, bit3);

        /* blue component (state 2, PROM C7) */
		bit0 = (color_prom[0x500 + i] >> 0) & 0x01;
		bit1 = (color_prom[0x500 + i] >> 1) & 0x01;
		bit2 = (color_prom[0x500 + i] >> 2) & 0x01;
		bit3 = (color_prom[0x500 + i] >> 3) & 0x01;
		b2 = combine_4_weights(weights_b, bit0, bit1, bit2, bit3);


        /* PROMs circuitry, 1st state */
		palette_set_color(machine, i, MAKE_RGB(r1, g1, b1));

        /* PROMs circuitry, 2nd state */
		palette_set_color(machine, i + 256, MAKE_RGB(r2, g2, b2));
	}
}


static TILE_GET_INFO( get_fg_tile_info )
{
	lucky74_state *state = machine.driver_data<lucky74_state>();
/*  - bits -
    7654 3210
    ---- xxxx   tiles color.
    xxxx ----   tiles page offset.
*/
	int bank = 0;
	int attr = state->m_fg_colorram[tile_index];
	int code = state->m_fg_videoram[tile_index] + ((attr & 0xf0) << 4);
	int color = (attr & 0x0f);

	SET_TILE_INFO(bank, code, color, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	lucky74_state *state = machine.driver_data<lucky74_state>();
/*  - bits -
    7654 3210
    ---- xxxx   tiles color.
    xxxx ----   tiles page offset.
*/
	int bank = 1;
	int attr = state->m_bg_colorram[tile_index];
	int code = state->m_bg_videoram[tile_index] + ((attr & 0xf0) << 4);
	int color = (attr & 0x0f);

	SET_TILE_INFO(bank, code, color, 0);
}


VIDEO_START( lucky74 )
{
	lucky74_state *state = machine.driver_data<lucky74_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_fg_tilemap->set_transparent_pen(0);
}

SCREEN_UPDATE_IND16( lucky74 )
{
	lucky74_state *state = screen.machine().driver_data<lucky74_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}


