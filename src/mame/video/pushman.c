#include "driver.h"

static tilemap *bg_tilemap,*tx_tilemap;
static UINT16 control[2];


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( background_scan_rows )
{
	/* logical (col,row) -> memory offset */
	return ((col & 0x7)) + ((7-(row & 0x7)) << 3) + ((col & 0x78) <<3) + ((0x38-(row&0x38))<<7);
}

static TILE_GET_INFO( get_back_tile_info )
{
	UINT8 *bgMap = memory_region(REGION_GFX4);
	int tile;

	tile=bgMap[tile_index<<1]+(bgMap[(tile_index<<1)+1]<<8);
	SET_TILE_INFO(
			2,
			(tile&0xff)|((tile&0x4000)>>6),
			(tile>>8)&0xf,
			(tile&0x2000)?TILE_FLIPX:0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int tile = videoram16[tile_index];
	SET_TILE_INFO(
			0,
			(tile&0xff)|((tile&0xc000)>>6)|((tile&0x2000)>>3),
			(tile>>8)&0xf,
			(tile&0x1000)?TILE_FLIPY:0);	/* not used? from Tiger Road */
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( pushman )
{
	bg_tilemap = tilemap_create(get_back_tile_info,background_scan_rows,TILEMAP_TYPE_PEN,     32,32,128,64);
	tx_tilemap = tilemap_create(get_text_tile_info,tilemap_scan_rows,   TILEMAP_TYPE_PEN, 8, 8, 32,32);

	tilemap_set_transparent_pen(tx_tilemap,3);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( pushman_scroll_w )
{
	COMBINE_DATA(&control[offset]);
}

WRITE16_HANDLER( pushman_videoram_w )
{
	COMBINE_DATA(&videoram16[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs,x,y,color,sprite,flipx,flipy;

	for (offs = 0x0800-4;offs >=0;offs -= 4)
	{
		/* Don't draw empty sprite table entries */
		x = spriteram16[offs+3]&0x1ff;
		if (x==0x180) continue;
		if (x>0xff) x=0-(0x200-x);
		y = 240-spriteram16[offs+2];
		color = ((spriteram16[offs+1]>>2)&0xf);
		sprite = spriteram16[offs]&0x7ff;
		/* ElSemi - Sprite flip info */
		flipx=spriteram16[offs+1]&2;
		flipy=spriteram16[offs+1]&1;	/* flip y untested */

		drawgfx(bitmap,machine->gfx[1],
				sprite,
                color,flipx,flipy,x,y,
				cliprect,TRANSPARENCY_PEN,15);
	}
}

VIDEO_UPDATE( pushman )
{
	/* Setup the tilemaps */
	tilemap_set_scrollx( bg_tilemap,0, control[0] );
	tilemap_set_scrolly( bg_tilemap,0, 0xf00-control[1] );

	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}
