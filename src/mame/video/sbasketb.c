/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

//UINT8 *sbasketb_scroll;
UINT8 *sbasketb_palettebank;
UINT8 *sbasketb_spriteram_select;

static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Super Basketball has three 256x4 palette PROMs (one per gun) and two 256x4
  lookup table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( sbasketb )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
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
	/* color_prom now points to the beginning of the character lookup table */


	/* characters use colors 240-255 */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = (*(color_prom++) & 0x0f) + 240;

	/* sprites use colors 0-256 (?) in 16 banks */
	for (i = 0;i < TOTAL_COLORS(1)/16;i++)
	{
		int j;


		for (j = 0;j < 16;j++)
			COLOR(1,i + j * TOTAL_COLORS(1)/16) = (*color_prom & 0x0f) + 16 * j;

		color_prom++;
	}
}

WRITE8_HANDLER( sbasketb_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( sbasketb_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( sbasketb_flipscreen_w )
{
	if (flip_screen != data)
	{
		flip_screen_set(data);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( sbasketb_scroll_w )
{
	int col;

	for (col = 6; col < 32; col++)
	{
		tilemap_set_scrolly(bg_tilemap, col, data);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] + ((colorram[tile_index] & 0x20) << 3);
	int color = colorram[tile_index] & 0x0f;
	int flags = ((colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( sbasketb )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	tilemap_set_scroll_cols(bg_tilemap, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs = (*sbasketb_spriteram_select & 0x01) * 0x100;
	int i;

	for (i = 0; i < 64; i++, offs += 4)
	{
		int sx = spriteram[offs + 2];
		int sy = spriteram[offs + 3];

		if (sx || sy)
		{
			int code  =  spriteram[offs + 0] | ((spriteram[offs + 1] & 0x20) << 3);
			int color = (spriteram[offs + 1] & 0x0f) + 16 * *sbasketb_palettebank;
			int flipx =  spriteram[offs + 1] & 0x40;
			int flipy =  spriteram[offs + 1] & 0x80;

			if (flip_screen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx(bitmap,machine->gfx[1],
				code, color,
				flipx, flipy,
				sx, sy,
				cliprect,
				TRANSPARENCY_PEN, 0);
		}
	}
}

VIDEO_UPDATE( sbasketb )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
