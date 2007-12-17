/**
 * @file video/djboy.c
 *
 * video hardware for DJ Boy
 */
#include "driver.h"
#include "kan_pand.h"

static UINT8 djboy_videoreg, djboy_scrollx, djboy_scrolly;
static tilemap *background;

void djboy_set_videoreg( UINT8 data )
{
	djboy_videoreg = data;
}

WRITE8_HANDLER( djboy_scrollx_w )
{
	djboy_scrollx = data;
}

WRITE8_HANDLER( djboy_scrolly_w )
{
	djboy_scrolly = data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 attr = videoram[tile_index + 0x800];
	int code = videoram[tile_index] + (attr&0xf)*256;
	int color = attr>>4;
	if( color&8 )
	{
		code |= 0x1000;
	}
	SET_TILE_INFO(1, code, color, 0);	/* no flip */
}

WRITE8_HANDLER( djboy_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty( background, offset & 0x7ff);
}

VIDEO_START( djboy )
{
	background = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,64,32);
	pandora_start(0,0,0);
}

WRITE8_HANDLER( djboy_paletteram_w )
{
	int val;

	paletteram[offset] = data;
	offset &= ~1;
	val = (paletteram[offset]<<8) | paletteram[offset+1];

	palette_set_color_rgb(Machine,offset/2,pal4bit(val >> 8),pal4bit(val >> 4),pal4bit(val >> 0));
}

VIDEO_UPDATE( djboy )
{
	/**
     * xx------ msb x
     * --x----- msb y
     * ---x---- flipscreen?
     * ----xxxx ROM bank
     */
	int scroll;
	scroll = djboy_scrollx | ((djboy_videoreg&0xc0)<<2);
	tilemap_set_scrollx( background, 0, scroll-0x391 );
	scroll = djboy_scrolly | ((djboy_videoreg&0x20)<<3);
	tilemap_set_scrolly( background, 0, scroll );
	tilemap_draw( bitmap, cliprect,background,0,0 );
	pandora_update(machine,bitmap,cliprect);
	return 0;
}

VIDEO_EOF( djboy )
{
	pandora_eof(machine);
}
