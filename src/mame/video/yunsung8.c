/***************************************************************************

                          -= Yun Sung 8 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows the background layer
        W       shows the foreground layer

        [ 2 Fixed Layers ]

            [ Background ]

            Layer Size:             512 x 256
            Tiles:                  8 x 8 x 8

            [ Foreground ]

            Layer Size:             512 x 256
            Tiles:                  8 x 8 x 4


        There are no sprites.

***************************************************************************/

#include "emu.h"
#include "includes/yunsung8.h"


/***************************************************************************

                                Memory Handlers

***************************************************************************/

WRITE8_MEMBER(yunsung8_state::yunsung8_videobank_w)
{
	m_videobank = data;
}


READ8_MEMBER(yunsung8_state::yunsung8_videoram_r)
{
	int bank;

	/*  Bit 1 of the bankswitching register contols the c000-c7ff
        area (Palette). Bit 0 controls the c800-dfff area (Tiles) */

	if (offset < 0x0800)
		bank = m_videobank & 2;
	else
		bank = m_videobank & 1;

	if (bank)
		return m_videoram_0[offset];
	else
		return m_videoram_1[offset];
}


WRITE8_MEMBER(yunsung8_state::yunsung8_videoram_w)
{

	if (offset < 0x0800)		// c000-c7ff    Banked Palette RAM
	{
		int bank = m_videobank & 2;
		UINT8 *RAM;
		int color;

		if (bank)
			RAM = m_videoram_0;
		else
			RAM = m_videoram_1;

		RAM[offset] = data;
		color = RAM[offset & ~1] | (RAM[offset | 1] << 8);

		/* BBBBBGGGGGRRRRRx */
		palette_set_color_rgb(machine(), offset / 2 + (bank ? 0x400 : 0), pal5bit(color >> 0), pal5bit(color >> 5), pal5bit(color >> 10));
	}
	else
	{
		int tile;
		int bank = m_videobank & 1;

		if (offset < 0x1000)
			tile = (offset - 0x0800);		// c800-cfff: Banked Color RAM
		else
			tile = (offset - 0x1000) / 2;	// d000-dfff: Banked Tiles RAM

		if (bank)
		{
			m_videoram_0[offset] = data;
			m_tilemap_0->mark_tile_dirty(tile);
		}
		else
		{
			m_videoram_1[offset] = data;
			m_tilemap_1->mark_tile_dirty(tile);
		}
	}
}


WRITE8_MEMBER(yunsung8_state::yunsung8_flipscreen_w)
{
	machine().tilemap().set_flip_all((data & 1) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
}


/***************************************************************************

                              [ Tiles Format ]

    Offset:

    Video RAM + 0000.b      Code (Low  Bits)
    Video RAM + 0001.b      Code (High Bits)

    Color RAM + 0000.b      Color


***************************************************************************/

/* Background */

#define DIM_NX_0			(0x40)
#define DIM_NY_0			(0x20)

static TILE_GET_INFO( get_tile_info_0 )
{
	yunsung8_state *state = machine.driver_data<yunsung8_state>();
	int code  =  state->m_videoram_0[0x1000 + tile_index * 2 + 0] + state->m_videoram_0[0x1000 + tile_index * 2 + 1] * 256;
	int color =  state->m_videoram_0[0x0800 + tile_index] & 0x07;
	SET_TILE_INFO(
			0,
			code,
			color,
			0);
}

/* Text Plane */

#define DIM_NX_1			(0x40)
#define DIM_NY_1			(0x20)

static TILE_GET_INFO( get_tile_info_1 )
{
	yunsung8_state *state = machine.driver_data<yunsung8_state>();
	int code  =  state->m_videoram_1[0x1000 + tile_index * 2 + 0] + state->m_videoram_1[0x1000 + tile_index * 2 + 1] * 256;
	int color =  state->m_videoram_1[0x0800 + tile_index] & 0x3f;
	SET_TILE_INFO(
			1,
			code,
			color,
			0);
}




/***************************************************************************


                            Vide Hardware Init


***************************************************************************/

VIDEO_START( yunsung8 )
{
	yunsung8_state *state = machine.driver_data<yunsung8_state>();

	state->m_tilemap_0 = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 8, 8, DIM_NX_0, DIM_NY_0 );
	state->m_tilemap_1 = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 8, 8, DIM_NX_1, DIM_NY_1 );

	state->m_tilemap_1->set_transparent_pen(0);
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

SCREEN_UPDATE_IND16( yunsung8 )
{
	yunsung8_state *state = screen.machine().driver_data<yunsung8_state>();
	int layers_ctrl = (~state->m_layers_ctrl) >> 4;

#ifdef MAME_DEBUG
if (screen.machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (screen.machine().input().code_pressed(KEYCODE_Q))	msk |= 1;
	if (screen.machine().input().code_pressed(KEYCODE_W))	msk |= 2;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl & 1)
		state->m_tilemap_0->draw(bitmap, cliprect, 0, 0);
	else
		bitmap.fill(0, cliprect);

	if (layers_ctrl & 2)
		state->m_tilemap_1->draw(bitmap, cliprect, 0, 0);

	return 0;
}
