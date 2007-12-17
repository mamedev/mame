/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static UINT8 gfx_bank, palette_bank;

static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mario Bros. has a 512x8 palette PROM; interstingly, bytes 0-255 contain an
  inverted palette, as other Nintendo games like Donkey Kong, while bytes
  256-511 contain a non inverted palette. This was probably done to allow
  connection to both the special Nintendo and a standard monitor.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor -- inverter  -- RED
        -- 470 ohm resistor -- inverter  -- RED
        -- 1  kohm resistor -- inverter  -- RED
        -- 220 ohm resistor -- inverter  -- GREEN
        -- 470 ohm resistor -- inverter  -- GREEN
        -- 1  kohm resistor -- inverter  -- GREEN
        -- 220 ohm resistor -- inverter  -- BLUE
  bit 0 -- 470 ohm resistor -- inverter  -- BLUE

***************************************************************************/
PALETTE_INIT( mario )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (*color_prom >> 5) & 1;
		bit1 = (*color_prom >> 6) & 1;
		bit2 = (*color_prom >> 7) & 1;
		r = 255 - (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);
		/* green component */
		bit0 = (*color_prom >> 2) & 1;
		bit1 = (*color_prom >> 3) & 1;
		bit2 = (*color_prom >> 4) & 1;
		g = 255 - (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);
		/* blue component */
		bit0 = (*color_prom >> 0) & 1;
		bit1 = (*color_prom >> 1) & 1;
		b = 255 - (0x55 * bit0 + 0xaa * bit1);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* characters use the same palette as sprites, however characters */
	/* use only colors 64-127 and 192-255. */
	for (i = 0;i < 8;i++)
	{
		COLOR(0,4*i) = 8*i + 64;
		COLOR(0,4*i+1) = 8*i+1 + 64;
		COLOR(0,4*i+2) = 8*i+2 + 64;
		COLOR(0,4*i+3) = 8*i+3 + 64;
	}
	for (i = 0;i < 8;i++)
	{
		COLOR(0,4*i+8*4) = 8*i + 192;
		COLOR(0,4*i+8*4+1) = 8*i+1 + 192;
		COLOR(0,4*i+8*4+2) = 8*i+2 + 192;
		COLOR(0,4*i+8*4+3) = 8*i+3 + 192;
	}

	/* sprites */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = i;
}

WRITE8_HANDLER( mario_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( mario_gfxbank_w )
{
	if (gfx_bank != (data & 0x01))
	{
		gfx_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( mario_palettebank_w )
{
	if (palette_bank != (data & 0x01))
	{
		palette_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( mario_scroll_w )
{
	tilemap_set_scrolly(bg_tilemap, 0, data + 17);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] + 256 * gfx_bank;
	int color = (videoram[tile_index] >> 5) + 8 * palette_bank;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( mario )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	state_save_register_global(gfx_bank);
	state_save_register_global(palette_bank);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		if (spriteram[offs])
		{
			drawgfx(bitmap,machine->gfx[1],
					spriteram[offs + 2],
					(spriteram[offs + 1] & 0x0f) + 16 * palette_bank,
					spriteram[offs + 1] & 0x80,spriteram[offs + 1] & 0x40,
					spriteram[offs + 3] - 8,240 - spriteram[offs] + 8,
					cliprect,TRANSPARENCY_PEN,0);
		}
	}
}

VIDEO_UPDATE( mario )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
