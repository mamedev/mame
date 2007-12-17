/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static tilemap *background_layer,*text_layer;


static TILE_GET_INFO( get_back_tile_info )
{
	int tile = videoram16[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int tile = colorram16[tile_index];
	int color = (tile>>10);

	tile &= 0x3ff;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}


VIDEO_START( cabal )
{
	background_layer = tilemap_create(get_back_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,16,16);
	text_layer       = tilemap_create(get_text_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,  8,8,32,32);

	tilemap_set_transparent_pen(text_layer,3);
	tilemap_set_transparent_pen(background_layer,15);
}


/**************************************************************************/

WRITE16_HANDLER( cabal_flipscreen_w )
{
	if (ACCESSING_LSB)
	{
		int flip = (data & 0x20) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
		tilemap_set_flip(background_layer,flip);
		tilemap_set_flip(text_layer,flip);

		flip_screen_set(data & 0x20);
	}
}

WRITE16_HANDLER( cabal_background_videoram16_w )
{
	COMBINE_DATA(&videoram16[offset]);
	tilemap_mark_tile_dirty(background_layer,offset);
}

WRITE16_HANDLER( cabal_text_videoram16_w )
{
	COMBINE_DATA(&colorram16[offset]);
	tilemap_mark_tile_dirty(text_layer,offset);
}


/********************************************************************

    Cabal Spriteram
    ---------------

    +0   .......x ........  Sprite enable bit
    +0   ........ xxxxxxxx  Sprite Y coordinate
    +1   ..??.... ........  ??? unknown ???
    +1   ....xxxx xxxxxxxx  Sprite tile number
    +2   .xxxx... ........  Sprite color bank
    +2   .....x.. ........  Sprite flip x
    +2   .......x xxxxxxxx  Sprite X coordinate
    +3   (unused)

            -------E YYYYYYYY
            ----BBTT TTTTTTTT
            -CCCCF-X XXXXXXXX
            -------- --------

********************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs,data0,data1,data2;

	for( offs = spriteram_size/2 - 4; offs >= 0; offs -= 4 )
	{
		data0 = spriteram16[offs];
		data1 = spriteram16[offs+1];
		data2 = spriteram16[offs+2];

		if( data0 & 0x100 )
		{
			int tile_number = data1 & 0xfff;
			int color   = ( data2 & 0x7800 ) >> 11;
			int sy = ( data0 & 0xff );
			int sx = ( data2 & 0x1ff );
			int flipx = ( data2 & 0x0400 );
			int flipy = 0;

			if ( sx>256 )   sx -= 512;

			if (flip_screen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx( bitmap,machine->gfx[2],
				tile_number,
				color,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0xf );
		}
	}
}


VIDEO_UPDATE( cabal )
{
	tilemap_draw(bitmap,cliprect,background_layer,TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,text_layer,0,0);
	return 0;
}


