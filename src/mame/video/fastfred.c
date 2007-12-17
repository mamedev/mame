/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "fastfred.h"

extern UINT8 galaxian_stars_on;
void galaxian_init_stars(running_machine *machine, int colors_offset);
void galaxian_draw_stars(running_machine *machine, mame_bitmap *bitmap);

UINT8 *fastfred_videoram;
UINT8 *fastfred_spriteram;
size_t fastfred_spriteram_size;
UINT8 *fastfred_attributesram;
UINT8 *imago_fg_videoram;


static rectangle spritevisiblearea =
{
      2*8, 32*8-1,
      2*8, 30*8-1
};

static rectangle spritevisibleareaflipx =
{
        0*8, 30*8-1,
        2*8, 30*8-1
};

static UINT16 charbank;
static UINT8 colorbank;
int fastfred_hardware_type;
static const UINT8 *fastfred_color_prom;
static tilemap *bg_tilemap, *fg_tilemap, *web_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

static void set_color(pen_t pen, int i)
{
	UINT8 r,g,b;
	int bit0, bit1, bit2, bit3;

	bit0 = (fastfred_color_prom[i] >> 0) & 0x01;
	bit1 = (fastfred_color_prom[i] >> 1) & 0x01;
	bit2 = (fastfred_color_prom[i] >> 2) & 0x01;
	bit3 = (fastfred_color_prom[i] >> 3) & 0x01;
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
	bit0 = (fastfred_color_prom[i + 0x100] >> 0) & 0x01;
	bit1 = (fastfred_color_prom[i + 0x100] >> 1) & 0x01;
	bit2 = (fastfred_color_prom[i + 0x100] >> 2) & 0x01;
	bit3 = (fastfred_color_prom[i + 0x100] >> 3) & 0x01;
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
	bit0 = (fastfred_color_prom[i + 0x200] >> 0) & 0x01;
	bit1 = (fastfred_color_prom[i + 0x200] >> 1) & 0x01;
	bit2 = (fastfred_color_prom[i + 0x200] >> 2) & 0x01;
	bit3 = (fastfred_color_prom[i + 0x200] >> 3) & 0x01;
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	palette_set_color_rgb(Machine,pen,r,g,b);
}

PALETTE_INIT( fastfred )
{
	pen_t i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	fastfred_color_prom = color_prom;	/* we'll need this later */

	for (i = 0;i < 256;i++)
	{
		set_color(i, i);
	}


	/* characters and sprites use the same palette */
	for (i = 0; i < TOTAL_COLORS(0); i++)
	{
		pen_t color;

		if ((i & 0x07) == 0)
			color = 0;
		else
			color = i;

		COLOR(0,i) = COLOR(1,i) = color;
	}
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
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

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


WRITE8_HANDLER( fastfred_background_color_w )
{
	set_color(0, data);
}


WRITE8_HANDLER( fastfred_flip_screen_x_w )
{
	if (flip_screen_x != (data & 0x01))
	{
		flip_screen_x = data & 0x01;

		tilemap_set_flip(bg_tilemap, (flip_screen_x ? TILEMAP_FLIPX : 0) | (flip_screen_y ? TILEMAP_FLIPY : 0));
	}
}

WRITE8_HANDLER( fastfred_flip_screen_y_w )
{
	if (flip_screen_y != (data & 0x01))
	{
		flip_screen_y = data & 0x01;

		tilemap_set_flip(bg_tilemap, (flip_screen_x ? TILEMAP_FLIPX : 0) | (flip_screen_y ? TILEMAP_FLIPY : 0));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
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

			//fastfred_spriteram[offs + 2] & 0xf8 get only set at startup
			//the code is greater than 0x3f only at startup

			/* TODO: find correct sprites banking */

			code  = (fastfred_spriteram[offs + 1]) & 0x1f;

			code |= fastfred_spriteram[offs + 2]<<5;

			if(fastfred_spriteram[offs + 1] & 0x20)
				code ^= 0xff;

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


		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[1],
				code,
				colorbank | (fastfred_spriteram[offs + 2] & 0x07),
				flipx,flipy,
				sx,sy,
				flip_screen_x ? &spritevisibleareaflipx : &spritevisiblearea,TRANSPARENCY_PEN,0);
	}
}


VIDEO_UPDATE( fastfred )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_sprites(machine, bitmap, cliprect);
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
	web_tilemap = tilemap_create(imago_get_tile_info_web,tilemap_scan_rows,TILEMAP_TYPE_PEN,     8,8,32,32);
	bg_tilemap   = tilemap_create(imago_get_tile_info_bg, tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	fg_tilemap   = tilemap_create(imago_get_tile_info_fg, tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transparent_pen(bg_tilemap, 0);
	tilemap_set_transparent_pen(fg_tilemap, 0);

	/* the game has a galaxian starfield */
	galaxian_init_stars(machine, 256);
	galaxian_stars_on = 1;

	/* web colors */
	palette_set_color(machine,256+64+0,MAKE_RGB(0x50,0x00,0x00));
	palette_set_color(machine,256+64+1,MAKE_RGB(0x00,0x00,0x00));
}

VIDEO_UPDATE( imago )
{
	tilemap_draw(bitmap,cliprect,web_tilemap,0,0);

	galaxian_draw_stars(machine, bitmap);

	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_sprites(machine, bitmap, cliprect);

	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	return 0;
}
