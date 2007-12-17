/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"



UINT8 *circusc_videoram,*circusc_colorram;
static tilemap *bg_tilemap;

UINT8 *circusc_spritebank;
UINT8 *circusc_scroll;



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Circus Charlie has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( circusc )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* color_prom now points to the beginning of the lookup table */

	/* sprites */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = *(color_prom++) & 0x0f;

	/* characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = (*(color_prom++) & 0x0f) + 0x10;
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT8 attr = circusc_colorram[tile_index];
	tileinfo->category = (attr & 0x10) >> 4;
	SET_TILE_INFO(
			0,
			circusc_videoram[tile_index] + ((attr & 0x20) << 3),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0xc0) >> 6));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( circusc )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_scroll_cols(bg_tilemap,32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( circusc_videoram_w )
{
	circusc_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE8_HANDLER( circusc_colorram_w )
{
	circusc_colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE8_HANDLER( circusc_flipscreen_w )
{
	flip_screen_set(data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;
	UINT8 *sr;


	if ((*circusc_spritebank & 0x01) != 0)
		sr = spriteram;
	else sr = spriteram_2;

	for (offs = 0; offs < spriteram_size;offs += 4)
	{
		int sx,sy,flipx,flipy;


		sx = sr[offs + 2];
		sy = sr[offs + 3];
		flipx = sr[offs + 1] & 0x40;
		flipy = sr[offs + 1] & 0x80;
		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


		drawgfx(bitmap,machine->gfx[1],
				sr[offs + 0] + 8 * (sr[offs + 1] & 0x20),
				sr[offs + 1] & 0x0f,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_COLOR,0);

	}
}

VIDEO_UPDATE( circusc )
{
	int i;

	for (i = 0;i < 10;i++)
		tilemap_set_scrolly(bg_tilemap,i,0);
	for (i = 10;i < 32;i++)
		tilemap_set_scrolly(bg_tilemap,i,*circusc_scroll);

	tilemap_draw(bitmap,cliprect,bg_tilemap,1,0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	return 0;
}
