/***************************************************************************

Functions to emulate the video hardware of the machine.

This hardware of the machine is similar with that of Mutant Night.
The difference between the two machines is that there are three BG
layers as against just one BG layer in Mutant Night.


Foreground RAM format ( Foreground RAM format is same as Mutant Night )
--------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ----       = bottom 8 bits of tile number
 ---- ----  xx-- ----       = top 2 bits of tile number
 ---- ----  --x- ----       = flip X
 ---- ----  ---x ----       = flip Y
 ---- ----  ---- xxxx       = color ( 00h - 0fh )


Background RAM format
--------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ----       = bottom 8 bits of tile number
 ---- ----  x--- ----       = bit 9 of tile number
 ---- ----  -x-- ----       = bit 8 of tile number
 ---- ----  --x- ----       = bit 10 of tile number
 ---- ----  ---x ----       = bit 11 of tile number (the most top bit)
 ---- ----  ---- xxxx       = color ( 00h - 1fh )


Sprite RAM format   ( Sprite format is same as Mutant Night )
--------------------------------------------------------------
 +0         +1         +2         +3         +4
 xxxx xxxx  ---- ----  ---- ----  ---- ----  ---- ----  = sprite Y position
 ---- ----  xxxx xxxx  ---- ----  ---- ----  ---- ----  = bottom 8 bits of sprite X position
 ---- ----  ---- ----  xx-- ----  ---- ----  ---- ----  = middle 2 bits of sprite number
 ---- ----  ---- ----  --x- ----  ---- ----  ---- ----  = flip X
 ---- ----  ---- ----  ---x ----  ---- ----  ---- ----  = flip Y
 ---- ----  ---- ----  ---- x---  ---- ----  ---- ----  = top bit of sprite number
 ---- ----  ---- ----  ---- -x--  ---- ----  ---- ----  = 0:normal size (16x16)  1:big size (32x32)
 ---- ----  ---- ----  ---- --x-  ---- ----  ---- ----  = sprite on / off
 ---- ----  ---- ----  ---- ---x  ---- ----  ---- ----  = top bit of sprite X position
 ---- ----  ---- ----  ---- ----  xxxx xxxx  ---- ----  = bottom 8 bits of sprite number
 ---- ----  ---- ----  ---- ----  ---- ----  xxxx xxxx  = color


Scroll RAM format (Omega Fighter)
--------------------------------------------------------------
    +0         +1
 X  ???? -xxx  xxxx xxxx        = scroll X (0 - 0x3ff)
 Y  ???? ---x  xxxx xxxx        = scroll Y (0 - 0x1ff)

Scroll RAM format (Atomic Robokid)
--------------------------------------------------------------
    +0         +1
 X  ???? ---x  xxxx xxxx        = scroll X (0 - 0x1ff)
 Y  ???? ---x  xxxx xxxx        = scroll Y (0 - 0x1ff)


Real screen resolution and virtual one
--------------------------------------------------------------
                Real        Virtual
Omega Fighter   256x192(H)  2048x512
Atomic Robokid  256x192(H)  512x512


***************************************************************************/

#include "driver.h"



/**************************************************************************
  Variables
**************************************************************************/

UINT8 *omegaf_fg_videoram;
static UINT8 *omegaf_bg0_videoram;
static UINT8 *omegaf_bg1_videoram;
static UINT8 *omegaf_bg2_videoram;
//size_t omegaf_fgvideoram_size;

static int omegaf_bg0_bank = 0;
static int omegaf_bg1_bank = 0;
static int omegaf_bg2_bank = 0;

UINT8 *omegaf_bg0_scroll_x;
UINT8 *omegaf_bg1_scroll_x;
UINT8 *omegaf_bg2_scroll_x;
UINT8 *omegaf_bg0_scroll_y;
UINT8 *omegaf_bg1_scroll_y;
UINT8 *omegaf_bg2_scroll_y;

static tilemap *fg_tilemap;
static tilemap *bg0_tilemap;
static tilemap *bg1_tilemap;
static tilemap *bg2_tilemap;

static int bg0_enabled = 1;
static int bg1_enabled = 1;
static int bg2_enabled = 1;

static mame_bitmap *bitmap_sp;	/* for sprite overdraw */
static int sprite_overdraw_enabled = 0;

static int scrollx_mask = 0x07ff;
static int bank_mask = 1;


