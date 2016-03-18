// license:BSD-3-Clause
// copyright-holders:David Haywood
/* video/angelkds.c - see drivers/angelkds.c for more info */

/* graphical issues

enable / disable tilemap bits might be wrong

*/

#include "emu.h"
#include "includes/angelkds.h"


/*** Text Layer Tilemap

*/

TILE_GET_INFO_MEMBER(angelkds_state::get_tx_tile_info)
{
	int tileno;

	tileno = m_txvideoram[tile_index] + (m_txbank * 0x100);
	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}

WRITE8_MEMBER(angelkds_state::angelkds_txvideoram_w)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(angelkds_state::angelkds_txbank_write)
{
	if (m_txbank != data)
	{
		m_txbank = data;
		m_tx_tilemap->mark_all_dirty();
	}
}

/*** Top Half Background Tilemap

*/

TILE_GET_INFO_MEMBER(angelkds_state::get_bgtop_tile_info)
{
	int tileno;

	tileno = m_bgtopvideoram[tile_index];

	tileno += m_bgtopbank * 0x100 ;
	SET_TILE_INFO_MEMBER(1, tileno, 0, 0);
}

WRITE8_MEMBER(angelkds_state::angelkds_bgtopvideoram_w)
{
	m_bgtopvideoram[offset] = data;
	m_bgtop_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(angelkds_state::angelkds_bgtopbank_write)
{
	if (m_bgtopbank != data)
	{
		m_bgtopbank = data;
		m_bgtop_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(angelkds_state::angelkds_bgtopscroll_write)
{
	m_bgtop_tilemap->set_scrollx(0, data);
}

/*** Bottom Half Background Tilemap

*/

TILE_GET_INFO_MEMBER(angelkds_state::get_bgbot_tile_info)
{
	int tileno;

	tileno = m_bgbotvideoram[tile_index];

	tileno += m_bgbotbank * 0x100 ;
	SET_TILE_INFO_MEMBER(2, tileno, 1, 0);
}

WRITE8_MEMBER(angelkds_state::angelkds_bgbotvideoram_w)
{
	m_bgbotvideoram[offset] = data;
	m_bgbot_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(angelkds_state::angelkds_bgbotbank_write)
{
	if (m_bgbotbank != data)
	{
		m_bgbotbank = data;
		m_bgbot_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(angelkds_state::angelkds_bgbotscroll_write)
{
	m_bgbot_tilemap->set_scrollx(0, data);
}


WRITE8_MEMBER(angelkds_state::angelkds_layer_ctrl_write)
{
	m_layer_ctrl = data;
}

/*** Sprites

the sprites are similar to the tilemaps in the sense that there is
a split down the middle of the screen

*/

void angelkds_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int enable_n)
{
	const UINT8 *source = m_spriteram + 0x100 - 4;
	const UINT8 *finish = m_spriteram;
	gfx_element *gfx = m_gfxdecode->gfx(3);

	while (source >= finish)
	{
	/*

	nnnn nnnn - EeFf B?cc - yyyy yyyy - xxxx xxxx

	n = sprite number
	E = Sprite Enabled in Top Half of Screen
	e = Sprite Enabled in Bottom Half of Screen
	F = Flip Y
	f = Flip X
	B = Tile Bank
	? = unknown, nothing / unused? recheck
	c = color
	y = Y position
	x = X position

	*/
		UINT16 tile_no = source[0];
		UINT8 attr = source[1];
		UINT8 ypos = source[2];
		UINT8 xpos = source[3];

		UINT8 enable = attr & 0xc0;
		UINT8 flipx = (attr & 0x10) >> 4;
		UINT8 flipy = (attr & 0x20) >> 5;
		UINT8 bank = attr & 0x08;
		UINT8 color = attr & 0x03;

		if (bank)
			tile_no += 0x100;

		ypos = 0xff - ypos;

		if (enable & enable_n)
		{
					gfx->transpen(
					bitmap,
					cliprect,
					tile_no,
					color*4,
					flipx,flipy,
					xpos,ypos,15
					);
			/* wraparound */
			if (xpos > 240)

						gfx->transpen(
						bitmap,
						cliprect,
						tile_no,
						color*4,
						flipx,flipy,
						xpos-256,ypos,15
						);
			/* wraparound */
			if (ypos > 240)
			{
						gfx->transpen(
						bitmap,
						cliprect,
						tile_no,
						color*4,
						flipx,flipy,
						xpos,ypos-256,15
						);
				/* wraparound */
				if (xpos > 240)

							gfx->transpen(
							bitmap,
							cliprect,
							tile_no,
							color*4,
							flipx,flipy,
							xpos-256,ypos-256,15
							);
			}
		}

		source -= 0x04;
	}

}


/*** Video Start & Update

*/

void angelkds_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(angelkds_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tx_tilemap->set_transparent_pen(0);

	m_bgbot_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(angelkds_state::get_bgbot_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bgbot_tilemap->set_transparent_pen(15);

	m_bgtop_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(angelkds_state::get_bgtop_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bgtop_tilemap->set_transparent_pen(15);
}

/* enable bits are uncertain */

UINT32 angelkds_state::screen_update_angelkds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = screen.visible_area();
	rectangle clip;

	bitmap.fill(0x3f, cliprect); /* is there a register controling the colour?, we currently use the last colour of the tx palette */

	/* draw top of screen */
	clip.set(8*0, 8*16-1, visarea.min_y, visarea.max_y);

	if ((m_layer_ctrl & 0x80) == 0x00)
		m_bgtop_tilemap->draw(screen, bitmap, clip, 0, 0);

	draw_sprites(bitmap, clip, 0x80);

	if ((m_layer_ctrl & 0x20) == 0x00)
		m_tx_tilemap->draw(screen, bitmap, clip, 0, 0);

	/* draw bottom of screen */
	clip.set(8*16, 8*32-1, visarea.min_y, visarea.max_y);

	if ((m_layer_ctrl & 0x40) == 0x00)
		m_bgbot_tilemap->draw(screen, bitmap, clip, 0, 0);

	draw_sprites(bitmap, clip, 0x40);

	if ((m_layer_ctrl & 0x20) == 0x00)
		m_tx_tilemap->draw(screen, bitmap, clip, 0, 0);

	return 0;
}
