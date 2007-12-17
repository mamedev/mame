/***************************************************************************

    Kyugo hardware games

***************************************************************************/

#include "driver.h"
#include "kyugo.h"


UINT8 *kyugo_fgvideoram;
UINT8 *kyugo_bgvideoram;
UINT8 *kyugo_bgattribram;
UINT8 *kyugo_spriteram_1;
UINT8 *kyugo_spriteram_2;


static UINT8 scroll_x_lo;
static UINT8 scroll_x_hi;
static UINT8 scroll_y;
static tilemap *fg_tilemap;
static tilemap *bg_tilemap;
static int bgpalbank,fgcolor;
static int flipscreen;
static const UINT8 *color_codes;


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = kyugo_fgvideoram[tile_index];
	SET_TILE_INFO(0,
				  code,
				  2*color_codes[code >> 3] + fgcolor,
				  0);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	int code = kyugo_bgvideoram[tile_index];
	int attr = kyugo_bgattribram[tile_index];
	SET_TILE_INFO(1,
				  code | ((attr & 0x03) << 8),
				  (attr >> 4) | (bgpalbank << 4),
				  TILE_FLIPYX((attr & 0x0c) >> 2));
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( kyugo )
{
	color_codes = memory_region(REGION_PROMS) + 0x300;


	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 64,32);
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,      8,8, 64,32);

	tilemap_set_transparent_pen(fg_tilemap,0);

	tilemap_set_scrolldx(fg_tilemap,   0, 224);
	tilemap_set_scrolldx(bg_tilemap, -32, 32);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( kyugo_fgvideoram_w )
{
	kyugo_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty( fg_tilemap, offset );
}


WRITE8_HANDLER( kyugo_bgvideoram_w )
{
	kyugo_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty( bg_tilemap, offset );
}


WRITE8_HANDLER( kyugo_bgattribram_w )
{
	kyugo_bgattribram[offset] = data;
	tilemap_mark_tile_dirty( bg_tilemap, offset );
}


READ8_HANDLER( kyugo_spriteram_2_r )
{
	// only the lower nibble is connected
	return kyugo_spriteram_2[offset] | 0xf0;
}


WRITE8_HANDLER( kyugo_scroll_x_lo_w )
{
	scroll_x_lo = data;
}


WRITE8_HANDLER( kyugo_gfxctrl_w )
{
	/* bit 0 is scroll MSB */
	scroll_x_hi = data & 0x01;

	/* bit 5 is front layer color (Son of Phoenix only) */
	if (fgcolor != ((data & 0x20) >> 5))
	{
		fgcolor = (data & 0x20) >> 5;

		tilemap_mark_all_tiles_dirty(fg_tilemap);
	}

	/* bit 6 is background palette bank */
	if (bgpalbank != ((data & 0x40) >> 6))
	{
		bgpalbank = (data & 0x40) >> 6;

		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	if (data & 0x9e)
		popmessage("%02x",data);
}


WRITE8_HANDLER( kyugo_scroll_y_w )
{
	scroll_y = data;
}


WRITE8_HANDLER( kyugo_flipscreen_w )
{
	if (flipscreen != (data & 0x01))
	{
		flipscreen = (data & 0x01);

		tilemap_set_flip(ALL_TILEMAPS, (flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY): 0));
	}
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	/* sprite information is scattered through memory */
	/* and uses a portion of the text layer memory (outside the visible area) */
	UINT8 *spriteram_area1 = &kyugo_spriteram_1[0x28];
	UINT8 *spriteram_area2 = &kyugo_spriteram_2[0x28];
	UINT8 *spriteram_area3 = &kyugo_fgvideoram[0x28];

	int n;

	for( n = 0; n < 12*2; n++ )
	{
		int offs,y,sy,sx,color;

		offs = 2*(n % 12) + 64*(n / 12);

		sx = spriteram_area3[offs+1] + 256 * (spriteram_area2[offs+1] & 1);
		if (sx > 320) sx -= 512;

		sy = 255 - spriteram_area1[offs];
		if (flipscreen) sy = 240 - sy;

		color = spriteram_area1[offs+1] & 0x1f;

		for (y = 0;y < 16;y++)
		{
			int code,attr,flipx,flipy;

			code = spriteram_area3[offs + 128*y];
			attr = spriteram_area2[offs + 128*y];

			code = code | ((attr & 0x01) << 9) | ((attr & 0x02) << 7);

			flipx =  attr & 0x08;
			flipy =  attr & 0x04;

			if (flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;
			}


			drawgfx( bitmap, machine->gfx[2],
					 code,
					 color,
					 flipx,flipy,
					 sx,flipscreen ? sy - 16*y : sy + 16*y,
					 cliprect,TRANSPARENCY_PEN, 0 );
		 }
	}
}


VIDEO_UPDATE( kyugo )
{
	if (flipscreen)
		tilemap_set_scrollx(bg_tilemap,0,-(scroll_x_lo + (scroll_x_hi*256)));
	else
		tilemap_set_scrollx(bg_tilemap,0,  scroll_x_lo + (scroll_x_hi*256));

	tilemap_set_scrolly(bg_tilemap,0,scroll_y);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