/***************************************************************************
  Callbacks for the tilemap code
***************************************************************************/

static TILE_GET_INFO( get_bg0_tile_info )
{
	int color, tile, hi, lo;

	int attrib = ( (tile_index & 0x00f) | ( (tile_index & 0x070) << 5 ) |
	                                      ( (tile_index & 0xf80) >> 3 ) ) << 1;
	lo  = omegaf_bg0_videoram[ attrib ];
	hi  = omegaf_bg0_videoram[ attrib | 1 ];
	color = hi & 0x0f;
	tile = ( ((hi & 0x80) << 2) | ((hi & 0x40) << 2) |
	         ((hi & 0x20) << 5) | ((hi & 0x10) << 7) ) | lo;
	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
	tileinfo->category = 0;
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	int color, tile, hi, lo;

	int attrib = ( (tile_index & 0x00f) | ( (tile_index & 0x070) << 5 ) |
	                                      ( (tile_index & 0xf80) >> 3 ) ) << 1;
	lo  = omegaf_bg1_videoram[ attrib ];
	hi  = omegaf_bg1_videoram[ attrib | 1 ];
	color = hi & 0x0f;
	tile = ( ((hi & 0x80) << 2) | ((hi & 0x40) << 2) |
	         ((hi & 0x20) << 5) | ((hi & 0x10) << 7) ) | lo;
	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
	tileinfo->category = 0;
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	int color, tile, hi, lo;

	int attrib = ( (tile_index & 0x00f) | ( (tile_index & 0x070) << 5 ) |
	                                      ( (tile_index & 0xf80) >> 3 ) ) << 1;
	lo  = omegaf_bg2_videoram[ attrib ];
	hi  = omegaf_bg2_videoram[ attrib | 1 ];
	color = hi & 0x0f;
	tile = ( ((hi & 0x80) << 2) | ((hi & 0x40) << 2) |
	         ((hi & 0x20) << 5) | ((hi & 0x10) << 7) ) | lo;
	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
	tileinfo->category = 0;
}

static TILE_GET_INFO( robokid_get_bg0_tile_info )
{
	int color, tile, hi, lo;

	int attrib = ( (tile_index & 0x00f) | ( (tile_index & 0x010) << 5 ) |
	                                      ( (tile_index & 0x3e0) >> 1 ) ) << 1;
	lo  = omegaf_bg0_videoram[ attrib ];
	hi  = omegaf_bg0_videoram[ attrib | 1 ];
	color = hi & 0x0f;
	tile = ( ((hi & 0x80) << 2) | ((hi & 0x40) << 2) |
	         ((hi & 0x20) << 5) | ((hi & 0x10) << 7) ) | lo;
	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
	tileinfo->category = 0;
}

static TILE_GET_INFO( robokid_get_bg1_tile_info )
{
	int color, tile, hi, lo;

	int attrib = ( (tile_index & 0x00f) | ( (tile_index & 0x010) << 5 ) |
	                                      ( (tile_index & 0x3e0) >> 1 ) ) << 1;
	lo  = omegaf_bg1_videoram[ attrib ];
	hi  = omegaf_bg1_videoram[ attrib | 1 ];
	color = hi & 0x0f;
	tile = ( ((hi & 0x80) << 2) | ((hi & 0x40) << 2) |
	         ((hi & 0x20) << 5) | ((hi & 0x10) << 7) ) | lo;
	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
	tileinfo->category = 0;
}

static TILE_GET_INFO( robokid_get_bg2_tile_info )
{
	int color, tile, hi, lo;

	int attrib = ( (tile_index & 0x00f) | ( (tile_index & 0x010) << 5 ) |
	                                      ( (tile_index & 0x3e0) >> 1 ) ) << 1;
	lo  = omegaf_bg2_videoram[ attrib ];
	hi  = omegaf_bg2_videoram[ attrib | 1 ];
	color = hi & 0x0f;
	tile = ( ((hi & 0x80) << 2) | ((hi & 0x40) << 2) |
	         ((hi & 0x20) << 5) | ((hi & 0x10) << 7) ) | lo;
	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
	tileinfo->category = 0;
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int color, tile, hi, lo;

	lo  = omegaf_fg_videoram[ tile_index << 1 ];
	hi = omegaf_fg_videoram[ (tile_index << 1) + 1 ];
	color  = hi & 0x0f;
	tile = ( ((hi & 0x80) << 2) | ((hi & 0x40) << 2) |
	         ((hi & 0x20) << 5) | ((hi & 0x10) << 7) ) | lo;
	SET_TILE_INFO(
			5,
			tile,
			color,
			0);
	tileinfo->category = 0;
}


