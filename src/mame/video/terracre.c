/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static UINT16 xscroll;
static UINT16 yscroll;
static tilemap *background, *foreground;
static const UINT8 *spritepalettebank;

UINT16 *amazon_videoram;

static
TILE_GET_INFO( get_bg_tile_info )
{
	/* xxxx.----.----.----
     * ----.xx--.----.----
     * ----.--xx.xxxx.xxxx */
	unsigned data = amazon_videoram[tile_index];
	unsigned color = data>>11;
	SET_TILE_INFO( 1,data&0x3ff,color,0 );
}

static
TILE_GET_INFO( get_fg_tile_info )
{
	int data = videoram16[tile_index];
	SET_TILE_INFO( 0,data&0xff,0,0 );
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	const gfx_element *pGfx = machine->gfx[2];
	const UINT16 *pSource = spriteram16;
	int i;
	int transparent_pen;

	if( pGfx->total_elements > 0x200 )
	{ /* HORE HORE Kid */
		transparent_pen = 0xf;
	}
	else
	{
		transparent_pen = 0x0;
	}
	for( i=0; i<0x200; i+=8 )
	{
		int tile = pSource[1]&0xff;
		int attrs = pSource[2];
		int flipx = attrs&0x04;
		int flipy = attrs&0x08;
		int color = (attrs&0xf0)>>4;
		int sx = (pSource[3] & 0xff) - 0x80 + 256 * (attrs & 1);
		int sy = 240 - (pSource[0] & 0xff);

		if( transparent_pen )
		{
			int bank;

			if( attrs&0x02 ) tile |= 0x200;
			if( attrs&0x10 ) tile |= 0x100;

			bank = (tile&0xfc)>>1;
			if( tile&0x200 ) bank |= 0x80;
			if( tile&0x100 ) bank |= 0x01;

			color &= 0xe;
			color += 16*(spritepalettebank[bank]&0xf);
		}
		else
		{
			if( attrs&0x02 ) tile|= 0x100;
			color += 16 * (spritepalettebank[(tile>>1)&0xff] & 0x0f);
		}

		if (flip_screen)
		{
				sx=240-sx;
				sy=240-sy;
				flipx = !flipx;
				flipy = !flipy;
		}

		drawgfx(
			bitmap,pGfx,tile, color,flipx,flipy,sx,sy,cliprect,TRANSPARENCY_PEN,transparent_pen );

		pSource += 4;
	}
}

PALETTE_INIT( amazon )
{
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])
	int i;
	for( i = 0; i<machine->drv->total_colors; i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->drv->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[2*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[2*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[2*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[2*machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	color_prom += 2*machine->drv->total_colors;
	/* color_prom now points to the beginning of the lookup table */


	/* characters use colors 0-15 */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = i;

	/* background tiles use colors 192-255 in four banks */
	/* the bottom two bits of the color code select the palette bank for */
	/* pens 0-7; the top two bits for pens 8-15. */
	for (i = 0;i < TOTAL_COLORS(1);i++)
	{
		if (i & 8) COLOR(1,i) = 192 + (i & 0x0f) + ((i & 0xc0) >> 2);
		else COLOR(1,i) = 192 + (i & 0x0f) + ((i & 0x30) >> 0);
	}

	/* sprites use colors 128-191 in four banks */
	/* The lookup table tells which colors to pick from the selected bank */
	/* the bank is selected by another PROM and depends on the top 8 bits of */
	/* the sprite code. The PROM selects the bank *separately* for pens 0-7 and */
	/* 8-15 (like for tiles). */
	for (i = 0;i < TOTAL_COLORS(2)/16;i++)
	{
		int j;

		for (j = 0;j < 16;j++)
		{
			if (i & 8)
				COLOR(2,i + j * (TOTAL_COLORS(2)/16)) = 128 + ((j & 0x0c) << 2) + (*color_prom & 0x0f);
			else
				COLOR(2,i + j * (TOTAL_COLORS(2)/16)) = 128 + ((j & 0x03) << 4) + (*color_prom & 0x0f);
		}

		color_prom++;
	}

	/* color_prom now points to the beginning of the sprite palette bank table */
	spritepalettebank = color_prom;	/* we'll need it at run time */
}

WRITE16_HANDLER( amazon_background_w )
{
	COMBINE_DATA( &amazon_videoram[offset] );
	tilemap_mark_tile_dirty( background, offset );
}

WRITE16_HANDLER( amazon_foreground_w )
{
	COMBINE_DATA( &videoram16[offset] );
	tilemap_mark_tile_dirty( foreground, offset );
}

WRITE16_HANDLER( amazon_flipscreen_w )
{
	if( ACCESSING_LSB )
	{
		coin_counter_w( 0, data&0x01 );
		coin_counter_w( 1, (data&0x02)>>1 );
		flip_screen_set(data&0x04);
	}
}

WRITE16_HANDLER( amazon_scrolly_w )
{
	COMBINE_DATA(&yscroll);
	tilemap_set_scrolly(background,0,yscroll);
}

WRITE16_HANDLER( amazon_scrollx_w )
{
	COMBINE_DATA(&xscroll);
	tilemap_set_scrollx(background,0,xscroll);
}

VIDEO_START( amazon )
{
	background = tilemap_create(get_bg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,16,16,64,32);
	foreground = tilemap_create(get_fg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,64,32);
		tilemap_set_transparent_pen(foreground,0xf);

	/* register for saving */
	state_save_register_global(xscroll);
	state_save_register_global(yscroll);
}

VIDEO_UPDATE( amazon )
{
	if( xscroll&0x2000 )
	{
		fillbitmap( bitmap,get_black_pen(machine),cliprect );
	}
	else
	{
		tilemap_draw( bitmap,cliprect, background, 0, 0 );
	}
	draw_sprites( machine, bitmap,cliprect );
	tilemap_draw( bitmap,cliprect, foreground, 0, 0 );
	return 0;
}
