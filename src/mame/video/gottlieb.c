/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *gottlieb_charram;

static int background_priority = 0;
static int spritebank;

static tilemap *bg_tilemap;
static int swap_bg_ramrom;

static UINT8 last_video_outputs;

/***************************************************************************

  Gottlieb games dosn't have a color PROM. They use 32 bytes of RAM to
  dynamically create the palette. Each couple of bytes defines one
  color (4 bits per pixel; the high 4 bits of the second byte are unused).

  The RAM is conected to the RGB output this way:

  bit 7 -- 240 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2  kohm resistor  -- GREEN
        -- 240 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2  kohm resistor  -- RED

  bit 7 -- unused
        -- unused
        -- unused
        -- unused
        -- 240 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2  kohm resistor  -- BLUE

***************************************************************************/
WRITE8_HANDLER( gottlieb_paletteram_w )
{
	int bit0, bit1, bit2, bit3;
	int r, g, b, val;

	paletteram[offset] = data;

	/* red component */

	val = paletteram[offset | 1];

	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;

	r = 0x10 * bit0 + 0x21 * bit1 + 0x46 * bit2 + 0x88 * bit3;

	/* green component */

	val = paletteram[offset & ~1];

	bit0 = (val >> 4) & 0x01;
	bit1 = (val >> 5) & 0x01;
	bit2 = (val >> 6) & 0x01;
	bit3 = (val >> 7) & 0x01;

	g = 0x10 * bit0 + 0x21 * bit1 + 0x46 * bit2 + 0x88 * bit3;

	/* blue component */

	val = paletteram[offset & ~1];

	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;

	b = 0x10 * bit0 + 0x21 * bit1 + 0x46 * bit2 + 0x88 * bit3;

	palette_set_color(Machine, offset / 2, MAKE_RGB(r, g, b));
}

WRITE8_HANDLER( gottlieb_video_outputs_w )
{
	extern void gottlieb_knocker(void);

	background_priority = data & 0x01;

	if (flip_screen_x != (data & 0x02))
	{
		flip_screen_x_set(data & 0x02);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	if (flip_screen_y != (data & 0x04))
	{
		flip_screen_y_set(data & 0x04);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	/* in Q*Bert Qubes only, bit 4 controls the sprite bank */
	spritebank = (data & 0x10) >> 4;

	output_set_value("knocker0", (data >> 5) & 1);

	last_video_outputs = data;
}

WRITE8_HANDLER( usvsthem_video_outputs_w )
{
	background_priority = data & 0x01;

	/* in most games, bits 1 and 2 flip screen, however in the laser */
	/* disc games they are different. */

	/* bit 1 controls the sprite bank. */
	spritebank = (data & 0x02) >> 1;

	/* bit 2 video enable (0 = black screen) */

	/* bit 3 genlock control (1 = show laserdisc image) */
}

WRITE8_HANDLER( gottlieb_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( gottlieb_charram_w )
{
	if (gottlieb_charram[offset] != data)
	{
		gottlieb_charram[offset] = data;

		decodechar(Machine->gfx[0], offset / 32, gottlieb_charram,
			Machine->drv->gfxdecodeinfo[0].gfxlayout);

		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index];

	SET_TILE_INFO(0, code^swap_bg_ramrom, 0, 0);
}

static void gottlieb_video_start_common(void)
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(bg_tilemap, 0);
}

VIDEO_START( gottlieb )
{
	swap_bg_ramrom = 0x00;
	gottlieb_video_start_common();
}

VIDEO_START( vidvince )
{
	swap_bg_ramrom = 0x80;
	gottlieb_video_start_common();
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
    int offs;

	for (offs = 0; offs < spriteram_size - 8; offs += 4)     /* it seems there's something strange with sprites #62 and #63 */
	{
		/* coordinates hand tuned to make the position correct in Q*Bert Qubes start */
		/* of level animation. */
		int sx = (spriteram[offs + 1]) - 4;
		int sy = (spriteram[offs]) - 13;
		int code = (255 ^ spriteram[offs + 2]) + 256 * spritebank;

		if (flip_screen_x) sx = 233 - sx;
		if (flip_screen_y) sy = 244 - sy;

		if (spriteram[offs] || spriteram[offs + 1])	/* needed to avoid garbage on screen */
			drawgfx(bitmap, machine->gfx[1],
				code, 0,
				flip_screen_x, flip_screen_y,
				sx,sy,
				cliprect,
				TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( gottlieb )
{
	if (!background_priority)
	{
		tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
	}
	else
	{
		fillbitmap(bitmap, machine->pens[0], cliprect);
	}

	draw_sprites(machine, bitmap, cliprect);

	if (background_priority)
	{
		tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	}
	return 0;
}
