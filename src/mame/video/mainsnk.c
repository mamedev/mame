#include "driver.h"

static UINT8 bg_color,  old_bg_color;
#define mainsnk_offset 8
static tilemap *me_fg_tilemap;
static tilemap *me_bg_tilemap;
UINT8 *mainsnk_fgram;
UINT8 *mainsnk_bgram;
static int me_gfx_ctrl;

WRITE8_HANDLER(mainsnk_c600_w)
{
	bg_color = data&0xf;
	me_gfx_ctrl=data;
	tilemap_mark_all_tiles_dirty (me_bg_tilemap);
	mame_printf_debug("canvas %04x\n",data&=0xf0);
}

static TILE_GET_INFO( get_me_fg_tile_info )
{
	int code = (mainsnk_fgram[tile_index]);

	SET_TILE_INFO(
			0,
			code,
			0x10,
			0);
}

static void stuff_palette( running_machine *machine, int source_index, int dest_index, int num_colors )
{



	UINT8 *color_prom = memory_region(REGION_PROMS) + source_index;
	int i;
	for( i=0; i<num_colors; i++ )
	{
		int bit0=0,bit1,bit2,bit3;
		int red, green, blue;

		bit0 = (color_prom[0x1000] >> 2) & 0x01; // ?
		bit1 = (color_prom[0x000] >> 1) & 0x01;
		bit2 = (color_prom[0x000] >> 2) & 0x01;
		bit3 = (color_prom[0x000] >> 3) & 0x01;
		red = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[0x1000] >> 1) & 0x01; // ?
		bit1 = (color_prom[0x800] >> 2) & 0x01;
		bit2 = (color_prom[0x800] >> 3) & 0x01;
		bit3 = (color_prom[0x000] >> 0) & 0x01;
		green = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[0x1000] >> 0) & 0x01; // ?
		bit1 = (color_prom[0x1000] >> 3) & 0x01; // ?
		bit2 = (color_prom[0x800] >> 0) & 0x01;
		bit3 = (color_prom[0x800] >> 1) & 0x01;
		blue = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color( machine, dest_index++, MAKE_RGB(red, green, blue) );
		color_prom++;
	}

}

static void update_palette( int type )
{
	if( bg_color!=old_bg_color )
	{
		stuff_palette( Machine, 256+16*(bg_color&0x7), (0x11-type)*16, 16 );
		old_bg_color = bg_color;
	}
}


WRITE8_HANDLER( mainsnk_fgram_w )
{
	mainsnk_fgram[offset] = data;
	tilemap_mark_tile_dirty(me_fg_tilemap,offset);
}


static TILE_GET_INFO( get_me_bg_tile_info )
{
	int code = (mainsnk_bgram[tile_index]);

	SET_TILE_INFO(
			0,
			code  + ((me_gfx_ctrl<<4)&0x700),
			0x10,
			0);
}


WRITE8_HANDLER( mainsnk_bgram_w )
{
	mainsnk_bgram[offset] = data;
	tilemap_mark_tile_dirty(me_bg_tilemap,offset);
}


VIDEO_START(mainsnk)
{
	old_bg_color = -1;
	stuff_palette( machine, 0, 0, 16*8 );
	stuff_palette( machine, 16*8*3, 16*8, 16*8 );
	me_fg_tilemap = tilemap_create(get_me_fg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,32, 32);
	tilemap_set_transparent_pen(me_fg_tilemap,15);
	me_bg_tilemap = tilemap_create(get_me_bg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,32, 32);
	tilemap_set_scrollx( me_fg_tilemap, 0, -mainsnk_offset );
	tilemap_set_scrollx( me_bg_tilemap, 0, -mainsnk_offset );
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int scrollx, int scrolly )
{
	const gfx_element *gfx = machine->gfx[1];
	const UINT8 *source, *finish;
	source =  spriteram;
	finish =  source + 0x64;

	while( source<finish )
	{
		int attributes = source[3];
		int tile_number = source[1];
		int sy = source[0];
		int sx = source[2];
		int color = attributes&0xf;
		if( sy>240 ) sy -= 256;

		tile_number |= attributes<<4 & 0x300;

		drawgfx( bitmap,gfx,
			tile_number,
			color,
			0,0,
			256-sx+mainsnk_offset,sy,
			cliprect,TRANSPARENCY_PEN,7);

		source+=4;
	}
}


static void draw_status(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect,int dx,int off )
{
	const UINT8 *base = mainsnk_fgram+off;
	const gfx_element *gfx = machine->gfx[0];
	int row;
	for( row=0; row<4; row++ )
	{
		int sy,sx = (row&1)*8;
		const UINT8 *source = base + (row&1)*32;
		if( row>1 )
		{
			sx+=256+16;
		}
		else
		{
			source+=30*32;
		}

		for( sy=0; sy<256; sy+=8 )
		{
			int tile_number = *source++;
			drawgfx( bitmap, gfx,
			    tile_number, tile_number>>5,
			    0,0,
			    sx+dx,sy,
			    cliprect,
			    TRANSPARENCY_NONE, 0xf );
		}
	}
}

VIDEO_UPDATE(mainsnk)
{
	rectangle myclip;
	myclip.min_x = cliprect->min_x+8;
	myclip.max_x = cliprect->max_x-8;
	myclip.min_y = cliprect->min_y;
	myclip.max_y = cliprect->max_y;
	tilemap_draw(bitmap,&myclip,me_bg_tilemap,0,0);
	draw_sprites(machine,bitmap,&myclip, 0,0 );
	tilemap_draw(bitmap,&myclip,me_fg_tilemap,0,0);
	draw_status(machine,bitmap,cliprect,0,0x400 );
	draw_status(machine,bitmap,cliprect,32*8,0x40 );
	update_palette(1);
	return 0;
}

VIDEO_UPDATE(canvas)
{
	rectangle myclip;
	myclip.min_x = cliprect->min_x+8;
	myclip.max_x = cliprect->max_x-8;
	myclip.min_y = cliprect->min_y;
	myclip.max_y = cliprect->max_y;
	tilemap_draw(bitmap,&myclip,me_bg_tilemap,0,0);
	draw_sprites(machine,bitmap,&myclip, 0,0 );
//  tilemap_draw(bitmap,&myclip,me_fg_tilemap,0,0);
//  draw_status(machine,bitmap,cliprect,0,0x400 );
//  draw_status(machine,bitmap,cliprect,32*8,0x40 );
	update_palette(1);
	return 0;
}
