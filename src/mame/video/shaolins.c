/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

extern UINT8 shaolins_nmi_enable;

static int palettebank;
static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Shao-lin's Road has three 256x4 palette PROMs (one per gun) and two 256x4
  lookup table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( shaolins )
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


	/* there are eight 32 colors palette banks; sprites use colors 0-15 and */
	/* characters 16-31 of each bank. */
	for (i = 0;i < TOTAL_COLORS(0)/8;i++)
	{
		int j;


		for (j = 0;j < 8;j++)
			COLOR(0,i + j * TOTAL_COLORS(0)/8) = (*color_prom & 0x0f) + 32 * j + 16;

		color_prom++;
	}

	for (i = 0;i < TOTAL_COLORS(1)/8;i++)
	{
		int j;


		for (j = 0;j < 8;j++)
		{
			/* preserve transparency */
			if ((*color_prom & 0x0f) == 0) COLOR(1,i + j * TOTAL_COLORS(1)/8) = 0;
			else COLOR(1,i + j * TOTAL_COLORS(1)/8) = (*color_prom & 0x0f) + 32 * j;
		}

		color_prom++;
	}
}

WRITE8_HANDLER( shaolins_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( shaolins_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( shaolins_palettebank_w )
{
	if (palettebank != (data & 0x07))
	{
		palettebank = data & 0x07;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( shaolins_scroll_w )
{
	int col;

	for (col = 4; col < 32; col++)
	{
		tilemap_set_scrolly(bg_tilemap, col, data + 1);
	}
}

WRITE8_HANDLER( shaolins_nmi_w )
{
	shaolins_nmi_enable = data;

	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0x40) << 2);
	int color = (attr & 0x0f) + 16 * palettebank;
	int flags = (attr & 0x20) ? TILE_FLIPY : 0;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( shaolins )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_scroll_cols(bg_tilemap, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size-32; offs >= 0; offs-=32 ) /* max 24 sprites */
	{
		if (spriteram[offs] && spriteram[offs + 6]) /* stop rogue sprites on high score screen */
		{
			int code = spriteram[offs + 8];
			int color = (spriteram[offs + 9] & 0x0f) + 16 * palettebank;
			int flipx = !(spriteram[offs + 9] & 0x40);
			int flipy = spriteram[offs + 9] & 0x80;
			int sx = 240 - spriteram[offs + 6];
			int sy = 248 - spriteram[offs + 4];

			if (flip_screen)
			{
				sx = 240 - sx;
				sy = 248 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx(bitmap, machine->gfx[1],
				code, color,
				flipx, flipy,
				sx, sy,
				cliprect,
				TRANSPARENCY_COLOR, 0);
				/* transparency_color, otherwise sprites in test mode are not visible */
		}
	}
}

VIDEO_UPDATE( shaolins )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
