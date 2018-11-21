// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Mike Coates, Nicola Salmoria, Miguel Angel Horna
/***************************************************************************

  World Rally Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/wrally.h"


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
      1  | -------- ---xxxxx | color
      1  | -------- --x----- | priority
      1  | -------- -x------ | flip y
      1  | -------- x------- | flip x
      1  | ---xxxxx -------- | data used to handle collisions, speed, etc
      1  | xxx----- -------- | not used?
*/

template<int Layer>
TILE_GET_INFO_MEMBER(wrally_state::get_tile_info)
{
	int data = m_videoram[(Layer * 0x2000/2) + (tile_index << 1)];
	int data2 = m_videoram[(Layer * 0x2000/2) + (tile_index << 1) + 1];
	int code = data & 0x3fff;

	tileinfo.category = (data2 >> 5) & 0x01;

	SET_TILE_INFO_MEMBER(0, code, data2 & 0x1f, TILE_FLIPYX((data2 >> 6) & 0x03));
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

void wrally_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(wrally_state::get_tile_info<0>),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(wrally_state::get_tile_info<1>),this),TILEMAP_SCAN_ROWS,16,16,64,32);

	m_tilemap[0]->set_transmask(0,0xff01,0x00ff); /* this layer is split in two (pens 1..7, pens 8-15) */
	m_tilemap[1]->set_transparent_pen(0);
}

/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t wrally_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sprites->draw_sprites(cliprect,m_spriteram,flip_screen());

	/* set scroll registers */
	if (!flip_screen()) {
		m_tilemap[0]->set_scrolly(0, m_vregs[0]);
		m_tilemap[0]->set_scrollx(0, m_vregs[1]+4);
		m_tilemap[1]->set_scrolly(0, m_vregs[2]);
		m_tilemap[1]->set_scrollx(0, m_vregs[3]);
	} else {
		m_tilemap[0]->set_scrolly(0, 248 - m_vregs[0]);
		m_tilemap[0]->set_scrollx(0, 1024 - m_vregs[1] - 4);
		m_tilemap[1]->set_scrolly(0, 248 - m_vregs[2]);
		m_tilemap[1]->set_scrollx(0, 1024 - m_vregs[3]);
	}

	/* draw tilemaps + sprites */
	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_LAYER0,0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_LAYER1,0);

	m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1),0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_LAYER0,0);

	m_sprites->mix_sprites(bitmap, cliprect, 0);

	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_LAYER1,0);

	m_sprites->mix_sprites(bitmap, cliprect, 1);

	return 0;
}
