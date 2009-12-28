/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static UINT16 xscroll;
static UINT16 yscroll;
static tilemap_t *background, *foreground;

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
	int data = machine->generic.videoram.u16[tile_index];
	SET_TILE_INFO( 0,data&0xff,0,0 );
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	const UINT8 *spritepalettebank = memory_region(machine, "user1");
	const gfx_element *pGfx = machine->gfx[2];
	const UINT16 *pSource = machine->generic.spriteram.u16;
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

		if (flip_screen_get(machine))
		{
				sx=240-sx;
				sy=240-sy;
				flipx = !flipx;
				flipy = !flipy;
		}

		drawgfx_transpen(
			bitmap,cliprect,pGfx,tile, color,flipx,flipy,sx,sy,transparent_pen );

		pSource += 4;
	}
}

PALETTE_INIT( amazon )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters use colors 0-0x0f */
	for (i = 0; i < 0x10; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* background tiles use colors 0xc0-0xff in four banks */
	/* the bottom two bits of the color code select the palette bank for */
	/* pens 0-7; the top two bits for pens 8-0x0f. */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry;

		if (i & 0x08)
			ctabentry = 0xc0 | (i & 0x0f) | ((i & 0xc0) >> 2);
		else
			ctabentry = 0xc0 | (i & 0x0f) | ((i & 0x30) >> 0);

		colortable_entry_set_value(machine->colortable, 0x10 + i, ctabentry);
	}

	/* sprites use colors 128-191 in four banks */
	/* The lookup table tells which colors to pick from the selected bank */
	/* the bank is selected by another PROM and depends on the top 8 bits of */
	/* the sprite code. The PROM selects the bank *separately* for pens 0-7 and */
	/* 8-15 (like for tiles). */
	for (i = 0; i < 0x1000; i++)
	{
		UINT8 ctabentry;
		int i_swapped = ((i & 0x0f) << 8) | ((i & 0xff0) >> 4);

		if (i & 0x80)
			ctabentry = 0x80 | ((i & 0x0c) << 2) | (color_prom[i >> 4] & 0x0f);
		else
			ctabentry = 0x80 | ((i & 0x03) << 4) | (color_prom[i >> 4] & 0x0f);

		colortable_entry_set_value(machine->colortable, 0x110 + i_swapped, ctabentry);
	}
}

WRITE16_HANDLER( amazon_background_w )
{
	COMBINE_DATA( &amazon_videoram[offset] );
	tilemap_mark_tile_dirty( background, offset );
}

WRITE16_HANDLER( amazon_foreground_w )
{
	COMBINE_DATA( &space->machine->generic.videoram.u16[offset] );
	tilemap_mark_tile_dirty( foreground, offset );
}

WRITE16_HANDLER( amazon_flipscreen_w )
{
	if( ACCESSING_BITS_0_7 )
	{
		coin_counter_w( space->machine, 0, data&0x01 );
		coin_counter_w( space->machine, 1, (data&0x02)>>1 );
		flip_screen_set(space->machine, data&0x04);
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
	background = tilemap_create(machine, get_bg_tile_info,tilemap_scan_cols,16,16,64,32);
	foreground = tilemap_create(machine, get_fg_tile_info,tilemap_scan_cols,8,8,64,32);
	tilemap_set_transparent_pen(foreground,0xf);

	/* register for saving */
	state_save_register_global(machine, xscroll);
	state_save_register_global(machine, yscroll);
}

VIDEO_UPDATE( amazon )
{
	if( xscroll&0x2000 )
		bitmap_fill( bitmap,cliprect ,get_black_pen(screen->machine));
	else
		tilemap_draw( bitmap,cliprect, background, 0, 0 );

	draw_sprites(screen->machine, bitmap,cliprect );
	tilemap_draw( bitmap,cliprect, foreground, 0, 0 );
	return 0;
}
