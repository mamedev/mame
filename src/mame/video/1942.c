/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


UINT8 *c1942_fgvideoram;
UINT8 *c1942_bgvideoram;

static int c1942_palette_bank;
static tilemap *fg_tilemap, *bg_tilemap;



/***************************************************************************

  Convert the color PROMs into a more useable format.

  1942 has three 256x4 palette PROMs (one per gun) and three 256x4 lookup
  table PROMs (one for characters, one for sprites, one for background tiles).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( 1942 )
{
	rgb_t palette[256];
	int i, colorbase;

	for (i = 0; i < 256; i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[i + 0*256] >> 0) & 0x01;
		bit1 = (color_prom[i + 0*256] >> 1) & 0x01;
		bit2 = (color_prom[i + 0*256] >> 2) & 0x01;
		bit3 = (color_prom[i + 0*256] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i + 1*256] >> 0) & 0x01;
		bit1 = (color_prom[i + 1*256] >> 1) & 0x01;
		bit2 = (color_prom[i + 1*256] >> 2) & 0x01;
		bit3 = (color_prom[i + 1*256] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i + 2*256] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*256] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*256] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*256] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette[i] = MAKE_RGB(r,g,b);
	}

	color_prom += 3*256;
	/* color_prom now points to the beginning of the lookup table */


	/* characters use palette entries 128-143 */
	colorbase = 0;
	for (i = 0; i < 64*4; i++)
		palette_set_color(machine, colorbase + i, palette[0x80 | *color_prom++]);
	colorbase += 64*4;

	/* background tiles use palette entries 0-63 in four banks */
	for (i = 0; i < 32*8; i++)
	{
		palette_set_color(machine, colorbase + 0*32*8 + i, palette[0x00 | *color_prom]);
		palette_set_color(machine, colorbase + 1*32*8 + i, palette[0x10 | *color_prom]);
		palette_set_color(machine, colorbase + 2*32*8 + i, palette[0x20 | *color_prom]);
		palette_set_color(machine, colorbase + 3*32*8 + i, palette[0x30 | *color_prom]);
		color_prom++;
	}
	colorbase += 4*32*8;

	/* sprites use palette entries 64-79 */
	for (i = 0; i < 16*16; i++)
		palette_set_color(machine, colorbase + i, palette[0x40 | *color_prom++]);
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	int code, color;

	code = c1942_fgvideoram[tile_index];
	color = c1942_fgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			code + ((color & 0x80) << 1),
			color & 0x3f,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code, color;

	tile_index = (tile_index & 0x0f) | ((tile_index & 0x01f0) << 1);

	code = c1942_bgvideoram[tile_index];
	color = c1942_bgvideoram[tile_index + 0x10];
	SET_TILE_INFO(
			1,
			code + ((color & 0x80) << 1),
			(color & 0x1f) + (0x20 * c1942_palette_bank),
			TILE_FLIPYX((color & 0x60) >> 5));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
VIDEO_START( 1942 )
{
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,32,32);
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,     16,16,32,16);

	tilemap_set_transparent_pen(fg_tilemap,0);

	state_save_register_global(c1942_palette_bank);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( c1942_fgvideoram_w )
{
	c1942_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( c1942_bgvideoram_w )
{
	c1942_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,(offset & 0x0f) | ((offset >> 1) & 0x01f0));
}


WRITE8_HANDLER( c1942_palette_bank_w )
{
	if (c1942_palette_bank != data)
	{
		c1942_palette_bank = data;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

WRITE8_HANDLER( c1942_scroll_w )
{
	static UINT8 scroll[2];

	scroll[offset] = data;
	tilemap_set_scrollx(bg_tilemap,0,scroll[0] | (scroll[1] << 8));
}


WRITE8_HANDLER( c1942_c804_w )
{
	/* bit 7: flip screen
       bit 4: cpu B reset
       bit 0: coin counter */

	coin_counter_w(0,data & 0x01);

	cpunum_set_input_line(1, INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	flip_screen_set(data & 0x80);
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;


	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int i,code,col,sx,sy,dir;


		code = (spriteram[offs] & 0x7f) + 4*(spriteram[offs + 1] & 0x20)
				+ 2*(spriteram[offs] & 0x80);
		col = spriteram[offs + 1] & 0x0f;
		sx = spriteram[offs + 3] - 0x10 * (spriteram[offs + 1] & 0x10);
		sy = spriteram[offs + 2];
		dir = 1;
		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			dir = -1;
		}

		/* handle double / quadruple height */
		i = (spriteram[offs + 1] & 0xc0) >> 6;
		if (i == 2) i = 3;

		do
		{
			drawgfx(bitmap,machine->gfx[2],
					code + i,col,
					flip_screen,flip_screen,
					sx,sy + 16 * i * dir,
					cliprect,TRANSPARENCY_PEN,15);

			i--;
		} while (i >= 0);
	}


}

VIDEO_UPDATE( 1942 )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	return 0;
}
