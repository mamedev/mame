#include "driver.h"


UINT16 *ohmygod_videoram;

static int spritebank;
static tilemap *bg_tilemap;



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT16 code = ohmygod_videoram[2*tile_index+1];
	UINT16 attr = ohmygod_videoram[2*tile_index];
	SET_TILE_INFO(
			0,
			code,
			(attr & 0x0f00) >> 8,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ohmygod )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,64);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( ohmygod_videoram_w )
{
	COMBINE_DATA(&ohmygod_videoram[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset/2);
}

WRITE16_HANDLER( ohmygod_spritebank_w )
{
	if (ACCESSING_MSB)
		spritebank = data & 0x8000;
}

WRITE16_HANDLER( ohmygod_scrollx_w )
{
	static UINT16 scroll;
	COMBINE_DATA(&scroll);
	tilemap_set_scrollx(bg_tilemap,0,scroll - 0x81ec);
}

WRITE16_HANDLER( ohmygod_scrolly_w )
{
	static UINT16 scroll;
	COMBINE_DATA(&scroll);
	tilemap_set_scrolly(bg_tilemap,0,scroll - 0x81ef);
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < spriteram_size/4;offs += 4)
	{
		int sx,sy,code,color,flipx;
		UINT16 *sr;

		sr = spritebank ? (spriteram16+spriteram_size/4) : spriteram16;

		code = sr[offs+3] & 0x0fff;
		color = sr[offs+2] & 0x000f;
		sx = sr[offs+0] - 29;
		sy = sr[offs+1];
		if (sy >= 32768) sy -= 65536;
		flipx = sr[offs+3] & 0x8000;

		drawgfx(bitmap,machine->gfx[1],
				code,
				color,
				flipx,0,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
}

VIDEO_UPDATE( ohmygod )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine,bitmap,cliprect);
	return 0;
}
