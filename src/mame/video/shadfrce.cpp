// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "includes/shadfrce.h"

TILE_GET_INFO_MEMBER(shadfrce_state::get_fgtile_info)
{
	/* ---- ----  tttt tttt  ---- ----  pppp TTTT */
	int tileno, colour;

	tileno = (m_fgvideoram[tile_index *2] & 0x00ff) | ((m_fgvideoram[tile_index *2+1] & 0x000f) << 8);
	colour = (m_fgvideoram[tile_index *2+1] & 0x00f0) >>4;

	SET_TILE_INFO_MEMBER(0,tileno,colour*4,0);
}

WRITE16_MEMBER(shadfrce_state::fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fgtilemap->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(shadfrce_state::get_bg0tile_info)
{
	/* ---- ----  ---- cccc  --TT TTTT TTTT TTTT */
	int tileno, colour,fyx;

	tileno = (m_bg0videoram[tile_index *2+1] & 0x3fff);
	colour = m_bg0videoram[tile_index *2] & 0x001f;
	if (colour & 0x10) colour ^= 0x30;  /* skip hole */
	fyx = (m_bg0videoram[tile_index *2] & 0x00c0) >>6;

	SET_TILE_INFO_MEMBER(2,tileno,colour,TILE_FLIPYX(fyx));
}

WRITE16_MEMBER(shadfrce_state::bg0videoram_w)
{
	m_bg0videoram[offset] = data;
	m_bg0tilemap->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(shadfrce_state::get_bg1tile_info)
{
	int tileno, colour;

	tileno = (m_bg1videoram[tile_index] & 0x0fff);
	colour = (m_bg1videoram[tile_index] & 0xf000) >> 12;

	SET_TILE_INFO_MEMBER(2,tileno,colour+64,0);
}

WRITE16_MEMBER(shadfrce_state::bg1videoram_w)
{
	m_bg1videoram[offset] = data;
	m_bg1tilemap->mark_tile_dirty(offset);
}




void shadfrce_state::video_start()
{
	m_fgtilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(shadfrce_state::get_fgtile_info),this),TILEMAP_SCAN_ROWS,    8,  8,64,32);
	m_fgtilemap->set_transparent_pen(0);

	m_bg0tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(shadfrce_state::get_bg0tile_info),this),TILEMAP_SCAN_ROWS, 16, 16,32,32);
	m_bg0tilemap->set_transparent_pen(0);

	m_bg1tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(shadfrce_state::get_bg1tile_info),this),TILEMAP_SCAN_ROWS, 16, 16,32,32);

	m_spvideoram_old = std::make_unique<UINT16[]>(m_spvideoram.bytes()/2);

	save_item(NAME(m_video_enable));
	save_item(NAME(m_irqs_enable));
	save_item(NAME(m_raster_scanline));
	save_item(NAME(m_raster_irq_enable));
	save_item(NAME(m_vblank));
	save_item(NAME(m_prev_value));
}

WRITE16_MEMBER(shadfrce_state::bg0scrollx_w)
{
	m_bg0tilemap->set_scrollx(0, data & 0x1ff );
}

WRITE16_MEMBER(shadfrce_state::bg0scrolly_w)
{
	m_bg0tilemap->set_scrolly(0, data  & 0x1ff );
}

WRITE16_MEMBER(shadfrce_state::bg1scrollx_w)
{
	m_bg1tilemap->set_scrollx(0, data  & 0x1ff );
}

WRITE16_MEMBER(shadfrce_state::bg1scrolly_w)
{
	m_bg1tilemap->set_scrolly(0, data & 0x1ff );
}




void shadfrce_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/* | ---- ---- hhhf Fe-Y | ---- ---- yyyy yyyy | ---- ---- TTTT TTTT | ---- ---- tttt tttt |
	   | ---- ---- -pCc cccX | ---- ---- xxxx xxxx | ---- ---- ---- ---- | ---- ---- ---- ---- | */

	/* h  = height
	   f  = flipx
	   F  = flipy
	   e  = enable
	   Yy = Y Position
	   Tt = Tile No.
	   Xx = X Position
	   Cc = color
	   P = priority
	*/

	gfx_element *gfx = m_gfxdecode->gfx(1);
	UINT16 *finish = m_spvideoram_old.get();
	UINT16 *source = finish + 0x2000/2 - 8;
	int hcount;
	while( source>=finish )
	{
		int ypos = 0x100 - (((source[0] & 0x0003) << 8) | (source[1] & 0x00ff));
		int xpos = (((source[4] & 0x0001) << 8) | (source[5] & 0x00ff)) + 1;
		int tile = ((source[2] & 0x00ff) << 8) | (source[3] & 0x00ff);
		int height = (source[0] & 0x00e0) >> 5;
		int enable = ((source[0] & 0x0004));
		int flipx = ((source[0] & 0x0010) >> 4);
		int flipy = ((source[0] & 0x0008) >> 3);
		int pal = ((source[4] & 0x003e));
		int pri_mask = (source[4] & 0x0040) ? 0x02 : 0x00;

		if (pal & 0x20) pal ^= 0x60;    /* skip hole */

		height++;
		if (enable) {
			for (hcount=0;hcount<height;hcount++) {
				gfx->prio_transpen(bitmap,cliprect,tile+hcount,pal,flipx,flipy,xpos,ypos-hcount*16-16,screen.priority(),pri_mask,0);
				gfx->prio_transpen(bitmap,cliprect,tile+hcount,pal,flipx,flipy,xpos-0x200,ypos-hcount*16-16,screen.priority(),pri_mask,0);
				gfx->prio_transpen(bitmap,cliprect,tile+hcount,pal,flipx,flipy,xpos,ypos-hcount*16-16+0x200,screen.priority(),pri_mask,0);
				gfx->prio_transpen(bitmap,cliprect,tile+hcount,pal,flipx,flipy,xpos-0x200,ypos-hcount*16-16+0x200,screen.priority(),pri_mask,0);
			}
		}
		source-=8;
	}
}

UINT32 shadfrce_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	if (m_video_enable)
	{
		m_bg1tilemap->draw(screen, bitmap, cliprect, 0,0);
		m_bg0tilemap->draw(screen, bitmap, cliprect, 0,1);
		draw_sprites(screen,bitmap,cliprect);
		m_fgtilemap->draw(screen, bitmap, cliprect, 0,0);
	}
	else
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
	}

	return 0;
}

void shadfrce_state::screen_eof(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		/* looks like sprites are *two* frames ahead */
		memcpy(m_spvideoram_old.get(), m_spvideoram, m_spvideoram.bytes());
	}
}
