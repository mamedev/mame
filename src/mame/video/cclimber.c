/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/cclimber.h"


#define CCLIMBER_FLIP_X		(cclimber_flip_screen[0] & 0x01)
#define CCLIMBER_FLIP_Y		(cclimber_flip_screen[1] & 0x01)
#define CCLIMBER_BG_PEN		(0)
#define SWIMMER_SIDE_BG_PEN	(0x120)
#define SWIMMER_BG_SPLIT	(0x18 * 8)
#define YAMATO_SKY_PEN_BASE	(0x60)

static tilemap_t *pf_tilemap;
static tilemap_t *bs_tilemap;

UINT8 *cclimber_videoram;
UINT8 *cclimber_colorram;
UINT8 *cclimber_spriteram;

UINT8 *cclimber_bigsprite_videoram;
UINT8 *cclimber_bigsprite_control;
UINT8 *cclimber_column_scroll;
UINT8 *cclimber_flip_screen;

UINT8 *swimmer_background_color;
UINT8 *swimmer_side_background_enabled;
UINT8 *swimmer_palettebank;

UINT8 *toprollr_bg_videoram;
UINT8 *toprollr_bg_coloram;
static tilemap_t *toproller_bg_tilemap;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Crazy Climber has three 32x8 palette PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( cclimber )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_rg[3], weights_b[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, resistances_rg, weights_rg, 0, 0,
			2, resistances_b,  weights_b,  0, 0,
			0, 0, 0, 0, 0);

	for (i = 0;i < machine->config->total_colors; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Swimmer has two 256x4 char/sprite palette PROMs and one 32x8 big sprite
  palette PROM.
  The palette PROMs are connected to the RGB output this way:
  (the 500 and 250 ohm resistors are made of 1 kohm resistors in parallel)

  bit 3 -- 250 ohm resistor  -- BLUE
        -- 500 ohm resistor  -- BLUE
        -- 250 ohm resistor  -- GREEN
  bit 0 -- 500 ohm resistor  -- GREEN
  bit 3 -- 1  kohm resistor  -- GREEN
        -- 250 ohm resistor  -- RED
        -- 500 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  bit 7 -- 250 ohm resistor  -- BLUE
        -- 500 ohm resistor  -- BLUE
        -- 250 ohm resistor  -- GREEN
        -- 500 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 250 ohm resistor  -- RED
        -- 500 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  Additionally, the background color of the score panel is determined by
  these resistors:

                  /--- tri-state --  470 -- BLUE
  +5V -- 1kohm ------- tri-state --  390 -- GREEN
                  \--- tri-state -- 1000 -- RED

***************************************************************************/

PALETTE_INIT( swimmer )
{
	int i;

	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		r = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		/* green component */
		bit0 = (color_prom[i + 0x000] >> 3) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 1) & 0x01;
		g = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 3) & 0x01;
		b = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}

	color_prom += 0x200;

	/* big sprite */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		palette_set_color(machine, i + 0x100, MAKE_RGB(r, g, b));
	}

	/* side panel backgrond pen */
#if 0
	// values calculated from the resistors don't seem to match the real board
	palette_set_color(machine, SWIMMER_SIDE_BG_PEN, MAKE_RGB(0x24, 0x5d, 0x4e));
#endif
	palette_set_color(machine, SWIMMER_SIDE_BG_PEN, MAKE_RGB(0x20, 0x98, 0x79));
}


PALETTE_INIT( yamato )
{
	int i;

	/* chars - 12 bits RGB */
	for (i = 0; i < 0x40; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x00] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x00] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x00] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x00] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[i + 0x00] >> 4) & 0x01;
		bit1 = (color_prom[i + 0x00] >> 5) & 0x01;
		bit2 = (color_prom[i + 0x00] >> 6) & 0x01;
		bit3 = (color_prom[i + 0x00] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[i + 0x40] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x40] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x40] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x40] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}

	/* big sprite - 8 bits RGB */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x80] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x80] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x80] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i + 0x80] >> 3) & 0x01;
		bit1 = (color_prom[i + 0x80] >> 4) & 0x01;
		bit2 = (color_prom[i + 0x80] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i + 0x80] >> 6) & 0x01;
		bit2 = (color_prom[i + 0x80] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i + 0x40, MAKE_RGB(r, g, b));
	}

	/* fake colors for bg gradient */
	for (i = 0; i < 0x100; i++)
		palette_set_color(machine, YAMATO_SKY_PEN_BASE + i, MAKE_RGB(0, 0, i));
}


