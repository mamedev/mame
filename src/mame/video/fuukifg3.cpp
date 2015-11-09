// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood, Luca Elia
/***************************************************************************

                          -= Fuuki 32 Bit Games (FG-3) =-

                driver by Paul Priest and David Haywood
                based on fuukifg2 by Luca Elia


    [ 4 Scrolling Layers ]

                            [ Layer 0 ]     [ Layer 1 ]     [ Layers 2&3 (double-buffered) ]

    Tile Size:              16 x 16 x 8     16 x 16 x 8     8 x 8 x 4
    Layer Size (tiles):     64 x 32         64 x 32         64 x 32

    [ 1024? Zooming Sprites ]

    Sprites are made of 16 x 16 x 4 tiles. Size can vary from 1 to 16
    tiles both horizontally and vertically.
    There is zooming (from full size to half size) and 4 levels of
    priority (wrt layers)

    Per-line raster effects used on many stages
    Sprites buffered by two frames
    Tilebank buffered by 3 frames? Only 2 in attract
    Sprite pens needs to be buffered by 3 frames? Or lazy programming? Probably 2

***************************************************************************/

#include "emu.h"
#include "includes/fuukifg3.h"


/***************************************************************************


                                    Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code

        2.w     fedc ba98 ---- ----
                ---- ---- 7--- ----     Flip Y
                ---- ---- -6-- ----     Flip X
                ---- ---- --54 3210     Color


***************************************************************************/

inline void fuuki32_state::get_tile_info8bpp(tile_data &tileinfo, tilemap_memory_index tile_index, int _N_)
{
	UINT16 code = (m_vram[_N_][tile_index] & 0xffff0000) >> 16;
	UINT16 attr = (m_vram[_N_][tile_index] & 0x0000ffff);
	SET_TILE_INFO_MEMBER(1 + _N_, code, (attr & 0x3f) >> 4, TILE_FLIPYX(attr >> 6));
}

TILE_GET_INFO_MEMBER(fuuki32_state::get_tile_info_0){ get_tile_info8bpp(tileinfo, tile_index, 0); }
TILE_GET_INFO_MEMBER(fuuki32_state::get_tile_info_1){ get_tile_info8bpp(tileinfo, tile_index, 1); }

inline void fuuki32_state::get_tile_info4bpp(tile_data &tileinfo, tilemap_memory_index tile_index, int _N_)
{
	UINT16 code = (m_vram[_N_][tile_index] & 0xffff0000) >> 16;
	UINT16 attr = (m_vram[_N_][tile_index] & 0x0000ffff);
	SET_TILE_INFO_MEMBER(1 + _N_, code, attr & 0x3f, TILE_FLIPYX(attr >> 6));
}

TILE_GET_INFO_MEMBER(fuuki32_state::get_tile_info_2){ get_tile_info4bpp(tileinfo, tile_index, 2); }
TILE_GET_INFO_MEMBER(fuuki32_state::get_tile_info_3){ get_tile_info4bpp(tileinfo, tile_index, 3); }

inline void fuuki32_state::vram_w(offs_t offset, UINT32 data, UINT32 mem_mask, int _N_)
{
	COMBINE_DATA(&m_vram[_N_][offset]);
	m_tilemap[_N_]->mark_tile_dirty(offset);
}

WRITE32_MEMBER(fuuki32_state::vram_0_w){ vram_w(offset, data, mem_mask, 0); }
WRITE32_MEMBER(fuuki32_state::vram_1_w){ vram_w(offset, data, mem_mask, 1); }
WRITE32_MEMBER(fuuki32_state::vram_2_w){ vram_w(offset, data, mem_mask, 2); }
WRITE32_MEMBER(fuuki32_state::vram_3_w){ vram_w(offset, data, mem_mask, 3); }


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void fuuki32_state::video_start()
{
//  m_buf_spriteram = auto_alloc_array(machine(), UINT32, m_spriteram.bytes() / 4);
//  m_buf_spriteram2 = auto_alloc_array(machine(), UINT32, m_spriteram.bytes() / 4);



	m_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fuuki32_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fuuki32_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fuuki32_state::get_tile_info_2),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fuuki32_state::get_tile_info_3),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[0]->set_transparent_pen(0xff);    // 8 bits
	m_tilemap[1]->set_transparent_pen(0xff);    // 8 bits
	m_tilemap[2]->set_transparent_pen(0x0f);    // 4 bits
	m_tilemap[3]->set_transparent_pen(0x0f);    // 4 bits

	//m_gfxdecode->gfx(1)->set_granularity(16); /* 256 colour tiles with palette selectable on 16 colour boundaries */
	//m_gfxdecode->gfx(2)->set_granularity(16);
}



