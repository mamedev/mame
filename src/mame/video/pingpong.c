/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static tilemap *bg_tilemap;


/* This is strange; it's unlikely that the sprites actually have a hardware */
/* clipping region, but I haven't found another way to have them masked by */
/* the characters at the top and bottom of the screen. */
static rectangle spritevisiblearea =
{
	0*8, 32*8-1,
	4*8, 29*8-1
};



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Ping Pong has a 32 bytes palette PROM and two 256 bytes color lookup table
  PROMs (one for sprites, one for characters).
  I don't know for sure how the palette PROM is connected to the RGB output,
  but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( pingpong )
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

	/* color_prom now points to the beginning of the char lookup table */

	/* sprites */
	for (i = 0;i < TOTAL_COLORS(1);i++)
	{
		int code;
		int bit0,bit1,bit2,bit3;

		/* the bits of the color code are in reverse order - 0123 instead of 3210 */
		code = *(color_prom++) & 0x0f;
		bit0 = (code >> 0) & 1;
		bit1 = (code >> 1) & 1;
		bit2 = (code >> 2) & 1;
		bit3 = (code >> 3) & 1;
		code = (bit0 << 3) | (bit1 << 2) | (bit2 << 1) | (bit3 << 0);
		COLOR(1,i) = code;
	}

	/* characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = (*(color_prom++) & 0x0f) + 0x10;
}

WRITE8_HANDLER( pingpong_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( pingpong_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0x20) << 3);
	int color = attr & 0x1f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( pingpong )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx,sy,flipx,flipy,color,schar;


		sx = spriteram[offs + 3];
		sy = 241 - spriteram[offs + 1];

		flipx = spriteram[offs] & 0x40;
		flipy = spriteram[offs] & 0x80;
		color = spriteram[offs] & 0x1F;
		schar = spriteram[offs + 2] & 0x7F;

		drawgfx(bitmap,machine->gfx[1],
				schar,
				color,
				flipx,flipy,
				sx,sy,
				&spritevisiblearea,TRANSPARENCY_COLOR,0);
	}
}

VIDEO_UPDATE( pingpong )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
