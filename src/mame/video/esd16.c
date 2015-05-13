// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
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


/***************************************************************************

                                    Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code

    Color code:  layer 0 (backmost) can bank at every 256 colors,
                 layer 1 uses the first 256.

***************************************************************************/

TILE_GET_INFO_MEMBER(esd16_state::get_tile_info_0)
{
	UINT16 code = m_vram_0[tile_index];
	SET_TILE_INFO_MEMBER(1,
			code,
			m_tilemap0_color,
			0);
}

TILE_GET_INFO_MEMBER(esd16_state::get_tile_info_0_16x16)
{
	UINT16 code = m_vram_0[tile_index];
	SET_TILE_INFO_MEMBER(2,
			code,
			m_tilemap0_color,
			0);
}


TILE_GET_INFO_MEMBER(esd16_state::get_tile_info_1)
{
	UINT16 code = m_vram_1[tile_index];
	SET_TILE_INFO_MEMBER(1,
			code,
			m_tilemap1_color,
			0);
}

TILE_GET_INFO_MEMBER(esd16_state::get_tile_info_1_16x16)
{
	UINT16 code = m_vram_1[tile_index];
	SET_TILE_INFO_MEMBER(2,
			code,
			m_tilemap1_color,
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
	m_tilemap0_color = data & 0x03;
	m_tilemap_0->mark_all_dirty();
	m_tilemap_0_16x16->mark_all_dirty();

	flip_screen_set(data & 0x80);
}

WRITE16_MEMBER(esd16_state::esd16_tilemap0_color_jumppop_w)
{
	// todo
	m_tilemap0_color = 2;
	m_tilemap1_color = 1;

	flip_screen_set(data & 0x80);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/


void esd16_state::video_start()
{
	m_tilemap_0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(esd16_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS, 8, 8, 0x80, 0x40);
	m_tilemap_1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(esd16_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS, 8, 8, 0x80, 0x40);

	/* swatpolc changes tilemap 0 to 16x16 at various times */
	m_tilemap_0_16x16 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(esd16_state::get_tile_info_0_16x16),this), TILEMAP_SCAN_ROWS, 16,16, 0x40, 0x40);

	/* hedpanic changes tilemap 1 to 16x16 at various times */
	m_tilemap_1_16x16 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(esd16_state::get_tile_info_1_16x16),this), TILEMAP_SCAN_ROWS, 16,16, 0x40, 0x40);

	m_tilemap_0->set_scrolldx(-0x60 + 2, -0x60);
	m_tilemap_1->set_scrolldx(-0x60, -0x60 + 2);
	m_tilemap_0_16x16->set_scrolldx(-0x60 + 2, -0x60);
	m_tilemap_1_16x16->set_scrolldx(-0x60, -0x60 + 2);

	m_tilemap_1->set_transparent_pen(0x00);
	m_tilemap_1_16x16->set_transparent_pen(0x00);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

UINT32 esd16_state::screen_update_hedpanic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

	screen.priority().fill(0, cliprect);

#ifdef MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 4;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl & 1)
	{
		if (m_head_layersize[0] & 0x0001)
		{
			m_tilemap_0_16x16->set_scrollx(0, m_scroll_0[0]);
			m_tilemap_0_16x16->set_scrolly(0, m_scroll_0[1]);
			m_tilemap_0_16x16->draw(screen, bitmap, cliprect, 0, 0);
		}
		else
		{
			m_tilemap_0->set_scrollx(0, m_scroll_0[0]);
			m_tilemap_0->set_scrolly(0, m_scroll_0[1]);
			m_tilemap_0->draw(screen, bitmap, cliprect, 0, 0);
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}


	if (layers_ctrl & 2)
	{
		if (m_head_layersize[0] & 0x0002)
		{
			m_tilemap_1_16x16->set_scrollx(0, m_scroll_1[0]);
			m_tilemap_1_16x16->set_scrolly(0, m_scroll_1[1]);
			m_tilemap_1_16x16->draw(screen, bitmap, cliprect, 0, 1);
		}
		else
		{
			m_tilemap_1->set_scrollx(0, m_scroll_1[0]);
			m_tilemap_1->set_scrolly(0, m_scroll_1[1]);
			m_tilemap_1->draw(screen, bitmap, cliprect, 0, 1);
		}

	}

	if (layers_ctrl & 4) m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);

//  popmessage("%04x %04x %04x %04x %04x",head_unknown1[0],head_layersize[0],head_unknown3[0],head_unknown4[0],head_unknown5[0]);
	return 0;
}
