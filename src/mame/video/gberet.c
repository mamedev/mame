#include "driver.h"


UINT8 *gberet_scrollram;

static int gberet_spritebank;

static tilemap *bg_tilemap;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Green Beret has a 32 bytes palette PROM and two 256 bytes color lookup table
  PROMs (one for sprites, one for characters).
  The palette PROM is connected to the RGB output, this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( gberet )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	for (i = 0;i < TOTAL_COLORS(1);i++)
	{
		if (*color_prom & 0x0f) COLOR(1,i) = *color_prom & 0x0f;
		else COLOR(1,i) = 0;
		color_prom++;
	}
	for (i = 0;i < TOTAL_COLORS(0);i++)
	{
		COLOR(0,i) = (*(color_prom++) & 0x0f) + 0x10;
	}
}

WRITE8_HANDLER( gberet_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( gberet_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( gberet_scroll_w )
{
	int scroll;

	gberet_scrollram[offset] = data;

	scroll = gberet_scrollram[offset & 0x1f] | (gberet_scrollram[offset | 0x20] << 8);
	tilemap_set_scrollx(bg_tilemap, offset & 0x1f, scroll);
}

WRITE8_HANDLER( gberet_sprite_bank_w )
{
	gberet_spritebank = data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0x40) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	tileinfo->category = (attr & 0x80) >> 7;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( gberet )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_COLORTABLE, 8, 8, 64, 32);

	tilemap_set_transparent_pen(bg_tilemap, 0x10);
	tilemap_set_scroll_rows(bg_tilemap, 32);
}

static void gberet_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;
	UINT8 *sr;

	if (gberet_spritebank & 0x08)
		sr = spriteram_2;
	else
		sr = spriteram;

	for (offs = 0; offs < 0xc0; offs += 4)
	{
		if (sr[offs + 3])
		{
			int attr = sr[offs + 1];
			int code = sr[offs+0] + ((attr & 0x40) << 2);
			int color = attr & 0x0f;
			int sx = sr[offs + 2] - 2 * (attr & 0x80);
			int sy = sr[offs + 3];
			int flipx = attr & 0x10;
			int flipy = attr & 0x20;

			if (flip_screen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx(bitmap, machine->gfx[1], code, color, flipx, flipy, sx, sy,
				cliprect, TRANSPARENCY_COLOR, 0);
		}
	}
}

VIDEO_UPDATE( gberet )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_OPAQUE | 0, 0);
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_OPAQUE | 1, 0);
	gberet_draw_sprites(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}

/* Green Beret (bootleg) */

WRITE8_HANDLER( gberetb_scroll_w )
{
	int scroll;

	scroll = data;
	if (offset) scroll |= 0x100;

	for (offset = 6; offset < 29; offset++)
		tilemap_set_scrollx(bg_tilemap, offset, scroll + 64-8);
}

static void gberetb_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = spriteram_size - 4; offs >= 0; offs -= 4)
	{
		if (spriteram[offs + 1])
		{
			int attr = spriteram[offs + 3];
			int code = spriteram[offs] + ((attr & 0x40) << 2);
			int color = attr & 0x0f;
			int sx = spriteram[offs + 2] - 2 * (attr & 0x80);
			int sy = 240 - spriteram[offs + 1];
			int flipx = attr & 0x10;
			int flipy = attr & 0x20;

			if (flip_screen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx(bitmap, machine->gfx[1], code, color, flipx, flipy, sx, sy,
				cliprect, TRANSPARENCY_COLOR, 0);
		}
	}
}

VIDEO_UPDATE( gberetb )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_OPAQUE | 0, 0);
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_OPAQUE | 1, 0);
	gberetb_draw_sprites(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
