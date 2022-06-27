// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood, Luca Elia
/***************************************************************************

                          -= Fuuki 32 Bit Games (FG-3) =-

                driver by Paul Priest and David Haywood
                based on fuukifg2 by Luca Elia


    [ 4 Scrolling Layers ]

                            [ Layer 0 ]     [ Layer 1 ]     [ Layer 2 (double-buffered) ]

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
#include "fuukifg3.h"


/***************************************************************************


                                    Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code

        2.w     fedc ba98 ---- ----
                ---- ---- 7--- ----     Flip Y
                ---- ---- -6-- ----     Flip X
                ---- ---- --54 3210     Color


***************************************************************************/

template<int Layer, int ColShift>
TILE_GET_INFO_MEMBER(fuuki32_state::get_tile_info)
{
	const int buffer = (Layer < 2) ? 0 : (m_vregs[0x1e / 2] & 0x40) >> 6;
	const u16 code = (m_vram[Layer|buffer][tile_index] & 0xffff0000) >> 16;
	const u16 attr = (m_vram[Layer|buffer][tile_index] & 0x0000ffff);
	tileinfo.set(Layer, code, (attr & 0x3f) >> ColShift, TILE_FLIPYX(attr >> 6));
}

/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void fuuki32_state::video_start()
{
	const u32 spriteram_size = m_spriteram.bytes();
	m_buf_spriteram[0] = make_unique_clear<u16[]>(spriteram_size / 2);
	m_buf_spriteram[1] = make_unique_clear<u16[]>(spriteram_size / 2);

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fuuki32_state::get_tile_info<0, 4>))), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fuuki32_state::get_tile_info<1, 4>))), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fuuki32_state::get_tile_info<2, 0>))), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);

	m_tilemap[0]->set_transparent_pen(0xff);    // 8 bits
	m_tilemap[1]->set_transparent_pen(0xff);    // 8 bits
	m_tilemap[2]->set_transparent_pen(0x0f);    // 4 bits

	//m_gfxdecode->gfx(0)->set_granularity(16); /* 256 colour tiles with palette selectable on 16 colour boundaries */
	//m_gfxdecode->gfx(1)->set_granularity(16);

	save_pointer(NAME(m_buf_spriteram[0]), spriteram_size / 2);
	save_pointer(NAME(m_buf_spriteram[1]), spriteram_size / 2);
}


void fuuki32_state::sprram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);
};

u16 fuuki32_state::sprram_r(offs_t offset)
{
	return m_spriteram[offset];
}

void fuuki32_state::tile_cb(u32 &code)
{
	const u32 bank = (code & 0xc000) >> 14;

	const u32 bank_lookedup = ((m_spr_buffered_tilebank[1] & 0xffff0000) >> (16 + bank * 4)) & 0xf;
	code &= 0x3fff;
	code += bank_lookedup * 0x4000;
}

void fuuki32_state::colpri_cb(u32 &colour, u32 &pri_mask)
{
	const u8 priority = (colour >> 6) & 3;
	switch (priority)
	{
		case 3:  pri_mask = 0xf0 | 0xcc | 0xaa;  break;  // behind all layers
		case 2:  pri_mask = 0xf0 | 0xcc;         break;  // behind fg + middle layer
		case 1:  pri_mask = 0xf0;                break;  // behind fg layer
		case 0:
		default: pri_mask = 0;                       // above all
	}
	colour &= 0x3f;
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
        1e.w        ? $3390/$3393 (Flip Screen Off/On), $0040 is buffer for tilemap 2

    Priority Register (priority):

        fedc ba98 7654 3---
        ---- ---- ---- -210     Layer Order


    Unknown Registers ($de0000.l):

        00.w        ? $0200/$0201   (Flip Screen Off/On)
        02.w        ? $f300/$0330

***************************************************************************/

/* Wrapper to handle bg and bg2 ttogether */
void fuuki32_state::draw_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 i, int flag, u8 pri, u8 primask)
{
	m_tilemap[i]->draw(screen, bitmap, cliprect, flag, pri, primask);
}

u32 fuuki32_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/*
	It's not independent bits causing layers to switch, that wouldn't make sense with 3 bits.
	*/

	static const u8 pri_table[6][3] = {
		{ 0, 1, 2 }, // Special moves 0>1, 0>2 (0,1,2 or 0,2,1)
		{ 0, 2, 1 }, // Two Levels - 0>1 (0,1,2 or 0,2,1 or 2,0,1)
		{ 1, 0, 2 }, // Most Levels - 2>1 1>0 2>0 (1,0,2)
		{ 1, 2, 0 }, // Not used?
		{ 2, 0, 1 }, // Title etc. - 0>1 (0,1,2 or 0,2,1 or 2,0,1)
		{ 2, 1, 0 }}; // Char Select, prison stage 1>0 (leaves 1,2,0 or 2,1,0)

	const u8 tm_front  = pri_table[(m_priority[0] >> 16) & 0x0f][0];
	const u8 tm_middle = pri_table[(m_priority[0] >> 16) & 0x0f][1];
	const u8 tm_back   = pri_table[(m_priority[0] >> 16) & 0x0f][2];

	flip_screen_set(m_vregs[0x1e / 2] & 1);

	/* Layers scrolling */

	const u16 scrolly_offs = m_vregs[0xc / 2] - (flip_screen() ? 0x103 : 0x1f3);
	const u16 scrollx_offs = m_vregs[0xe / 2] - (flip_screen() ? 0x2c7 : 0x3f6);

	const u16 layer0_scrolly = m_vregs[0x0 / 2] + scrolly_offs;
	const u16 layer0_scrollx = m_vregs[0x2 / 2] + scrollx_offs;
	const u16 layer1_scrolly = m_vregs[0x4 / 2] + scrolly_offs;
	const u16 layer1_scrollx = m_vregs[0x6 / 2] + scrollx_offs;

	const u16 layer2_scrolly = m_vregs[0x8 / 2];
	const u16 layer2_scrollx = m_vregs[0xa / 2];

	m_tilemap[0]->set_scrollx(0, layer0_scrollx);
	m_tilemap[0]->set_scrolly(0, layer0_scrolly);
	m_tilemap[1]->set_scrollx(0, layer1_scrollx);
	m_tilemap[1]->set_scrolly(0, layer1_scrolly);

	m_tilemap[2]->set_scrollx(0, layer2_scrollx);
	m_tilemap[2]->set_scrolly(0, layer2_scrolly);

	/* The bg colour is the last pen i.e. 0x1fff */
	bitmap.fill((0x800 * 4) - 1, cliprect);
	screen.priority().fill(0, cliprect);

	draw_layer(screen, bitmap, cliprect, tm_back,   0, 1);
	draw_layer(screen, bitmap, cliprect, tm_middle, 0, 2);
	draw_layer(screen, bitmap, cliprect, tm_front,  0, 4);

	m_fuukivid->draw_sprites(screen, bitmap, cliprect, flip_screen(), m_buf_spriteram[1].get(), m_spriteram.bytes() / 2);
	return 0;
}

WRITE_LINE_MEMBER(fuuki32_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		/* Buffer sprites and tilebank by 2 frames */
		m_spr_buffered_tilebank[1] = m_spr_buffered_tilebank[0];
		m_spr_buffered_tilebank[0] = m_tilebank[0];
		memcpy(m_buf_spriteram[1].get(), m_buf_spriteram[0].get(), m_spriteram.bytes());
		memcpy(m_buf_spriteram[0].get(), m_spriteram, m_spriteram.bytes());
	}
}
