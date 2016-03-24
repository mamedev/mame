// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

  Target Hits Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/targeth.h"


/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (64*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | --xxxxxx xxxxxxxx | code
      0  | xx------ -------- | not used?
      1  | -------- ---xxxxx | color (uses 1st half of the palette)
      1  | -------- --x----- | flip y
      1  | -------- -x------ | flip x
      1  | xxxxxxxx x------- | not used?
*/

TILE_GET_INFO_MEMBER(targeth_state::get_tile_info_screen0)
{
	int data = m_videoram[tile_index << 1];
	int data2 = m_videoram[(tile_index << 1) + 1];
	int code = data & 0x3fff;

	SET_TILE_INFO_MEMBER(0, code, data2 & 0x1f, TILE_FLIPXY((data2 >> 5) & 0x03));
}

TILE_GET_INFO_MEMBER(targeth_state::get_tile_info_screen1)
{
	int data = m_videoram[(0x2000/2) + (tile_index << 1)];
	int data2 = m_videoram[(0x2000/2) + (tile_index << 1) + 1];
	int code = data & 0x3fff;

	SET_TILE_INFO_MEMBER(0, code, data2 & 0x1f, TILE_FLIPXY((data2 >> 5) & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_MEMBER(targeth_state::vram_w)
{
	m_videoram[offset] = data;
	m_pant[(offset & 0x1fff) >> 12]->mark_tile_dirty(((offset << 1) & 0x1fff) >> 2);
}


/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

void targeth_state::video_start()
{
	m_pant[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(targeth_state::get_tile_info_screen0),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_pant[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(targeth_state::get_tile_info_screen1),this),TILEMAP_SCAN_ROWS,16,16,64,32);

	m_pant[0]->set_transparent_pen(0);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used?
      2  | ------xx xxxxxxxx | x position
      2  | -xxxxx-- -------- | sprite color (uses 2nd half of the palette)
      3  | --xxxxxx xxxxxxxx | sprite code
      3  | xx------ -------- | not used?
*/

void targeth_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (int i = 3; i < (0x1000 - 6)/2; i += 4){
		int sx = m_spriteram[i+2] & 0x03ff;
		int sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = m_spriteram[i+3] & 0x3fff;
		int color = (m_spriteram[i+2] & 0x7c00) >> 10;
		int attr = (m_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;

		gfx->transpen(bitmap,cliprect,number,
				0x20 + color,xflip,yflip,
				sx - 0x0f,sy,0);
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

UINT32 targeth_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* set scroll registers */
	m_pant[0]->set_scrolly(0, m_vregs[0]);
	m_pant[0]->set_scrollx(0, m_vregs[1] + 0x04);
	m_pant[1]->set_scrolly(0, m_vregs[2]);
	m_pant[1]->set_scrollx(0, m_vregs[3]);

	m_pant[1]->draw(screen, bitmap, cliprect, 0,0);
	m_pant[0]->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);

	return 0;
}
