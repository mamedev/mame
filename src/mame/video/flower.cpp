// license:???
// copyright-holders:insideoutboy, David Haywood, Stephh
/* Flower Video Hardware */

#include "emu.h"
#include "includes/flower.h"


void flower_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	UINT8 *source = m_spriteram + 0x200;
	UINT8 *finish = source - 0x200;

	source -= 8;

	while( source>=finish )
	{
		int xblock,yblock;
		int sy = 256-32-source[0]+1;
		int sx = (source[4]|(source[5]<<8))-55;
		int code = source[1] & 0x3f;
		int color = (source[6]>>4);

		/*
		    Byte 0: Y
		    Byte 1:
		        0x80 - FlipY
		        0x40 - FlipX
		        0x3f - Tile
		    Byte 2:
		        0x08 - Tile MSB
		        0x01 - Tile MSB
		    Byte 3:
		        0x07 - X Zoom
		        0x08 - X Size
		        0x70 - Y Zoom
		        0x80 - Y Size
		    Byte 4: X LSB
		    Byte 5: X MSB
		    Byte 6:
		        0xf0 - Colour
		*/

		int flipy = source[1] & 0x80;
		int flipx = source[1] & 0x40;

		int size = source[3];

		int xsize = ((size & 0x08)>>3);
		int ysize = ((size & 0x80)>>7);

		xsize++;
		ysize++;

		if (ysize==2) sy -= 16;

		code |= ((source[2] & 0x01) << 6);
		code |= ((source[2] & 0x08) << 4);

		if(flip_screen())
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = sx+16;
			sy = 250-sy;

			if (ysize==2) sy += 16;
		}

		for (xblock = 0; xblock<xsize; xblock++)
		{
			int xoffs=!flipx ? (xblock*8) : ((xsize-xblock-1)*8);
			int zoomx=((size&7)+1)<<13;
			int zoomy=((size&0x70)+0x10)<<9;
			int xblocksizeinpixels=(zoomx*16)>>16;
			int yblocksizeinpixels=(zoomy*16)>>16;

			for (yblock = 0; yblock<ysize; yblock++)
			{
				int yoffs=!flipy ? yblock : (ysize-yblock-1);
				int sxoffs=(16-xblocksizeinpixels)/2;
				int syoffs=(16-yblocksizeinpixels)/2;
				if (xblock) sxoffs+=xblocksizeinpixels;
				if (yblock) syoffs+=yblocksizeinpixels;

				gfx->zoom_transpen(bitmap,cliprect,
						code+yoffs+xoffs,
						color,
						flipx,flipy,
						sx+sxoffs,sy+syoffs,
						zoomx,zoomy,15);
			}
		}
		source -= 8;
	}

}

TILE_GET_INFO_MEMBER(flower_state::get_bg0_tile_info)
{
	int code = m_bg0ram[tile_index];
	int color = m_bg0ram[tile_index+0x100];
	/* Todo - may be tile flip bits? */

	SET_TILE_INFO_MEMBER(2, code, color>>4, 0);
}

TILE_GET_INFO_MEMBER(flower_state::get_bg1_tile_info)
{
	int code = m_bg1ram[tile_index];
	int color = m_bg1ram[tile_index+0x100];
	/* Todo - may be tile flip bits? */

	SET_TILE_INFO_MEMBER(2, code, color>>4, 0);
}

TILE_GET_INFO_MEMBER(flower_state::get_text_tile_info)
{
	int code = m_textram[tile_index];
	int color = m_textram[tile_index+0x400];
	/* Todo - may be tile flip bits? */

	SET_TILE_INFO_MEMBER(0, code, color>>2, 0);
}

void flower_state::video_start()
{
	m_bg0_tilemap        = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flower_state::get_bg0_tile_info),this), TILEMAP_SCAN_ROWS,16,16,16,16);
	m_bg1_tilemap        = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flower_state::get_bg1_tile_info),this), TILEMAP_SCAN_ROWS,16,16,16,16);
	m_text_tilemap       = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flower_state::get_text_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,32,32);
	m_text_right_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(flower_state::get_text_tile_info),this),TILEMAP_SCAN_COLS, 8, 8, 2,32);

	m_bg1_tilemap->set_transparent_pen(15);
	m_text_tilemap->set_transparent_pen(3);
	m_text_right_tilemap->set_transparent_pen(3);

	m_text_tilemap->set_scrolly(0, 16);
	m_text_right_tilemap->set_scrolly(0, 16);
}

UINT32 flower_state::screen_update_flower(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle myclip = cliprect;

	m_bg0_tilemap->set_scrolly(0, m_bg0_scroll[0]+16);
	m_bg1_tilemap->set_scrolly(0, m_bg1_scroll[0]+16);

	m_bg0_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_bg1_tilemap->draw(screen, bitmap, cliprect, 0,0);

	draw_sprites(bitmap,cliprect);

	if(flip_screen())
	{
		myclip.min_x = cliprect.min_x;
		myclip.max_x = cliprect.min_x + 15;
	}
	else
	{
		myclip.min_x = cliprect.max_x - 15;
		myclip.max_x = cliprect.max_x;
	}

	m_text_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_text_right_tilemap->draw(screen, bitmap, myclip, 0,0);
	return 0;
}

WRITE8_MEMBER(flower_state::flower_textram_w)
{
	m_textram[offset] = data;
	m_text_tilemap->mark_tile_dirty(offset);
	m_text_right_tilemap->mark_all_dirty();
}

WRITE8_MEMBER(flower_state::flower_bg0ram_w)
{
	m_bg0ram[offset] = data;
	m_bg0_tilemap->mark_tile_dirty(offset & 0x1ff);
}

WRITE8_MEMBER(flower_state::flower_bg1ram_w)
{
	m_bg1ram[offset] = data;
	m_bg1_tilemap->mark_tile_dirty(offset & 0x1ff);
}

WRITE8_MEMBER(flower_state::flower_flipscreen_w)
{
	flip_screen_set(data);
}