/***************************************************************************
  Initialize and destroy video hardware emulation
***************************************************************************/

static void videoram_alloc(running_machine *machine, int size)
{
	/* create video ram */
	omegaf_bg0_videoram = auto_malloc(size);
	memset( omegaf_bg0_videoram, 0x00, size );

	omegaf_bg1_videoram = auto_malloc(size);
	memset( omegaf_bg1_videoram, 0x00, size );

	omegaf_bg2_videoram = auto_malloc(size);
	memset( omegaf_bg2_videoram, 0x00, size );

	bitmap_sp = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
}

VIDEO_START( omegaf )
{
	scrollx_mask = 0x07ff;
	bank_mask = 7;

	videoram_alloc(machine, 0x2000);

	/*                           Info               Offset             Type                 w   h  col  row */
	fg_tilemap  = tilemap_create(get_fg_tile_info,  tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,  8,  32, 32);
	bg0_tilemap = tilemap_create(get_bg0_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 128, 32);
	bg1_tilemap = tilemap_create(get_bg1_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 128, 32);
	bg2_tilemap = tilemap_create(get_bg2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 128, 32);

	tilemap_set_transparent_pen( fg_tilemap,  15 );
	tilemap_set_transparent_pen( bg0_tilemap, 15 );
	tilemap_set_transparent_pen( bg1_tilemap, 15 );
	tilemap_set_transparent_pen( bg2_tilemap, 15 );
}

VIDEO_START( robokid )
{
	scrollx_mask = 0x01ff;
	bank_mask = 1;

	videoram_alloc(machine, 0x0800);

	/*                           Info               Offset             Type                         w   h  col  row */
	fg_tilemap  = tilemap_create(        get_fg_tile_info,  tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,  8,  32, 32);
	bg0_tilemap = tilemap_create(robokid_get_bg0_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,		16, 16, 32, 32);
	bg1_tilemap = tilemap_create(robokid_get_bg1_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 32, 32);
	bg2_tilemap = tilemap_create(robokid_get_bg2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16, 16, 32, 32);

	tilemap_set_transparent_pen( fg_tilemap,  15 );
	tilemap_set_transparent_pen( bg1_tilemap, 15 );
	tilemap_set_transparent_pen( bg2_tilemap, 15 );
}


/***************************************************************************
  Memory handler
***************************************************************************/

WRITE8_HANDLER( omegaf_bg0_bank_w )
{
	omegaf_bg0_bank = data & bank_mask;
}

WRITE8_HANDLER( omegaf_bg1_bank_w )
{
	omegaf_bg1_bank = data & bank_mask;
}

WRITE8_HANDLER( omegaf_bg2_bank_w )
{
	omegaf_bg2_bank = data & bank_mask;
}

READ8_HANDLER( omegaf_bg0_videoram_r )
{
	return omegaf_bg0_videoram[ (omegaf_bg0_bank << 10) | offset ];
}

READ8_HANDLER( omegaf_bg1_videoram_r )
{
	return omegaf_bg1_videoram[ (omegaf_bg1_bank << 10) | offset ];
}

READ8_HANDLER( omegaf_bg2_videoram_r )
{
	return omegaf_bg2_videoram[ (omegaf_bg2_bank << 10) | offset ];
}

WRITE8_HANDLER( omegaf_bg0_videoram_w )
{
	int address;
	int tile_index;

	address = (omegaf_bg0_bank << 10 ) | offset;
	omegaf_bg0_videoram[ address ] = data;
	tile_index = ( (address & 0x001e) >> 1 ) | ( (address & 0x1c00) >> 6 ) |
	             ( (address & 0x03e0) << 2 );
	tilemap_mark_tile_dirty( bg0_tilemap, tile_index );
}

WRITE8_HANDLER( omegaf_bg1_videoram_w )
{
	int address;
	int tile_index;

	address = (omegaf_bg1_bank << 10 ) | offset;
	omegaf_bg1_videoram[ address ] = data;
	tile_index = ( (address & 0x001e) >> 1 ) | ( (address & 0x1c00) >> 6 ) |
	             ( (address & 0x03e0) << 2 );
	tilemap_mark_tile_dirty( bg1_tilemap, tile_index );
}

