// license:BSD-3-Clause
// copyright-holders:David Haywood,Stephane Humbert
/*******************************************************************************
 WWF Superstars (C) 1989 Technos Japan  (video/wwfsstar.c)
********************************************************************************
 driver by David Haywood

 see (drivers/wwfsstar.c) for more notes
*******************************************************************************/

#include "emu.h"
#include "includes/wwfsstar.h"

/*******************************************************************************
 Write Handlers
********************************************************************************
 for writes to Video Ram
*******************************************************************************/

WRITE16_MEMBER(wwfsstar_state::fg0_videoram_w)
{
	COMBINE_DATA(&m_fg0_videoram[offset]);
	m_fg0_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(wwfsstar_state::bg0_videoram_w)
{
	COMBINE_DATA(&m_bg0_videoram[offset]);
	m_bg0_tilemap->mark_tile_dirty(offset/2);
}

/*******************************************************************************
 Tilemap Related Functions
*******************************************************************************/

TILE_GET_INFO_MEMBER(wwfsstar_state::get_fg0_tile_info)
{
	/*- FG0 RAM Format -**

	  0x1000 sized region (4096 bytes)

	  32x32 tilemap, 4 bytes per tile

	  ---- ----  CCCC TTTT  ---- ----  TTTT TTTT

	  C = Colour Bank (0-15)
	  T = Tile Number (0 - 4095)

	  other bits unknown / unused

	**- End of Comments -*/

	UINT16 *tilebase;
	int tileno;
	int colbank;

	tilebase =  &m_fg0_videoram[tile_index*2];
	tileno =  (tilebase[1] & 0x00ff) | ((tilebase[0] & 0x000f) << 8);
	colbank = (tilebase[0] & 0x00f0) >> 4;
	SET_TILE_INFO_MEMBER(0,
			tileno,
			colbank,
			0);
}

TILEMAP_MAPPER_MEMBER(wwfsstar_state::bg0_scan)
{
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

TILE_GET_INFO_MEMBER(wwfsstar_state::get_bg0_tile_info)
{
	/*- BG0 RAM Format -**

	  0x1000 sized region (4096 bytes)

	  32x32 tilemap, 4 bytes per tile

	  ---- ----  FCCC TTTT  ---- ----  TTTT TTTT

	  C = Colour Bank (0-7)
	  T = Tile Number (0 - 4095)
	  F = FlipX

	  other bits unknown / unused

	**- End of Comments -*/

	UINT16 *tilebase;
	int tileno, colbank, flipx;

	tilebase =  &m_bg0_videoram[tile_index*2];
	tileno =  (tilebase[1] & 0x00ff) | ((tilebase[0] & 0x000f) << 8);
	colbank = (tilebase[0] & 0x0070) >> 4;
	flipx   = (tilebase[0] & 0x0080) >> 7;
	SET_TILE_INFO_MEMBER(2,
			tileno,
			colbank,
			flipx ? TILE_FLIPX : 0);
}

/*******************************************************************************
 Sprite Related Functions
********************************************************************************
 sprite colour marking could probably be improved..
*******************************************************************************/

void wwfsstar_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/*- SPR RAM Format -**

	  0x3FF sized region (1024 bytes)

	  10 bytes per sprite

	  ---- ---- yyyy yyyy ---- ---- CCCC XYLE ---- ---- fFNN NNNN ---- ---- nnnn nnnn ---- ---- xxxx xxxx

	  Yy = sprite Y Position
	  Xx = sprite X Position
	  C  = colour bank
	  f  = flip Y
	  F  = flip X
	  L  = chain sprite (32x16)
	  E  = sprite enable
	  Nn = Sprite Number

	  other bits unused

	**- End of Comments -*/

	gfx_element *gfx = m_gfxdecode->gfx(1);
	UINT16 *source = m_spriteram;
	UINT16 *finish = source + 0x3ff/2;

	while (source < finish)
	{
		int xpos, ypos, colourbank, flipx, flipy, chain, enable, number, count;

		enable = (source [1] & 0x0001);

		if (enable)
		{
			ypos = ((source [0] & 0x00ff) | ((source [1] & 0x0004) << 6) );
			ypos = (((256 - ypos) & 0x1ff) - 16) ;
			xpos = ((source [4] & 0x00ff) | ((source [1] & 0x0008) << 5) );
			xpos = (((256 - xpos) & 0x1ff) - 16);
			flipx = (source [2] & 0x0080 ) >> 7;
			flipy = (source [2] & 0x0040 ) >> 6;
			chain = (source [1] & 0x0002 ) >> 1;
			chain += 1;
			number = (source [3] & 0x00ff) | ((source [2] & 0x003f) << 8);
			colourbank = (source [1] & 0x00f0) >> 4;

			number &= ~(chain - 1);

			if (flip_screen())
			{
				flipy = !flipy;
				flipx = !flipx;
				ypos=240-ypos;
				xpos=240-xpos;
			}

			for (count=0;count<chain;count++)
			{
				if (flip_screen())
				{
					if (!flipy)
					{
						gfx->transpen(bitmap,cliprect,number+count,colourbank,flipx,flipy,xpos,ypos+16*count,0);
					}
					else
					{
						gfx->transpen(bitmap,cliprect,number+count,colourbank,flipx,flipy,xpos,ypos+(16*(chain-1))-(16*count),0);
					}
				}
				else
				{
					if (!flipy)
					{
						gfx->transpen(bitmap,cliprect,number+count,colourbank,flipx,flipy,xpos,ypos-(16*(chain-1))+(16*count),0);
					}
					else
					{
						gfx->transpen(bitmap,cliprect,number+count,colourbank,flipx,flipy,xpos,ypos-16*count,0);
					}
				}
			}
		}

		source+=5;
	}
}

/*******************************************************************************
 Video Start and Refresh Functions
********************************************************************************
 Drawing Order is simple
 BG0 - Back
 SPR - Middle
 FG0 - Front
*******************************************************************************/


void wwfsstar_state::video_start()
{
	m_fg0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wwfsstar_state::get_fg0_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,32,32);
	m_fg0_tilemap->set_transparent_pen(0);

	m_bg0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wwfsstar_state::get_bg0_tile_info),this),tilemap_mapper_delegate(FUNC(wwfsstar_state::bg0_scan),this), 16, 16,32,32);
	m_fg0_tilemap->set_transparent_pen(0);

	save_item(NAME(m_vblank));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
}

UINT32 wwfsstar_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg0_tilemap->set_scrolly(0, m_scrolly  );
	m_bg0_tilemap->set_scrollx(0, m_scrollx  );

	m_bg0_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect );
	m_fg0_tilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}
