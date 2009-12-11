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

#include "driver.h"
#include "includes/yunsung8.h"


/***************************************************************************

                                Memory Handlers

***************************************************************************/

WRITE8_HANDLER( yunsung8_videobank_w )
{
	yunsung8_state *state = (yunsung8_state *)space->machine->driver_data;
	state->videobank = data;
}


READ8_HANDLER( yunsung8_videoram_r )
{
	yunsung8_state *state = (yunsung8_state *)space->machine->driver_data;
	int bank;

	/*  Bit 1 of the bankswitching register contols the c000-c7ff
        area (Palette). Bit 0 controls the c800-dfff area (Tiles) */

	if (offset < 0x0800)
		bank = state->videobank & 2;
	else
		bank = state->videobank & 1;

	if (bank)
		return state->videoram_0[offset];
	else
		return state->videoram_1[offset];
}


WRITE8_HANDLER( yunsung8_videoram_w )
{
	yunsung8_state *state = (yunsung8_state *)space->machine->driver_data;

	if (offset < 0x0800)		// c000-c7ff    Banked Palette RAM
	{
		int bank = state->videobank & 2;
		UINT8 *RAM;
		int color;

		if (bank)
			RAM = state->videoram_0;
		else
			RAM = state->videoram_1;

		RAM[offset] = data;
		color = RAM[offset & ~1] | (RAM[offset | 1] << 8);

		/* BBBBBGGGGGRRRRRx */
		palette_set_color_rgb(space->machine, offset / 2 + (bank ? 0x400 : 0), pal5bit(color >> 0), pal5bit(color >> 5), pal5bit(color >> 10));
	}
	else
	{
		int tile;
		int bank = state->videobank & 1;

		if (offset < 0x1000)
			tile = (offset - 0x0800);		// c800-cfff: Banked Color RAM
		else
			tile = (offset - 0x1000) / 2;	// d000-dfff: Banked Tiles RAM

		if (bank)
		{
			state->videoram_0[offset] = data;
			tilemap_mark_tile_dirty(state->tilemap_0, tile);
		}
		else
		{
			state->videoram_1[offset] = data;
			tilemap_mark_tile_dirty(state->tilemap_1, tile);
		}
	}
}


WRITE8_HANDLER( yunsung8_flipscreen_w )
{
	tilemap_set_flip_all(space->machine, (data & 1) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
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
	yunsung8_state *state = (yunsung8_state *)machine->driver_data;
	int code  =  state->videoram_0[0x1000 + tile_index * 2 + 0] + state->videoram_0[0x1000 + tile_index * 2 + 1] * 256;
	int color =  state->videoram_0[0x0800 + tile_index] & 0x07;
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
	yunsung8_state *state = (yunsung8_state *)machine->driver_data;
	int code  =  state->videoram_1[0x1000 + tile_index * 2 + 0] + state->videoram_1[0x1000 + tile_index * 2 + 1] * 256;
	int color =  state->videoram_1[0x0800 + tile_index] & 0x3f;
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
	yunsung8_state *state = (yunsung8_state *)machine->driver_data;

	state->tilemap_0 = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 8, 8, DIM_NX_0, DIM_NY_0 );
	state->tilemap_1 = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 8, 8, DIM_NX_1, DIM_NY_1 );

	tilemap_set_transparent_pen(state->tilemap_1, 0);
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

VIDEO_UPDATE( yunsung8 )
{
	yunsung8_state *state = (yunsung8_state *)screen->machine->driver_data;
	int layers_ctrl = (~state->layers_ctrl) >> 4;

#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{
	int msk = 0;
	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl & 1)
		tilemap_draw(bitmap, cliprect, state->tilemap_0, 0, 0);
	else
		bitmap_fill(bitmap, cliprect, 0);

	if (layers_ctrl & 2)
		tilemap_draw(bitmap, cliprect, state->tilemap_1, 0, 0);

	return 0;
}