WRITE8_HANDLER( omegaf_bg2_videoram_w )
{
	int address;
	int tile_index;

	address = (omegaf_bg2_bank << 10 ) | offset;
	omegaf_bg2_videoram[ address ] = data;
	tile_index = ( (address & 0x001e) >> 1 ) | ( (address & 0x1c00) >> 6 ) |
	             ( (address & 0x03e0) << 2 );
	tilemap_mark_tile_dirty( bg2_tilemap, tile_index );
}

WRITE8_HANDLER( robokid_bg0_videoram_w )
{
	int address;
	int tile_index;

	address = (omegaf_bg0_bank << 10 ) | offset;
	omegaf_bg0_videoram[ address ] = data;
	tile_index = ( (address & 0x001e) >> 1 ) | ( (address & 0x0400) >> 6 ) |
	               (address & 0x03e0);
	tilemap_mark_tile_dirty( bg0_tilemap, tile_index );
}

WRITE8_HANDLER( robokid_bg1_videoram_w )
{
	int address;
	int tile_index;

	address = (omegaf_bg1_bank << 10 ) | offset;
	omegaf_bg1_videoram[ address ] = data;
	tile_index = ( (address & 0x001e) >> 1 ) | ( (address & 0x0400) >> 6 ) |
	               (address & 0x03e0);
	tilemap_mark_tile_dirty( bg1_tilemap, tile_index );
}

WRITE8_HANDLER( robokid_bg2_videoram_w )
{
	int address;
	int tile_index;

	address = (omegaf_bg2_bank << 10 ) | offset;
	omegaf_bg2_videoram[ address ] = data;
	tile_index = ( (address & 0x001e) >> 1 ) | ( (address & 0x0400) >> 6 ) |
	               (address & 0x03e0);
	tilemap_mark_tile_dirty( bg2_tilemap, tile_index );
}

WRITE8_HANDLER( omegaf_bg0_scrollx_w )
{
	int scrollx;

	omegaf_bg0_scroll_x[offset] = data;

	scrollx = (omegaf_bg0_scroll_x[1] << 8) | omegaf_bg0_scroll_x[0];
	scrollx &= scrollx_mask;
	tilemap_set_scrollx( bg0_tilemap, 0, scrollx );
}

WRITE8_HANDLER( omegaf_bg0_scrolly_w )
{
	int scrolly;

	omegaf_bg0_scroll_y[offset] = data;

	scrolly = (omegaf_bg0_scroll_y[1] << 8) | omegaf_bg0_scroll_y[0];
	scrolly &= 0x01ff;
	tilemap_set_scrolly( bg0_tilemap, 0, scrolly );
}

WRITE8_HANDLER( omegaf_bg1_scrollx_w )
{
	int scrollx;

	omegaf_bg1_scroll_x[offset] = data;

	scrollx = (omegaf_bg1_scroll_x[1] << 8) | omegaf_bg1_scroll_x[0];
	scrollx &= scrollx_mask;
	tilemap_set_scrollx( bg1_tilemap, 0, scrollx );
}

WRITE8_HANDLER( omegaf_bg1_scrolly_w )
{
	int scrolly;

	omegaf_bg1_scroll_y[offset] = data;

	scrolly = (omegaf_bg1_scroll_y[1] << 8) | omegaf_bg1_scroll_y[0];
	scrolly &= 0x01ff;
	tilemap_set_scrolly( bg1_tilemap, 0, scrolly );
}

WRITE8_HANDLER( omegaf_bg2_scrollx_w )
{
	int scrollx;

	omegaf_bg2_scroll_x[offset] = data;

	scrollx = (omegaf_bg2_scroll_x[1] << 8) | omegaf_bg2_scroll_x[0];
	scrollx &= scrollx_mask;
	tilemap_set_scrollx( bg2_tilemap, 0, scrollx );
}

WRITE8_HANDLER( omegaf_bg2_scrolly_w )
{
	int scrolly;

	omegaf_bg2_scroll_y[offset] = data;

	scrolly = (omegaf_bg2_scroll_y[1] << 8) | omegaf_bg2_scroll_y[0];
	scrolly &= 0x01ff;
	tilemap_set_scrolly( bg2_tilemap, 0, scrolly );
}

