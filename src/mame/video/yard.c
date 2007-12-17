/***************************************************************************

  video.c

  10 Yard Fight

L Taylor
J Clegg

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *yard_scroll_x_low;
UINT8 *yard_scroll_x_high;
UINT8 *yard_scroll_y_low;
UINT8 *yard_score_panel_disabled;
static mame_bitmap *scroll_panel_bitmap;

static tilemap *bg_tilemap;

#define SCROLL_PANEL_WIDTH  (14*4)
#define RADAR_PALETTE_BASE (256+16)
#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

static rectangle clippanel =
{
	26*8, 32*8-1,
	1*8, 31*8-1
};

static rectangle clippanelflip =
{
	0*8, 6*8-1,
	1*8, 31*8-1
};

/***************************************************************************

  Convert the color PROMs into a more useable format.

  10 Yard Fight has two 256x4 character palette PROMs, one 32x8 sprite
  palette PROM, one 256x4 sprite color lookup table PROM, and two 256x4
  radar palette PROMs.

  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably something like this; note that RED and BLUE
  are swapped wrt the usual configuration.

  bit 7 -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
  bit 0 -- 1  kohm resistor  -- BLUE

***************************************************************************/
PALETTE_INIT( yard )
{
	int i;

	/* character palette */
	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[256] >> 2) & 0x01;
		bit2 = (color_prom[256] >> 3) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[256] >> 0) & 0x01;
		bit2 = (color_prom[256] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}

	color_prom += 256;
	/* color_prom now points to the beginning of the sprite palette */

	/* sprite palette */
	for (i = 0;i < 16;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i+256,MAKE_RGB(r,g,b));

		color_prom++;
	}

	color_prom += 16;
	/* color_prom now points to the beginning of the sprite lookup table */

	/* sprite lookup table */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = 256 + (*(color_prom++) & 0x0f);

	/* color_prom now points to the beginning of the radar palette */

	/* radar palette */
	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[256] >> 2) & 0x01;
		bit2 = (color_prom[256] >> 3) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[256] >> 0) & 0x01;
		bit2 = (color_prom[256] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i+256+16,MAKE_RGB(r,g,b));

		color_prom++;
	}
}

WRITE8_HANDLER( yard_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( yard_scroll_panel_w )
{
	int sx,sy,i;

	sx = ( offset % 16 );
	sy = ( offset / 16 );

	if (sx < 1 || sx > 14)  return;

	sx = 4 * (sx - 1);

	for (i = 0;i < 4;i++)
	{
		int col;

		col = (data >> i) & 0x11;
		col = ((col >> 3) | col) & 3;

		*BITMAP_ADDR16(scroll_panel_bitmap, sy, sx + i) = Machine->pens[RADAR_PALETTE_BASE + (sy & 0xfc) + col];
	}
}

static TILE_GET_INFO( yard_get_bg_tile_info )
{
	int offs = tile_index * 2;
	int attr = videoram[offs + 1];
	int code = videoram[offs] + ((attr & 0xc0) << 2);
	int color = attr & 0x1f;
	int flags = (attr & 0x20) ? TILE_FLIPX : 0;

	SET_TILE_INFO(0, code, color, flags);
}

static UINT32 yard_tilemap_scan_rows( UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows )
{
	/* logical (col,row) -> memory offset */
	if (col >= 32)
		return (row+32)*32 + col-32;
	else
		return row*32 + col;
}

VIDEO_START( yard )
{
	bg_tilemap = tilemap_create(yard_get_bg_tile_info, yard_tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 64, 32);

	scroll_panel_bitmap = auto_bitmap_alloc(SCROLL_PANEL_WIDTH, machine->screen[0].height, machine->screen[0].format);
}

#define DRAW_SPRITE(code, sy) drawgfx(bitmap, machine->gfx[1], code, color, flipx, flipy, sx, sy, cliprect, TRANSPARENCY_COLOR, 256);

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 1];
		int bank = (attr & 0x20) >> 5;
		int code1 = spriteram[offs + 2] & 0xbf;
		int code2 = 0;
		int color = attr & 0x1f;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram[offs + 3];
		int sy1 = 241 - spriteram[offs];
		int sy2 = 0;

		if (flipy)
		{
			code2 = code1;
			code1 += 0x40;
		}
		else
		{
			code2 = code1 + 0x40;
		}

		if (flip_screen)
		{
			sx = 240 - sx;
			sy2 = 224 - sy1;
			sy1 = sy2 + 0x10;
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sy2 = sy1 + 0x10;
		}

		DRAW_SPRITE(code1 + 256 * bank, sy1)
		DRAW_SPRITE(code2 + 256 * bank, sy2)
	}
}

static void draw_panel( mame_bitmap *bitmap, const rectangle *cliprect )
{
	if (! *yard_score_panel_disabled)
	{
		int sx = flip_screen ? cliprect->min_x - 8 : cliprect->max_x + 1 - SCROLL_PANEL_WIDTH;

		copybitmap(bitmap, scroll_panel_bitmap, flip_screen, flip_screen, sx, 0,
			flip_screen ? &clippanelflip : &clippanel, TRANSPARENCY_NONE, 0);
	}
}

VIDEO_UPDATE( yard )
{
	tilemap_set_scrollx(bg_tilemap, 0, (*yard_scroll_x_high * 0x100) + *yard_scroll_x_low);
	tilemap_set_scrolly(bg_tilemap, 0, *yard_scroll_y_low);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	draw_panel(bitmap, cliprect);
	return 0;
}
