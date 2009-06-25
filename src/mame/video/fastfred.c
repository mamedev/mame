/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/resnet.h"
#include "fastfred.h"
#include "includes/galaxold.h"

UINT8 *fastfred_videoram;
UINT8 *fastfred_spriteram;
size_t fastfred_spriteram_size;
UINT8 *fastfred_attributesram;
UINT8 *fastfred_background_color;
UINT8 *imago_fg_videoram;


static const rectangle spritevisiblearea =
{
      2*8, 32*8-1,
      2*8, 30*8-1
};

static const rectangle spritevisibleareaflipx =
{
        0*8, 30*8-1,
        2*8, 30*8-1
};

static UINT16 charbank;
static UINT8 colorbank;
int fastfred_hardware_type;
static tilemap *bg_tilemap, *fg_tilemap, *web_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( fastfred )
{
	static const int resistances[4] = { 1000, 470, 220, 100 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, rweights, 470, 0,
			4, resistances, gweights, 470, 0,
			4, resistances, bweights, 470, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
		r = combine_4_weights(rweights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = combine_4_weights(gweights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* characters and sprites use the same palette */
	for (i = 0; i < 0x100; i++)
		colortable_entry_set_value(machine->colortable, i, i);
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT8 x = tile_index & 0x1f;

	UINT16 code = charbank | fastfred_videoram[tile_index];
	UINT8 color = colorbank | (fastfred_attributesram[2 * x + 1] & 0x07);

	SET_TILE_INFO(0, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( fastfred )
{
	bg_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(bg_tilemap, 0);
	tilemap_set_scroll_cols(bg_tilemap, 32);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( fastfred_videoram_w )
{
	fastfred_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( fastfred_attributes_w )
{
	if (fastfred_attributesram[offset] != data)
	{
		if (offset & 0x01)
		{
			/* color change */
			int i;

			for (i = offset / 2; i < 0x0400; i += 32)
				tilemap_mark_tile_dirty(bg_tilemap, i);
		}
		else
		{
			/* coloumn scroll */
			tilemap_set_scrolly(bg_tilemap, offset / 2, data);
		}

		fastfred_attributesram[offset] = data;
	}
}


WRITE8_HANDLER( fastfred_charbank1_w )
{
	UINT16 new_data = (charbank & 0x0200) | ((data & 0x01) << 8);

	if (new_data != charbank)
	{
		tilemap_mark_all_tiles_dirty(bg_tilemap);

		charbank = new_data;
	}
}

WRITE8_HANDLER( fastfred_charbank2_w )
{
	UINT16 new_data = (charbank & 0x0100) | ((data & 0x01) << 9);

	if (new_data != charbank)
	{
		tilemap_mark_all_tiles_dirty(bg_tilemap);

		charbank = new_data;
	}
}


WRITE8_HANDLER( fastfred_colorbank1_w )
{
	UINT8 new_data = (colorbank & 0x10) | ((data & 0x01) << 3);

	if (new_data != colorbank)
	{
		tilemap_mark_all_tiles_dirty(bg_tilemap);

		colorbank = new_data;
	}
}

WRITE8_HANDLER( fastfred_colorbank2_w )
{
	UINT8 new_data = (colorbank & 0x08) | ((data & 0x01) << 4);

	if (new_data != colorbank)
	{
		tilemap_mark_all_tiles_dirty(bg_tilemap);

		colorbank = new_data;
	}
}



WRITE8_HANDLER( fastfred_flip_screen_x_w )
{
	if (flip_screen_x_get(space->machine) != (data & 0x01))
	{
		flip_screen_x_set(space->machine, data & 0x01);

		tilemap_set_flip(bg_tilemap, (flip_screen_x_get(space->machine) ? TILEMAP_FLIPX : 0) | (flip_screen_y_get(space->machine) ? TILEMAP_FLIPY : 0));
	}
}

WRITE8_HANDLER( fastfred_flip_screen_y_w )
{
	if (flip_screen_y_get(space->machine) != (data & 0x01))
	{
		flip_screen_y_set(space->machine, data & 0x01);

		tilemap_set_flip(bg_tilemap, (flip_screen_x_get(space->machine) ? TILEMAP_FLIPX : 0) | (flip_screen_y_get(space->machine) ? TILEMAP_FLIPY : 0));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = fastfred_spriteram_size - 4; offs >= 0; offs -= 4)
	{
		UINT8 code,sx,sy;
		int flipx,flipy;

		sx = fastfred_spriteram[offs + 3];
		sy = 240 - fastfred_spriteram[offs];

		if (fastfred_hardware_type == 3)
		{
			// Imago
			code  = (fastfred_spriteram[offs + 1]) & 0x3f;
			flipx = 0;
			flipy = 0;
		}
		else if (fastfred_hardware_type == 2)
		{
			// Boggy 84
			code  =  fastfred_spriteram[offs + 1] & 0x7f;
			flipx =  0;
			flipy =  fastfred_spriteram[offs + 1] & 0x80;
		}
		else if (fastfred_hardware_type == 1)
		{
			// Fly-Boy/Fast Freddie/Red Robin
			code  =  fastfred_spriteram[offs + 1] & 0x7f;
			flipx =  0;
			flipy = ~fastfred_spriteram[offs + 1] & 0x80;
		}
		else
		{
			// Jump Coaster
			code  = (fastfred_spriteram[offs + 1] & 0x3f) | 0x40;
			flipx = ~fastfred_spriteram[offs + 1] & 0x40;
			flipy =  fastfred_spriteram[offs + 1] & 0x80;
		}


		if (flip_screen_x_get(machine))
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y_get(machine))
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,flip_screen_x_get(machine) ? &spritevisibleareaflipx : &spritevisiblearea,machine->gfx[1],
				code,
				colorbank | (fastfred_spriteram[offs + 2] & 0x07),
				flipx,flipy,
				sx,sy,0);
	}
}


VIDEO_UPDATE( fastfred )
{
	bitmap_fill(bitmap, cliprect, *fastfred_background_color);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(screen->machine, bitmap, cliprect);

	return 0;
}


static TILE_GET_INFO( imago_get_tile_info_bg )
{
	UINT8 x = tile_index & 0x1f;

	UINT16 code = charbank * 0x100 + fastfred_videoram[tile_index];
	UINT8 color = colorbank | (fastfred_attributesram[2 * x + 1] & 0x07);

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( imago_get_tile_info_fg )
{
	int code = imago_fg_videoram[tile_index];
	SET_TILE_INFO(2, code, 2, 0);
}

static TILE_GET_INFO( imago_get_tile_info_web )
{
	SET_TILE_INFO(3, tile_index & 0x1ff, 0, 0);
}

WRITE8_HANDLER( imago_fg_videoram_w )
{
	imago_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( imago_charbank_w )
{
	if( charbank != data )
	{
		charbank = data;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

VIDEO_START( imago )
{
	web_tilemap = tilemap_create(machine, imago_get_tile_info_web,tilemap_scan_rows,     8,8,32,32);
	bg_tilemap   = tilemap_create(machine, imago_get_tile_info_bg, tilemap_scan_rows,8,8,32,32);
	fg_tilemap   = tilemap_create(machine, imago_get_tile_info_fg, tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(bg_tilemap, 0);
	tilemap_set_transparent_pen(fg_tilemap, 0);

	/* the game has a galaxian starfield */
	galaxold_init_stars(machine, 256);
	galaxold_stars_on = 1;

	/* web colors */
	palette_set_color(machine,256+64+0,MAKE_RGB(0x50,0x00,0x00));
	palette_set_color(machine,256+64+1,MAKE_RGB(0x00,0x00,0x00));
}

VIDEO_UPDATE( imago )
{
	tilemap_draw(bitmap,cliprect,web_tilemap,0,0);
	galaxold_draw_stars(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);

	return 0;
}