PALETTE_INIT( toprollr )
{
	int i;

	for (i = 0; i < 0xa0; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


/***************************************************************************

  Swimmer can directly set the background color.
  The latch is connected to the RGB output this way:
  (the 500 and 250 ohm resistors are made of 1 kohm resistors in parallel)

  bit 7 -- 250 ohm resistor  -- RED
        -- 500 ohm resistor  -- RED
        -- 250 ohm resistor  -- GREEN
        -- 500 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 250 ohm resistor  -- BLUE
        -- 500 ohm resistor  -- BLUE
  bit 0 -- 1  kohm resistor  -- BLUE

***************************************************************************/

static void swimmer_set_background_pen(running_machine *machine)
{
	int bit0, bit1, bit2;
	int r, g, b;

	/* red component */
	bit0 = 0;
	bit1 = (*swimmer_background_color >> 6) & 0x01;
	bit2 = (*swimmer_background_color >> 7) & 0x01;
	r = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

	/* green component */
	bit0 = (*swimmer_background_color >> 3) & 0x01;
	bit1 = (*swimmer_background_color >> 4) & 0x01;
	bit2 = (*swimmer_background_color >> 5) & 0x01;
	g = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

	/* blue component */
	bit0 = (*swimmer_background_color >> 0) & 0x01;
	bit1 = (*swimmer_background_color >> 1) & 0x01;
	bit2 = (*swimmer_background_color >> 2) & 0x01;
	b = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

	palette_set_color(machine, CCLIMBER_BG_PEN, MAKE_RGB(r, g, b));
}



WRITE8_HANDLER( cclimber_colorram_w )
{
	/* A5 is not connected, there is only 0x200 bytes of RAM */
	cclimber_colorram[offset & ~0x20] = data;
	cclimber_colorram[offset |  0x20] = data;
}


WRITE8_HANDLER( cannonb_flip_screen_w )
{
	cclimber_flip_screen[0] = data;
	cclimber_flip_screen[1] = data;
}


static TILE_GET_INFO( cclimber_get_pf_tile_info )
{
	int code, color;

	int flags = TILE_FLIPYX(cclimber_colorram[tile_index] >> 6);

	/* vertical flipping flips two adjacent characters */
	if (flags & 0x02)
		tile_index = tile_index ^ 0x20;

	code = ((cclimber_colorram[tile_index] & 0x10) << 5) |
		   ((cclimber_colorram[tile_index] & 0x20) << 3) |
			 cclimber_videoram[tile_index];

	color = cclimber_colorram[tile_index] & 0x0f;

	SET_TILE_INFO(0, code, color, flags);
}


static TILE_GET_INFO( swimmer_get_pf_tile_info )
{
	int code, color;

	int flags = TILE_FLIPYX(cclimber_colorram[tile_index] >> 6);

	/* vertical flipping flips two adjacent characters */
	if (flags & 0x02)
		tile_index = tile_index ^ 0x20;

	code = ((cclimber_colorram[tile_index] & 0x10) << 4) | cclimber_videoram[tile_index];
	color = ((*swimmer_palettebank & 0x01) << 4) | (cclimber_colorram[tile_index] & 0x0f);

	SET_TILE_INFO(0, code, color, flags);
}


static TILE_GET_INFO( toprollr_get_pf_tile_info )
{
	int code, attr, color;

	attr = tile_index & 0x10 ? cclimber_colorram[tile_index & ~0x20] : cclimber_colorram[tile_index];
	code = ((attr & 0x30) << 4) | cclimber_videoram[tile_index];
	color = attr & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}


static TILE_GET_INFO( cclimber_get_bs_tile_info )
{
	int code, color;

	/* only the lower right is visible */
	tileinfo->group = ((tile_index & 0x210) == 0x210) ? 0 : 1;

	/* the address doesn't use A4 of the coordinates, giving a 16x16 map */
	tile_index = ((tile_index & 0x1e0) >> 1) | (tile_index & 0x0f);

	code = ((cclimber_bigsprite_control[1] & 0x08) << 5) | cclimber_bigsprite_videoram[tile_index];
	color = cclimber_bigsprite_control[1] & 0x07;

	SET_TILE_INFO(2, code, color, 0);
}


static TILE_GET_INFO( toprollr_get_bs_tile_info )
{
	int code, color;

	/* only the lower right is visible */
	tileinfo->group = ((tile_index & 0x210) == 0x210) ? 0 : 1;

	/* the address doesn't use A4 of the coordinates, giving a 16x16 map */
	tile_index = ((tile_index & 0x1e0) >> 1) | (tile_index & 0x0f);

	code = ((cclimber_bigsprite_control[1] & 0x18) << 5) | cclimber_bigsprite_videoram[tile_index];
	color = cclimber_bigsprite_control[1] & 0x07;

	SET_TILE_INFO(2, code, color, 0);
}


static TILE_GET_INFO( toproller_get_bg_tile_info )
{
	int code = ((toprollr_bg_coloram[tile_index] & 0x40) << 2) | toprollr_bg_videoram[tile_index];
	int color = toprollr_bg_coloram[tile_index] & 0x0f;

	SET_TILE_INFO(3, code, color, TILE_FLIPX);
}


VIDEO_START( cclimber )
{
	pf_tilemap = tilemap_create(machine, cclimber_get_pf_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(pf_tilemap, 0);
	tilemap_set_scroll_cols(pf_tilemap, 32);

	bs_tilemap = tilemap_create(machine, cclimber_get_bs_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(bs_tilemap, 1);
	tilemap_set_scroll_rows(bs_tilemap, 1);
	tilemap_set_transmask(bs_tilemap, 0, 0x01, 0);	/* pen 0 is transaprent */
	tilemap_set_transmask(bs_tilemap, 1, 0x0f, 0);  /* all 4 pens are transparent */
}


VIDEO_START( swimmer )
{
	pf_tilemap = tilemap_create(machine, swimmer_get_pf_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(pf_tilemap, 0);
	tilemap_set_scroll_cols(pf_tilemap, 32);

	bs_tilemap = tilemap_create(machine, cclimber_get_bs_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(bs_tilemap, 1);
	tilemap_set_scroll_rows(bs_tilemap, 1);
	tilemap_set_transmask(bs_tilemap, 0, 0x01, 0);	/* pen 0 is transaprent */
	tilemap_set_transmask(bs_tilemap, 1, 0xff, 0);  /* all 8 pens are transparent */
}


VIDEO_START( toprollr )
{
	pf_tilemap = tilemap_create(machine, toprollr_get_pf_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(pf_tilemap, 0);

	toproller_bg_tilemap = tilemap_create(machine, toproller_get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_rows(toproller_bg_tilemap, 1);

	bs_tilemap = tilemap_create(machine, toprollr_get_bs_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(bs_tilemap, 1);
	tilemap_set_scroll_rows(bs_tilemap, 1);
	tilemap_set_transmask(bs_tilemap, 0, 0x01, 0);	/* pen 0 is transaprent */
	tilemap_set_transmask(bs_tilemap, 1, 0x0f, 0);  /* all 4 pens are transparent */
}


static void draw_playfield(bitmap_t *bitmap, const rectangle *cliprect)
{
	int i;

	tilemap_mark_all_tiles_dirty(pf_tilemap);
	tilemap_set_flip(pf_tilemap, (CCLIMBER_FLIP_X ? TILEMAP_FLIPX : 0) |
								 (CCLIMBER_FLIP_Y ? TILEMAP_FLIPY : 0));
	for (i = 0; i < 32; i++)
		tilemap_set_scrolly(pf_tilemap, i, cclimber_column_scroll[i]);

	tilemap_draw(bitmap, cliprect, pf_tilemap, 0, 0);
}


static void cclimber_draw_bigsprite(bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 x = cclimber_bigsprite_control[3] - 8;
	UINT8 y = cclimber_bigsprite_control[2];
	int bigsprite_flip_x = (cclimber_bigsprite_control[1] & 0x10) >> 4;
	int bigsprite_flip_y = (cclimber_bigsprite_control[1] & 0x20) >> 5;

	if (bigsprite_flip_x)
		x = 0x80 - x;

	if (bigsprite_flip_y)
		y = 0x80 - y;

	tilemap_mark_all_tiles_dirty(bs_tilemap);

	tilemap_set_flip(bs_tilemap, (bigsprite_flip_x ? TILEMAP_FLIPX : 0) |
								 (CCLIMBER_FLIP_Y ^ bigsprite_flip_y ? TILEMAP_FLIPY : 0));

	tilemap_set_scrollx(bs_tilemap, 0, x);
	tilemap_set_scrolly(bs_tilemap, 0, y);

	tilemap_draw(bitmap, cliprect, bs_tilemap, 0, 0);
}


static void toprollr_draw_bigsprite(bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 x = cclimber_bigsprite_control[3] - 8;
	UINT8 y = cclimber_bigsprite_control[2];

	tilemap_mark_all_tiles_dirty(bs_tilemap);

	tilemap_set_flip(bs_tilemap, CCLIMBER_FLIP_Y ? TILEMAP_FLIPY : 0);

	tilemap_set_scrollx(bs_tilemap, 0, x);
	tilemap_set_scrolly(bs_tilemap, 0, y);

	tilemap_draw(bitmap, cliprect, bs_tilemap, 0, 0);
}


static void cclimber_draw_sprites(bitmap_t *bitmap, const rectangle *cliprect, const gfx_element *gfx)
{
	int offs;

	/* draw the sprites -- note that it is important to draw them exactly in this
       order, to have the correct priorities. */
	for (offs = 0x1c; offs >= 0; offs -= 4)
	{
		int x = cclimber_spriteram[offs + 3];
		int y = 240 - cclimber_spriteram[offs + 2];

		int code = ((cclimber_spriteram[offs + 1] & 0x10) << 3) |
				   ((cclimber_spriteram[offs + 1] & 0x20) << 1) |
				   ( cclimber_spriteram[offs + 0] & 0x3f);

		int color = cclimber_spriteram[offs + 1] & 0x0f;

		int flipx = cclimber_spriteram[offs + 0] & 0x40;
		int flipy = cclimber_spriteram[offs + 0] & 0x80;

		if (CCLIMBER_FLIP_X)
		{
			x = 240 - x;
			flipx = !flipx;
		}

		if (CCLIMBER_FLIP_Y)
		{
			y = 240 - y;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, gfx, code, color, flipx, flipy, x, y, 0);
	}
}


static void swimmer_draw_sprites(bitmap_t *bitmap, const rectangle *cliprect, const gfx_element *gfx)
{
	int offs;

	/* draw the sprites -- note that it is important to draw them exactly in this
       order, to have the correct priorities. */
	for (offs = 0x1c; offs >= 0; offs -= 4)
	{
		int x = cclimber_spriteram[offs + 3];
		int y = 240 - cclimber_spriteram[offs + 2];

		int code = ((cclimber_spriteram[offs + 1] & 0x10) << 2) |
				   (cclimber_spriteram[offs + 0] & 0x3f);

		int color = ((*swimmer_palettebank & 0x01) << 4) |
					(cclimber_spriteram[offs + 1] & 0x0f);

		int flipx = cclimber_spriteram[offs + 0] & 0x40;
		int flipy = cclimber_spriteram[offs + 0] & 0x80;

		if (CCLIMBER_FLIP_X)
		{
			x = 240 - x;
			flipx = !flipx;
		}

		if (CCLIMBER_FLIP_Y)
		{
			y = 240 - y;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, gfx, code, color, flipx, flipy, x, y, 0);
	}
}


VIDEO_UPDATE( cclimber )
{
	bitmap_fill(bitmap, cliprect, CCLIMBER_BG_PEN);
	draw_playfield(bitmap, cliprect);

	/* draw the "big sprite" under the regular sprites */
	if ((cclimber_bigsprite_control[0] & 0x01))
	{
		cclimber_draw_bigsprite(bitmap, cliprect);
		cclimber_draw_sprites(bitmap, cliprect, screen->machine->gfx[1]);
	}

	/* draw the "big sprite" over the regular sprites */
	else
	{
		cclimber_draw_sprites(bitmap, cliprect, screen->machine->gfx[1]);
		cclimber_draw_bigsprite(bitmap, cliprect);
	}

	return 0;
}


VIDEO_UPDATE( yamato )
{
	int i;
	UINT8 *sky_rom = memory_region(screen->machine, "user1") + 0x1200;

	for (i = 0; i < 0x100; i++)
	{
		int j;
		pen_t pen = YAMATO_SKY_PEN_BASE + sky_rom[(CCLIMBER_FLIP_X ? 0x80 : 0) + (i >> 1)];

		for (j = 0; j < 0x100; j++)
			*BITMAP_ADDR16(bitmap, j, (i - 8) & 0xff) = pen;
	}

	draw_playfield(bitmap, cliprect);

	/* draw the "big sprite" under the regular sprites */
	if ((cclimber_bigsprite_control[0] & 0x01))
	{
		cclimber_draw_bigsprite(bitmap, cliprect);
		cclimber_draw_sprites(bitmap, cliprect, screen->machine->gfx[1]);
	}

	/* draw the "big sprite" over the regular sprites */
	else
	{
		cclimber_draw_sprites(bitmap, cliprect, screen->machine->gfx[1]);
		cclimber_draw_bigsprite(bitmap, cliprect);
	}

	return 0;
}


VIDEO_UPDATE( swimmer )
{
	swimmer_set_background_pen(screen->machine);

	if (*swimmer_side_background_enabled & 0x01)
	{
		if (CCLIMBER_FLIP_X)
		{
			rectangle split_rect_left =  { 0, 0xff - SWIMMER_BG_SPLIT, 0, 0xff };
			rectangle split_rect_right = { 0x100 - SWIMMER_BG_SPLIT, 0xff, 0, 0xff };

			sect_rect(&split_rect_left, cliprect);
			bitmap_fill(bitmap, &split_rect_left, SWIMMER_SIDE_BG_PEN);

			sect_rect(&split_rect_right, cliprect);
			bitmap_fill(bitmap, &split_rect_right, CCLIMBER_BG_PEN);
		}
		else
		{
			rectangle split_rect_left =  { 0, SWIMMER_BG_SPLIT - 1, 0, 0xff };
			rectangle split_rect_right = { SWIMMER_BG_SPLIT, 0xff, 0, 0xff };

			sect_rect(&split_rect_left, cliprect);
			bitmap_fill(bitmap, &split_rect_left, CCLIMBER_BG_PEN);

			sect_rect(&split_rect_right, cliprect);
			bitmap_fill(bitmap, &split_rect_right, SWIMMER_SIDE_BG_PEN);
		}
	}
	else
		bitmap_fill(bitmap, cliprect, CCLIMBER_BG_PEN);

	draw_playfield(bitmap, cliprect);

	/* draw the "big sprite" under the regular sprites */
	if ((cclimber_bigsprite_control[0] & 0x01))
	{
		cclimber_draw_bigsprite(bitmap, cliprect);
		swimmer_draw_sprites(bitmap, cliprect, screen->machine->gfx[1]);
	}

	/* draw the "big sprite" over the regular sprites */
	else
	{
		swimmer_draw_sprites(bitmap, cliprect, screen->machine->gfx[1]);
		cclimber_draw_bigsprite(bitmap, cliprect);
	}

	return 0;
}


VIDEO_UPDATE( toprollr )
{
	rectangle scroll_area_clip = *cliprect;
	scroll_area_clip.min_x = 4*8;
	scroll_area_clip.max_x = 29*8-1;

	bitmap_fill(bitmap, cliprect, CCLIMBER_BG_PEN);

	tilemap_set_scrollx(toproller_bg_tilemap, 0, toprollr_bg_videoram[0]);
	tilemap_set_flip(toproller_bg_tilemap, (CCLIMBER_FLIP_X ? TILEMAP_FLIPX : 0) |
										   (CCLIMBER_FLIP_Y ? TILEMAP_FLIPY : 0));
	tilemap_mark_all_tiles_dirty(toproller_bg_tilemap);
	tilemap_draw(bitmap, &scroll_area_clip, toproller_bg_tilemap, 0, 0);

	/* draw the "big sprite" over the regular sprites */
	if ((cclimber_bigsprite_control[1] & 0x20))
	{
		cclimber_draw_sprites(bitmap, &scroll_area_clip, screen->machine->gfx[1]);
		toprollr_draw_bigsprite(bitmap, &scroll_area_clip);
	}

	/* draw the "big sprite" under the regular sprites */
	else
	{
		toprollr_draw_bigsprite(bitmap, &scroll_area_clip);
		cclimber_draw_sprites(bitmap, &scroll_area_clip, screen->machine->gfx[1]);
	}

	tilemap_mark_all_tiles_dirty(pf_tilemap);
	tilemap_set_flip(pf_tilemap, (CCLIMBER_FLIP_X ? TILEMAP_FLIPX : 0) |
								 (CCLIMBER_FLIP_Y ? TILEMAP_FLIPY : 0));
	tilemap_draw(bitmap, cliprect, pf_tilemap, 0, 0);

	return 0;
}
