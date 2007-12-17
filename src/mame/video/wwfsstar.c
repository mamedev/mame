/*******************************************************************************
 WWF Superstars (C) 1989 Technos Japan  (video/wwfsstar.c)
********************************************************************************
 driver by David Haywood

 see (drivers/wwfsstar.c) for more notes
*******************************************************************************/

#include "driver.h"

UINT16 *wwfsstar_fg0_videoram, *wwfsstar_bg0_videoram;
extern int wwfsstar_scrollx, wwfsstar_scrolly;
static tilemap *fg0_tilemap, *bg0_tilemap;

/*******************************************************************************
 Write Handlers
********************************************************************************
 for writes to Video Ram
*******************************************************************************/

WRITE16_HANDLER( wwfsstar_fg0_videoram_w )
{
	COMBINE_DATA(&wwfsstar_fg0_videoram[offset]);
	tilemap_mark_tile_dirty(fg0_tilemap,offset/2);
}

WRITE16_HANDLER( wwfsstar_bg0_videoram_w )
{
	COMBINE_DATA(&wwfsstar_bg0_videoram[offset]);
	tilemap_mark_tile_dirty(bg0_tilemap,offset/2);
}

/*******************************************************************************
 Tilemap Related Functions
*******************************************************************************/

static TILE_GET_INFO( get_fg0_tile_info )
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
	tilebase =  &wwfsstar_fg0_videoram[tile_index*2];
	tileno =  (tilebase[1] & 0x00ff) | ((tilebase[0] & 0x000f) << 8);
	colbank = (tilebase[0] & 0x00f0) >> 4;
	SET_TILE_INFO(
			0,
			tileno,
			colbank,
			0);
}

static TILEMAP_MAPPER( bg0_scan )
{
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

static TILE_GET_INFO( get_bg0_tile_info )
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
	tilebase =  &wwfsstar_bg0_videoram[tile_index*2];
	tileno =  (tilebase[1] & 0x00ff) | ((tilebase[0] & 0x000f) << 8);
	colbank = (tilebase[0] & 0x0070) >> 4;
	flipx   = (tilebase[0] & 0x0080) >> 7;
	SET_TILE_INFO(
			2,
			tileno,
			colbank,
			flipx ? TILE_FLIPX : 0);
}

/*******************************************************************************
 Sprite Related Functions
********************************************************************************
 sprite colour marking could probably be improved..
*******************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
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

	const gfx_element *gfx = machine->gfx[1];
	UINT16 *source = spriteram16;
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

			if (flip_screen)
			{
				flipy = !flipy;
				flipx = !flipx;
				ypos=240-ypos;
				xpos=240-xpos;
			}

			for (count=0;count<chain;count++)
			{
				if (flip_screen)
				{
					if (!flipy)
					{
						drawgfx(bitmap,gfx,number+count,colourbank,flipx,flipy,xpos,ypos+16*count,cliprect,TRANSPARENCY_PEN,0);
					}
					else
					{
						drawgfx(bitmap,gfx,number+count,colourbank,flipx,flipy,xpos,ypos+(16*(chain-1))-(16*count),cliprect,TRANSPARENCY_PEN,0);
					}
				}
				else
				{
					if (!flipy)
					{
						drawgfx(bitmap,gfx,number+count,colourbank,flipx,flipy,xpos,ypos-(16*(chain-1))+(16*count),cliprect,TRANSPARENCY_PEN,0);
					}
					else
					{
						drawgfx(bitmap,gfx,number+count,colourbank,flipx,flipy,xpos,ypos-16*count,cliprect,TRANSPARENCY_PEN,0);
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


VIDEO_START( wwfsstar )
{
	fg0_tilemap = tilemap_create(get_fg0_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,32,32);
	tilemap_set_transparent_pen(fg0_tilemap,0);

	bg0_tilemap = tilemap_create(get_bg0_tile_info,bg0_scan,TILEMAP_TYPE_PEN, 16, 16,32,32);
	tilemap_set_transparent_pen(fg0_tilemap,0);
}

VIDEO_UPDATE( wwfsstar )
{
	tilemap_set_scrolly( bg0_tilemap, 0, wwfsstar_scrolly  );
	tilemap_set_scrollx( bg0_tilemap, 0, wwfsstar_scrollx  );

	tilemap_draw(bitmap,cliprect,bg0_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect );
	tilemap_draw(bitmap,cliprect,fg0_tilemap,0,0);

	return 0;
}
