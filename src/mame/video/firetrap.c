/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"



UINT8 *firetrap_bg1videoram;
UINT8 *firetrap_bg2videoram;
UINT8 *firetrap_fgvideoram;

static tilemap *fg_tilemap, *bg1_tilemap, *bg2_tilemap;



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Fire Trap has one 256x8 and one 256x4 palette PROMs.
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT( firetrap )
{
	int i;


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i + machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( get_fg_memory_offset )
{
	return (row ^ 0x1f) + (col << 5);
}

static TILEMAP_MAPPER( get_bg_memory_offset )
{
	return ((row & 0x0f) ^ 0x0f) | ((col & 0x0f) << 4) |
			/* hole at bit 8 */
			((row & 0x10) << 5) | ((col & 0x10) << 6);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code, color;

	code = firetrap_fgvideoram[tile_index];
	color = firetrap_fgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			code | ((color & 0x01) << 8),
			color >> 4,
			0);
}

INLINE void get_bg_tile_info(running_machine *machine, tile_data *tileinfo, int tile_index, UINT8 *bgvideoram, int gfx_region)
{
	int code, color;

	code = bgvideoram[tile_index];
	color = bgvideoram[tile_index + 0x100];
	SET_TILE_INFO(
			gfx_region,
			code + ((color & 0x03) << 8),
			(color & 0x30) >> 4,
			TILE_FLIPXY((color & 0x0c) >> 2));
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	get_bg_tile_info(machine, tileinfo, tile_index, firetrap_bg1videoram, 1);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	get_bg_tile_info(machine, tileinfo, tile_index, firetrap_bg2videoram, 2);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( firetrap )
{
	fg_tilemap  = tilemap_create(get_fg_tile_info, get_fg_memory_offset,TILEMAP_TYPE_PEN, 8, 8,32,32);
	bg1_tilemap = tilemap_create(get_bg1_tile_info,get_bg_memory_offset,TILEMAP_TYPE_PEN,16,16,32,32);
	bg2_tilemap = tilemap_create(get_bg2_tile_info,get_bg_memory_offset,TILEMAP_TYPE_PEN,     16,16,32,32);

	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(bg1_tilemap,0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( firetrap_fgvideoram_w )
{
	firetrap_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( firetrap_bg1videoram_w )
{
	firetrap_bg1videoram[offset] = data;
	tilemap_mark_tile_dirty(bg1_tilemap,offset & 0x6ff);
}

WRITE8_HANDLER( firetrap_bg2videoram_w )
{
	firetrap_bg2videoram[offset] = data;
	tilemap_mark_tile_dirty(bg2_tilemap,offset & 0x6ff);
}


WRITE8_HANDLER( firetrap_bg1_scrollx_w )
{
	static UINT8 scroll[2];

	scroll[offset] = data;
	tilemap_set_scrollx(bg1_tilemap,0,scroll[0] | (scroll[1] << 8));
}

WRITE8_HANDLER( firetrap_bg1_scrolly_w )
{
	static UINT8 scroll[2];

	scroll[offset] = data;
	tilemap_set_scrolly(bg1_tilemap,0,-(scroll[0] | (scroll[1] << 8)));
}

WRITE8_HANDLER( firetrap_bg2_scrollx_w )
{
	static UINT8 scroll[2];

	scroll[offset] = data;
	tilemap_set_scrollx(bg2_tilemap,0,scroll[0] | (scroll[1] << 8));
}

WRITE8_HANDLER( firetrap_bg2_scrolly_w )
{
	static UINT8 scroll[2];

	scroll[offset] = data;
	tilemap_set_scrolly(bg2_tilemap,0,-(scroll[0] | (scroll[1] << 8)));
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;


	for (offs = 0;offs < spriteram_size; offs += 4)
	{
		int sx,sy,flipx,flipy,code,color;


		/* the meaning of bit 3 of [offs] is unknown */

		sy = spriteram[offs];
		sx = spriteram[offs + 2];
		code = spriteram[offs + 3] + 4 * (spriteram[offs + 1] & 0xc0);
		color = ((spriteram[offs + 1] & 0x08) >> 2) | (spriteram[offs + 1] & 0x01);
		flipx = spriteram[offs + 1] & 0x04;
		flipy = spriteram[offs + 1] & 0x02;
		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (spriteram[offs + 1] & 0x10)	/* double width */
		{
			if (flip_screen) sy -= 16;

			drawgfx(bitmap,machine->gfx[3],
					code & ~1,
					color,
					flipx,flipy,
					sx,flipy ? sy : sy + 16,
					cliprect,TRANSPARENCY_PEN,0);
			drawgfx(bitmap,machine->gfx[3],
					code | 1,
					color,
					flipx,flipy,
					sx,flipy ? sy + 16 : sy,
					cliprect,TRANSPARENCY_PEN,0);

			/* redraw with wraparound */
			drawgfx(bitmap,machine->gfx[3],
					code & ~1,
					color,
					flipx,flipy,
					sx - 256,flipy ? sy : sy + 16,
					cliprect,TRANSPARENCY_PEN,0);
			drawgfx(bitmap,machine->gfx[3],
					code | 1,
					color,
					flipx,flipy,
					sx - 256,flipy ? sy + 16 : sy,
					cliprect,TRANSPARENCY_PEN,0);
		}
		else
		{
			drawgfx(bitmap,machine->gfx[3],
					code,
					color,
					flipx,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,0);

			/* redraw with wraparound */
			drawgfx(bitmap,machine->gfx[3],
					code,
					color,
					flipx,flipy,
					sx - 256,sy,
					cliprect,TRANSPARENCY_PEN,0);
		}
	}
}

VIDEO_UPDATE( firetrap )
{
	tilemap_draw(bitmap,cliprect,bg2_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,bg1_tilemap,0,0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	return 0;
}
