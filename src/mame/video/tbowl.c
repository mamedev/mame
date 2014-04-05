/* video/tbowl.c */

/* see drivers/tbowl.c for more info */

#include "emu.h"
#include "includes/tbowl.h"


/* Foreground Layer (tx) Tilemap */

TILE_GET_INFO_MEMBER(tbowl_state::get_tx_tile_info)
{
	int tileno;
	int col;

	tileno = m_txvideoram[tile_index] | ((m_txvideoram[tile_index+0x800] & 0x07) << 8);
	col = (m_txvideoram[tile_index+0x800] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(0,tileno,col,0);
}

WRITE8_MEMBER(tbowl_state::tbowl_txvideoram_w)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}

/* Bottom BG Layer (bg) Tilemap */

TILE_GET_INFO_MEMBER(tbowl_state::get_bg_tile_info)
{
	int tileno;
	int col;

	tileno = m_bgvideoram[tile_index] | ((m_bgvideoram[tile_index+0x1000] & 0x0f) << 8);
	col = (m_bgvideoram[tile_index+0x1000] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(1,tileno,col,0);
}

WRITE8_MEMBER(tbowl_state::tbowl_bg2videoram_w)
{
	m_bg2videoram[offset] = data;
	m_bg2_tilemap->mark_tile_dirty(offset & 0xfff);
}

WRITE8_MEMBER(tbowl_state::tbowl_bgxscroll_lo)
{
	m_xscroll = (m_xscroll & 0xff00) | data;
}

WRITE8_MEMBER(tbowl_state::tbowl_bgxscroll_hi)
{
	m_xscroll = (m_xscroll & 0x00ff) | (data << 8);
}

WRITE8_MEMBER(tbowl_state::tbowl_bgyscroll_lo)
{
	m_yscroll = (m_yscroll & 0xff00) | data;
}

WRITE8_MEMBER(tbowl_state::tbowl_bgyscroll_hi)
{
	m_yscroll = (m_yscroll & 0x00ff) | (data << 8);
}

/* Middle BG Layer (bg2) Tilemaps */

TILE_GET_INFO_MEMBER(tbowl_state::get_bg2_tile_info)
{
	int tileno;
	int col;

	tileno = m_bg2videoram[tile_index] | ((m_bg2videoram[tile_index+0x1000] & 0x0f) << 8);
	tileno ^= 0x400;
	col = (m_bg2videoram[tile_index+0x1000] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(2,tileno,col,0);
}

WRITE8_MEMBER(tbowl_state::tbowl_bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0xfff);
}

WRITE8_MEMBER(tbowl_state::tbowl_bg2xscroll_lo)
{
	m_bg2xscroll = (m_bg2xscroll & 0xff00) | data;
}

WRITE8_MEMBER(tbowl_state::tbowl_bg2xscroll_hi)
{
	m_bg2xscroll = (m_bg2xscroll & 0x00ff) | (data << 8);
}

WRITE8_MEMBER(tbowl_state::tbowl_bg2yscroll_lo)
{
	m_bg2yscroll = (m_bg2yscroll & 0xff00) | data;
}

WRITE8_MEMBER(tbowl_state::tbowl_bg2yscroll_hi)
{
	m_bg2yscroll = (m_bg2yscroll & 0x00ff) | (data << 8);
}


/*** Video Start / Update ***/

void tbowl_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tbowl_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tbowl_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16,128,32);
	m_bg2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tbowl_state::get_bg2_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16,128,32);

	m_tx_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg2_tilemap->set_transparent_pen(0);
}


void tbowl_state::tbowl_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, int xscroll, UINT8* spriteram)
{
	int offs;
	static const UINT8 layout[8][8] =
	{
		{0,1,4,5,16,17,20,21},
		{2,3,6,7,18,19,22,23},
		{8,9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	for (offs = 0;offs < 0x800;offs += 8)
	{
		if (spriteram[offs+0] & 0x80)  /* enable */
		{
			int code,color,sizex,sizey,flipx,flipy,xpos,ypos;
			int x,y;//,priority,priority_mask;

			code = (spriteram[offs+2])+(spriteram[offs+1]<<8);
			color = (spriteram[offs+3])&0x1f;
			sizex = 1 << ((spriteram[offs+0] & 0x03) >> 0);
			sizey = 1 << ((spriteram[offs+0] & 0x0c) >> 2);

			flipx = (spriteram[offs+0])&0x20;
			flipy = 0;
			xpos = (spriteram[offs+6])+((spriteram[offs+4]&0x03)<<8);
			ypos = (spriteram[offs+5])+((spriteram[offs+4]&0x10)<<4);

			/* bg: 1; fg:2; text: 4 */

			for (y = 0;y < sizey;y++)
			{
				for (x = 0;x < sizex;x++)
				{
					int sx = xpos + 8*(flipx?(sizex-1-x):x);
					int sy = ypos + 8*(flipy?(sizey-1-y):y);

					sx -= xscroll;

					m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
							code + layout[y][x],
							color,
							flipx,flipy,
							sx,sy,0 );

					/* wraparound */
					m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
							code + layout[y][x],
							color,
							flipx,flipy,
							sx,sy-0x200,0 );

					/* wraparound */
					m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
							code + layout[y][x],
							color,
							flipx,flipy,
							sx-0x400,sy,0 );

					/* wraparound */
					m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
							code + layout[y][x],
							color,
							flipx,flipy,
							sx-0x400,sy-0x200,0 );



				}
			}
		}
	}
}

UINT32 tbowl_state::screen_update_tbowl_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_xscroll );
	m_bg_tilemap->set_scrolly(0, m_yscroll );
	m_bg2_tilemap->set_scrollx(0, m_bg2xscroll );
	m_bg2_tilemap->set_scrolly(0, m_bg2yscroll );
	m_tx_tilemap->set_scrollx(0, 0 );
	m_tx_tilemap->set_scrolly(0, 0 );

	bitmap.fill(0x100, cliprect); /* is there a register controling the colour? looks odd when screen is blank */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	tbowl_draw_sprites(bitmap,cliprect, 0, m_spriteram);
	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}

UINT32 tbowl_state::screen_update_tbowl_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_xscroll+32*8 );
	m_bg_tilemap->set_scrolly(0, m_yscroll );
	m_bg2_tilemap->set_scrollx(0, m_bg2xscroll+32*8 );
	m_bg2_tilemap->set_scrolly(0, m_bg2yscroll );
	m_tx_tilemap->set_scrollx(0, 32*8 );
	m_tx_tilemap->set_scrolly(0, 0 );

	bitmap.fill(0x100, cliprect); /* is there a register controling the colour? looks odd when screen is blank */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	tbowl_draw_sprites(bitmap,cliprect, 32*8, m_spriteram);
	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}
