// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Peter Ferrie
/***************************************************************************

  Gaelco Type 1 Video Hardware Rev B

  The video hardware it's nearly identical to the previous
  revision but it can handle more tiles and more sprites

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/thoop2.h"


/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- ------xx | code (high bits)
      0  | xxxxxxxx xxxxxx-- | code (low bits)
      1  | -------- --xxxxxx | color
      1  | -------- xx------ | priority
      1  | --xxxxxx -------- | not used
      1  | -x------ -------- | flip x
      1  | x------- -------- | flip y
*/

TILE_GET_INFO_MEMBER(thoop2_state::get_tile_info_thoop2_screen0)
{
	int data = m_videoram[tile_index << 1];
	int data2 = m_videoram[(tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2) | ((data & 0x0003) << 14);

	tileinfo.category = (data2 >> 6) & 0x03;

	SET_TILE_INFO_MEMBER(1, code, data2 & 0x3f, TILE_FLIPYX((data2 >> 14) & 0x03));
}


TILE_GET_INFO_MEMBER(thoop2_state::get_tile_info_thoop2_screen1)
{
	int data = m_videoram[(0x1000/2) + (tile_index << 1)];
	int data2 = m_videoram[(0x1000/2) + (tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2) | ((data & 0x0003) << 14);

	tileinfo.category = (data2 >> 6) & 0x03;

	SET_TILE_INFO_MEMBER(1, code, data2 & 0x3f, TILE_FLIPYX((data2 >> 14) & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_MEMBER(thoop2_state::thoop2_vram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_pant[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

void thoop2_state::video_start()
{
	int i;

	m_pant[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(thoop2_state::get_tile_info_thoop2_screen0),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_pant[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(thoop2_state::get_tile_info_thoop2_screen1),this),TILEMAP_SCAN_ROWS,16,16,32,32);

	m_pant[0]->set_transmask(0,0xff01,0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */
	m_pant[1]->set_transmask(0,0xff01,0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */

	for (i = 0; i < 5; i++){
		m_sprite_table[i] = std::make_unique<int[]>(512);
	}
}

/***************************************************************************

    Sprites

***************************************************************************/

void thoop2_state::thoop2_sort_sprites()
{
	int i;

	m_sprite_count[0] = 0;
	m_sprite_count[1] = 0;
	m_sprite_count[2] = 0;
	m_sprite_count[3] = 0;
	m_sprite_count[4] = 0;

	for (i = 3; i < (0x1000 - 6)/2; i += 4){
		int color = (m_spriteram[i+2] & 0x7e00) >> 9;
		int priority = (m_spriteram[i] & 0x3000) >> 12;

		/* palettes 0x38-0x3f are used for high priority sprites in Big Karnak */
		if (color >= 0x38){
			m_sprite_table[4][m_sprite_count[4]] = i;
			m_sprite_count[4]++;
		}

		/* save sprite number in the proper array for later */
		m_sprite_table[priority][m_sprite_count[priority]] = i;
		m_sprite_count[priority]++;
	}
}

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | -----xxx -------- | not used
      0  | ----x--- -------- | sprite size
      0  | --xx---- -------- | sprite priority
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used
      2  | -------x xxxxxxxx | x position
      2  | -xxxxxx- -------- | sprite color
      3  | -------- ------xx | sprite code (high bits)
      3  | xxxxxxxx xxxxxx-- | sprite code (low bits)
*/

void thoop2_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	int j, x, y, ex, ey;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	static const int x_offset[2] = {0x0,0x2};
	static const int y_offset[2] = {0x0,0x1};

	for (j = 0; j < m_sprite_count[pri]; j++){
		int i = m_sprite_table[pri][j];
		int sx = m_spriteram[i+2] & 0x01ff;
		int sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = m_spriteram[i+3];
		int color = (m_spriteram[i+2] & 0x7e00) >> 9;
		int attr = (m_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size;

		number |= ((number & 0x03) << 16);

		if (attr & 0x04){
			spr_size = 1;
		}
		else{
			spr_size = 2;
			number &= (~3);
		}

		for (y = 0; y < spr_size; y++){
			for (x = 0; x < spr_size; x++){
				ex = xflip ? (spr_size-1-x) : x;
				ey = yflip ? (spr_size-1-y) : y;

				gfx->transpen(bitmap,cliprect,number + x_offset[ex] + y_offset[ey],
						color,xflip,yflip,
						sx-0x0f+x*8,sy+y*8,0);
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

UINT32 thoop2_state::screen_update_thoop2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* set scroll registers */
	m_pant[0]->set_scrolly(0, m_vregs[0]);
	m_pant[0]->set_scrollx(0, m_vregs[1]+4);
	m_pant[1]->set_scrolly(0, m_vregs[2]);
	m_pant[1]->set_scrollx(0, m_vregs[3]);

	thoop2_sort_sprites();

	bitmap.fill(0, cliprect );

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3,0);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3,0);
	draw_sprites(bitmap,cliprect,3);
	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3,0);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3,0);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2,0);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2,0);
	draw_sprites(bitmap,cliprect,2);
	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2,0);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2,0);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1,0);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1,0);
	draw_sprites(bitmap,cliprect,1);
	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1,0);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1,0);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0,0);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0,0);
	draw_sprites(bitmap,cliprect,0);
	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0,0);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0,0);

	draw_sprites(bitmap,cliprect,4);
	return 0;
}