/***************************************************************************


                                Screen Drawing

    Video Registers (vregs):

        00.w        Layer 0 Scroll Y
        02.w        Layer 0 Scroll X
        04.w        Layer 1 Scroll Y
        06.w        Layer 1 Scroll X
        08.w        Layer 2 Scroll Y
        0a.w        Layer 2 Scroll X
        0c.w        Layers Y Offset
        0e.w        Layers X Offset

        10-1a.w     ? 0
        1c.w        Trigger a level 5 irq on this raster line
        1e.w        ? $3390/$3393 (Flip Screen Off/On), $0040 is buffer for tilemap 2 or 3

    Priority Register (priority):

        fedc ba98 7654 3---
        ---- ---- ---- -210     Layer Order


    Unknown Registers ($de0000.l):

        00.w        ? $0200/$0201   (Flip Screen Off/On)
        02.w        ? $f300/$0330

***************************************************************************/

/* Wrapper to handle bg and bg2 ttogether */
void fuuki32_state::draw_layer( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int i, int flag, int pri )
{
	int buffer = ((m_vregs[0x1e / 4] & 0x0000ffff) & 0x40);

	switch( i )
	{
		case 2: if (buffer) m_tilemap[3]->draw(screen, bitmap, cliprect, flag, pri);
				else        m_tilemap[2]->draw(screen, bitmap, cliprect, flag, pri);
				return;
		case 1: m_tilemap[1]->draw(screen, bitmap, cliprect, flag, pri);
				return;
		case 0: m_tilemap[0]->draw(screen, bitmap, cliprect, flag, pri);
				return;
	}
}

UINT32 fuuki32_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 layer0_scrollx, layer0_scrolly;
	UINT16 layer1_scrollx, layer1_scrolly;
	UINT16 layer2_scrollx, layer2_scrolly;
	UINT16 scrollx_offs,   scrolly_offs;

	/*
	It's not independent bits causing layers to switch, that wouldn't make sense with 3 bits.
	*/

	static const int pri_table[6][3] = {
		{ 0, 1, 2 }, // Special moves 0>1, 0>2 (0,1,2 or 0,2,1)
		{ 0, 2, 1 }, // Two Levels - 0>1 (0,1,2 or 0,2,1 or 2,0,1)
		{ 1, 0, 2 }, // Most Levels - 2>1 1>0 2>0 (1,0,2)
		{ 1, 2, 0 }, // Not used?
		{ 2, 0, 1 }, // Title etc. - 0>1 (0,1,2 or 0,2,1 or 2,0,1)
		{ 2, 1, 0 }}; // Char Select, prison stage 1>0 (leaves 1,2,0 or 2,1,0)

	int tm_front  = pri_table[(m_priority[0] >> 16) & 0x0f][0];
	int tm_middle = pri_table[(m_priority[0] >> 16) & 0x0f][1];
	int tm_back   = pri_table[(m_priority[0] >> 16) & 0x0f][2];

	flip_screen_set((m_vregs[0x1e / 4] & 0x0000ffff) & 1);

	/* Layers scrolling */

	scrolly_offs = ((m_vregs[0xc / 4] & 0xffff0000) >> 16) - (flip_screen() ? 0x103 : 0x1f3);
	scrollx_offs =  (m_vregs[0xc / 4] & 0x0000ffff) - (flip_screen() ? 0x2c7 : 0x3f6);

	layer0_scrolly = ((m_vregs[0x0 / 4] & 0xffff0000) >> 16) + scrolly_offs;
	layer0_scrollx = ((m_vregs[0x0 / 4] & 0x0000ffff)) + scrollx_offs;
	layer1_scrolly = ((m_vregs[0x4 / 4] & 0xffff0000) >> 16) + scrolly_offs;
	layer1_scrollx = ((m_vregs[0x4 / 4] & 0x0000ffff)) + scrollx_offs;

	layer2_scrolly = ((m_vregs[0x8 / 4] & 0xffff0000) >> 16);
	layer2_scrollx = ((m_vregs[0x8 / 4] & 0x0000ffff));

	m_tilemap[0]->set_scrollx(0, layer0_scrollx);
	m_tilemap[0]->set_scrolly(0, layer0_scrolly);
	m_tilemap[1]->set_scrollx(0, layer1_scrollx);
	m_tilemap[1]->set_scrolly(0, layer1_scrolly);

	m_tilemap[2]->set_scrollx(0, layer2_scrollx);
	m_tilemap[2]->set_scrolly(0, layer2_scrolly);
	m_tilemap[3]->set_scrollx(0, layer2_scrollx);
	m_tilemap[3]->set_scrolly(0, layer2_scrolly);

	/* The bg colour is the last pen i.e. 0x1fff */
	bitmap.fill((0x800 * 4) - 1, cliprect);
	screen.priority().fill(0, cliprect);

	draw_layer(screen, bitmap, cliprect, tm_back,   0, 1);
	draw_layer(screen, bitmap, cliprect, tm_middle, 0, 2);
	draw_layer(screen, bitmap, cliprect, tm_front,  0, 4);

	m_fuukivid->draw_sprites(screen, bitmap, cliprect, flip_screen(), m_spr_buffered_tilebank);
	return 0;
}

void fuuki32_state::screen_eof(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		/* Buffer sprites and tilebank by 2 frames */
		m_spr_buffered_tilebank[1] = m_spr_buffered_tilebank[0];
		m_spr_buffered_tilebank[0] = m_tilebank[0];
		m_fuukivid->buffer_sprites();
	}
}