WRITE8_HANDLER( omegaf_fgvideoram_w )
{
	omegaf_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset >> 1);
}

WRITE8_HANDLER( omegaf_bg0_enabled_w )
{
	if (bg0_enabled != data)
		bg0_enabled = data;
}

WRITE8_HANDLER( omegaf_bg1_enabled_w )
{
	if (bg1_enabled != data)
		bg1_enabled = data;
}

WRITE8_HANDLER( omegaf_bg2_enabled_w )
{
	if (bg2_enabled != data)
		bg2_enabled = data;
}

WRITE8_HANDLER( omegaf_sprite_overdraw_w )
{
	logerror( "sprite overdraw flag : %02x\n", data );
	if (sprite_overdraw_enabled != (data & 1))
	{
		sprite_overdraw_enabled = data & 1;
		fillbitmap(bitmap_sp, 15, &Machine->screen[0].visarea);
	}
}

WRITE8_HANDLER( omegaf_flipscreen_w )
{
	flip_screen_set(data & 0x80);
}

/***************************************************************************
  Screen refresh
***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	/* Draw the sprites */
	for (offs = 11 ;offs < spriteram_size; offs += 16)
	{
		int sx, sy, tile, color, flipx, flipy, big;

		if (spriteram[offs + 2] & 2)
		{
			sx = spriteram[offs + 1];
			sy = spriteram[offs];
			if (spriteram[offs + 2] & 1)
				sx -= 256;
			tile = spriteram[offs + 3] |
					((spriteram[offs + 2] & 0xc0) << 2) |
					((spriteram[offs + 2] & 0x08) << 7);

			big  = spriteram[offs + 2] & 4;
			if (big)
				tile >>= 2;
			flipx = spriteram[offs + 2] & 0x10;
			flipy = spriteram[offs + 2] & 0x20;
			color = spriteram[offs + 4] & 0x0f;

			if(sprite_overdraw_enabled && (color & 8))
			{
				/* "static" sprites */
				drawgfx(bitmap_sp,machine->gfx[(big) ? 4 : 3],
						tile,
						color,
						flipx,flipy,
						sx,sy,
						0,
						TRANSPARENCY_PEN, 15);
			}
			else
			{
				drawgfx(bitmap,machine->gfx[(big) ? 4 : 3],
						tile,
						color,
						flipx,flipy,
						sx,sy,
						cliprect,
						TRANSPARENCY_PEN, 15);

				/* all the "normal" sprites clear the "static" ones */
				if(sprite_overdraw_enabled)
				{
					int x,y,offset = 0;
					const gfx_element *gfx = machine->gfx[(big) ? 4 : 3];
					UINT8 *srcgfx = gfx->gfxdata + tile * gfx->char_modulo;

					for(y = 0; y < gfx->height; y++)
					{
						for(x = 0; x < gfx->width; x++)
						{
							if(srcgfx[offset] != 15)
							{
								*BITMAP_ADDR16(bitmap_sp, sy + y, sx + x) = 15;
							}

							offset++;
						}
					}
				}
			}
		}
	}

	if(sprite_overdraw_enabled)
		copybitmap(bitmap, bitmap_sp, 0, 0, 0, 0, cliprect, TRANSPARENCY_PEN, 15);
}

VIDEO_UPDATE( omegaf )
{
	fillbitmap(bitmap,machine->pens[0],cliprect);

	if (bg0_enabled)	tilemap_draw(bitmap,cliprect, bg0_tilemap, 0, 0);
	if (bg1_enabled)	tilemap_draw(bitmap,cliprect, bg1_tilemap, 0, 0);
	if (bg2_enabled)	tilemap_draw(bitmap,cliprect, bg2_tilemap, 0, 0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect, fg_tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( robokid )
{
	fillbitmap(bitmap,machine->pens[0],cliprect);

	if (bg0_enabled)	tilemap_draw(bitmap,cliprect, bg0_tilemap, 0, 0);
	if (bg1_enabled)	tilemap_draw(bitmap,cliprect, bg1_tilemap, 0, 0);
	draw_sprites(machine,bitmap,cliprect);
	if (bg2_enabled)	tilemap_draw(bitmap,cliprect, bg2_tilemap, 0, 0);
	tilemap_draw(bitmap,cliprect, fg_tilemap, 0, 0);
	return 0;
}
