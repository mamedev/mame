/***************************************************************************

                          -= ESD 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q / W           Shows Layer 0 / 1
        A               Shows Sprites

        Keys can be used together!


    [ 2 Scrolling Layers ]

        Tile Size:              8 x 8 x 8
        Color Codes:            1 per Layer (banked for Layer 0)
        Layer Size (tiles) :    128 x 64
        Layer Size (pixels):    1024 x 512

    [ 256 Sprites ]

        Sprites are made of 16 x 16 x 5 tiles. Size can vary from 1 to
        8 tiles vertically, while their width is always 1 tile.

    [ Priorities ]

        The game only uses this scheme:

        Back -> Front:  Layer 0, Layer 1, Sprites

***************************************************************************/

#include "emu.h"
#include "includes/esd16.h"
#include "video/decospr.h"

/***************************************************************************

                                    Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code

    Color code:  layer 0 (backmost) can bank at every 256 colors,
                 layer 1 uses the first 256.

***************************************************************************/

static TILE_GET_INFO( get_tile_info_0 )
{
	esd16_state *state = machine.driver_data<esd16_state>();
	UINT16 code = state->m_vram_0[tile_index];
	SET_TILE_INFO(
			1,
			code,
			state->m_tilemap0_color,
			0);
}

static TILE_GET_INFO( get_tile_info_0_16x16 )
{
	esd16_state *state = machine.driver_data<esd16_state>();
	UINT16 code = state->m_vram_0[tile_index];
	SET_TILE_INFO(
			2,
			code,
			state->m_tilemap0_color,
			0);
}


static TILE_GET_INFO( get_tile_info_1 )
{
	esd16_state *state = machine.driver_data<esd16_state>();
	UINT16 code = state->m_vram_1[tile_index];
	SET_TILE_INFO(
			1,
			code,
			0,
			0);
}

static TILE_GET_INFO( get_tile_info_1_16x16 )
{
	esd16_state *state = machine.driver_data<esd16_state>();
	UINT16 code = state->m_vram_1[tile_index];
	SET_TILE_INFO(
			2,
			code,
			0,
			0);
}

WRITE16_MEMBER(esd16_state::esd16_vram_0_w)
{
	COMBINE_DATA(&m_vram_0[offset]);
	m_tilemap_0->mark_tile_dirty(offset);
	m_tilemap_0_16x16->mark_tile_dirty(offset);
}

WRITE16_MEMBER(esd16_state::esd16_vram_1_w)
{
	COMBINE_DATA(&m_vram_1[offset]);
	m_tilemap_1->mark_tile_dirty(offset);
	m_tilemap_1_16x16->mark_tile_dirty(offset);
}

WRITE16_MEMBER(esd16_state::esd16_tilemap0_color_w)
{
	m_tilemap0_color = data & 3;
	m_tilemap_0->mark_all_dirty();

	flip_screen_set(machine(), data & 0x80);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/


VIDEO_START( esd16 )
{
	esd16_state *state = machine.driver_data<esd16_state>();

	state->m_tilemap_0 = tilemap_create(	machine, get_tile_info_0, tilemap_scan_rows, 8, 8, 0x80, 0x40);
	state->m_tilemap_1 = tilemap_create(	machine, get_tile_info_1, tilemap_scan_rows, 8, 8, 0x80, 0x40);

	/* swatpolc changes tilemap 0 to 16x16 at various times */
	state->m_tilemap_0_16x16 = tilemap_create(machine, get_tile_info_0_16x16, tilemap_scan_rows, 16,16, 0x40, 0x40);

	/* hedpanic changes tilemap 1 to 16x16 at various times */
	state->m_tilemap_1_16x16 = tilemap_create(machine, get_tile_info_1_16x16, tilemap_scan_rows, 16,16, 0x40, 0x40);

	state->m_tilemap_0->set_scrolldx(-0x60 + 2, -0x60);
	state->m_tilemap_1->set_scrolldx(-0x60, -0x60 + 2);
	state->m_tilemap_0_16x16->set_scrolldx(-0x60 + 2, -0x60);
	state->m_tilemap_1_16x16->set_scrolldx(-0x60, -0x60 + 2);

	state->m_tilemap_1->set_transparent_pen(0x00);
	state->m_tilemap_1_16x16->set_transparent_pen(0x00);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

SCREEN_UPDATE_IND16( hedpanic )
{
	esd16_state *state = screen.machine().driver_data<esd16_state>();
	int layers_ctrl = -1;

	screen.machine().priority_bitmap.fill(0, cliprect);

#ifdef MAME_DEBUG
if (screen.machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (screen.machine().input().code_pressed(KEYCODE_Q))	msk |= 1;
	if (screen.machine().input().code_pressed(KEYCODE_W))	msk |= 2;
	if (screen.machine().input().code_pressed(KEYCODE_A))	msk |= 4;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl & 1)
	{
		if (state->m_head_layersize[0] & 0x0001)
		{
			state->m_tilemap_0_16x16->set_scrollx(0, state->m_scroll_0[0]);
			state->m_tilemap_0_16x16->set_scrolly(0, state->m_scroll_0[1]);
			state->m_tilemap_0_16x16->draw(bitmap, cliprect, 0, 0);
		}
		else
		{
			state->m_tilemap_0->set_scrollx(0, state->m_scroll_0[0]);
			state->m_tilemap_0->set_scrolly(0, state->m_scroll_0[1]);
			state->m_tilemap_0->draw(bitmap, cliprect, 0, 0);
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}


	if (layers_ctrl & 2)
	{
		if (state->m_head_layersize[0] & 0x0002)
		{
			state->m_tilemap_1_16x16->set_scrollx(0, state->m_scroll_1[0]);
			state->m_tilemap_1_16x16->set_scrolly(0, state->m_scroll_1[1]);
			state->m_tilemap_1_16x16->draw(bitmap, cliprect, 0, 1);
		}
		else
		{
			state->m_tilemap_1->set_scrollx(0, state->m_scroll_1[0]);
			state->m_tilemap_1->set_scrolly(0, state->m_scroll_1[1]);
			state->m_tilemap_1->draw(bitmap, cliprect, 0, 1);
		}

	}

	if (layers_ctrl & 4) screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, 0x400);

//  popmessage("%04x %04x %04x %04x %04x",head_unknown1[0],head_layersize[0],head_unknown3[0],head_unknown4[0],head_unknown5[0]);
	return 0;
}
