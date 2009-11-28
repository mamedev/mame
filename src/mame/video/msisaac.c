/*
*   Video Driver for Metal Soldier Isaac II (1985)
*/

#include "driver.h"

/*
*   variables
*/
UINT8 *msisaac_videoram;
UINT8 *msisaac_videoram2;

//static int textbank1;
static int bg2_textbank;

static tilemap *background, *background2, *foreground;


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	int tile_number = machine->generic.videoram.u8[tile_index];
	SET_TILE_INFO(	0,
			tile_number,
			0x10,
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	int tile_number = msisaac_videoram2[tile_index];

	/* graphics 0 or 1 */
	int gfx_b = (bg2_textbank>>3) & 1;

	SET_TILE_INFO(	gfx_b,
			tile_number,
			0x20,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int tile_number = msisaac_videoram[tile_index];
	SET_TILE_INFO(	1,
			0x100+tile_number,
			0x30,
			0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( msisaac )
{
	background  = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,     8,8,32,32);
	background2 = tilemap_create(machine, get_bg2_tile_info,tilemap_scan_rows,8,8,32,32);
	foreground  = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(background2,0);
	tilemap_set_transparent_pen(foreground,0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( msisaac_fg_scrolly_w )
{
	tilemap_set_scrolly( foreground, 0, data );
}

WRITE8_HANDLER( msisaac_fg_scrollx_w )
{
	tilemap_set_scrollx( foreground, 0, 9+data );
}

WRITE8_HANDLER( msisaac_bg2_scrolly_w )
{
	tilemap_set_scrolly( background2, 0, data );
}

WRITE8_HANDLER( msisaac_bg2_scrollx_w )
{
	tilemap_set_scrollx( background2, 0, 9+2+data );
}

WRITE8_HANDLER( msisaac_bg_scrolly_w )
{
	tilemap_set_scrolly( background, 0, data );
}

WRITE8_HANDLER( msisaac_bg_scrollx_w )
{
	tilemap_set_scrollx( background, 0, 9+4+data );
}


#ifdef UNUSED_FUNCTION
WRITE8_HANDLER( msisaac_textbank1_w )
{
	if( textbank1!=data )
	{
		textbank1 = data;
		tilemap_mark_all_tiles_dirty( foreground );
	}
}
#endif

WRITE8_HANDLER( msisaac_bg2_textbank_w )
{
	if( bg2_textbank != data )
	{
		bg2_textbank = data;
		tilemap_mark_all_tiles_dirty( background2 );

		//check if we are correct on this one
		if ((data!=8) && (data!=0))
		{
			logerror("bg2 control=%2x\n",data);
		}
	}
}

WRITE8_HANDLER( msisaac_bg_videoram_w )
{
	msisaac_videoram[offset]=data;
	tilemap_mark_tile_dirty(background,offset);
}

WRITE8_HANDLER( msisaac_bg2_videoram_w )
{
	msisaac_videoram2[offset]=data;
	tilemap_mark_tile_dirty(background2,offset);
}

WRITE8_HANDLER( msisaac_fg_videoram_w )
{
	space->machine->generic.videoram.u8[offset]=data;
	tilemap_mark_tile_dirty(foreground,offset);
}


/***************************************************************************

  Display refresh

***************************************************************************/
static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	const UINT8 *source = machine->generic.spriteram.u8+32*4-4;
	const UINT8 *finish = machine->generic.spriteram.u8; /* ? */

	while( source>=finish )
	{
		int sx 				= source[0];
		int sy 				= 240-source[1]-1;
		int attributes		= source[2];
		int sprite_number	= source[3];

		int color = (attributes>>4) & 0xf;
		int flipx = (attributes&0x1);
		int flipy = (attributes&0x2);

		gfx_element *gfx = machine->gfx[2];

		if (attributes&4)
		{
			//color = rand()&15;
			gfx = machine->gfx[3];
		}

		if (attributes&8)	/* double size sprite */
		{
			switch(attributes&3)
			{
			case 0: /* flipx==0 && flipy==0 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			case 1: /* flipx==1 && flipy==0 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			case 2: /* flipx==0 && flipy==1 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			case 3: /* flipx==1 && flipy==1 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			}
		}
		else
		{
			drawgfx_transpen(bitmap,cliprect,gfx,
				sprite_number,
				color,
				flipx,flipy,
				sx,sy,0 );
		}
		source -= 4;
	}
}

VIDEO_UPDATE( msisaac )
{
	tilemap_draw(bitmap,cliprect,background, 0,0);
	tilemap_draw(bitmap,cliprect,background2,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,foreground, 0,0);
	return 0;
}

