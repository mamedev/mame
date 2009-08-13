/***************************************************************************

Knuckle Joe - (c) 1985 Taito Corporation

***************************************************************************/

#include "driver.h"

static tilemap *bg_tilemap;
static int tile_bank,sprite_bank;
static int flipscreen;

UINT8 *kncljoe_scrollregs;

/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT( kncljoe )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x90);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x80; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	for (i = 0x80; i < 0x90; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[(i - 0x80) + 0x300] >> 6) & 0x01;
		bit2 = (color_prom[(i - 0x80) + 0x300] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[(i - 0x80) + 0x300] >> 3) & 0x01;
		bit1 = (color_prom[(i - 0x80) + 0x300] >> 4) & 0x01;
		bit2 = (color_prom[(i - 0x80) + 0x300] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (color_prom[(i - 0x80) + 0x300] >> 0) & 0x01;
		bit1 = (color_prom[(i - 0x80) + 0x300] >> 1) & 0x01;
		bit2 = (color_prom[(i - 0x80) + 0x300] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x320;

	/* chars */
	for (i = 0; i < 0x80; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* sprite lookup table */
	for (i = 0x80; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i - 0x80] & 0x0f) | 0x80;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = videoram[2*tile_index+1];
	int code = videoram[2*tile_index] + ((attr & 0xc0) << 2) + (tile_bank << 10);

	SET_TILE_INFO(
			0,
			code,
			attr & 0xf,
			TILE_FLIPXY((attr & 0x30) >> 4));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( kncljoe )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_scroll_rows(bg_tilemap,4);

	tile_bank = sprite_bank = flipscreen = 0;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( kncljoe_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset/2);
}

WRITE8_HANDLER( kncljoe_control_w )
{
	int i;
	/*
            0x01    screen flip
            0x02    coin counter#1
            0x04    sprite bank
            0x10    character bank
            0x20    coin counter#2

            reset when IN0 - Coin 1 goes low (active)
            set after IN0 - Coin 1 goes high AND the credit has been added
   */
	flipscreen = data & 0x01;
	tilemap_set_flip_all(space->machine,flipscreen ? TILEMAP_FLIPX : TILEMAP_FLIPY);

	coin_counter_w(0,data & 0x02);
	coin_counter_w(1,data & 0x20);

	i = (data & 0x10) >> 4;
	if (tile_bank != i)
	{
		tile_bank = i;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	i = (data & 0x04) >> 2;
	if (sprite_bank != i)
	{
		sprite_bank = i;
		memset(memory_region(space->machine, "maincpu")+0xf100, 0, 0x180);
	}
}

WRITE8_HANDLER( kncljoe_scroll_w )
{
	int scrollx;

	kncljoe_scrollregs[offset] = data;
	scrollx = kncljoe_scrollregs[0] | kncljoe_scrollregs[1]<<8;
	tilemap_set_scrollx(bg_tilemap,0,scrollx);
	tilemap_set_scrollx(bg_tilemap,1,scrollx);
	tilemap_set_scrollx(bg_tilemap,2,scrollx);
	tilemap_set_scrollx(bg_tilemap,3,0);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	rectangle clip = *cliprect;
	const gfx_element *gfx = machine->gfx[1 + sprite_bank];
	int i, j;
	static const int pribase[4]={0x0180, 0x0080, 0x0100, 0x0000};
	const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);

	/* score covers sprites */
	if (flipscreen)
	{
		if (clip.max_y > visarea->max_y - 64)
			clip.max_y = visarea->max_y - 64;
	}
	else
	{
		if (clip.min_y < visarea->min_y + 64)
			clip.min_y = visarea->min_y + 64;
	}

	for (i=0; i<4; i++)
	for (j=0x7c; j>=0; j-=4)
	{
		int offs = pribase[i] + j;
		int sy = spriteram[offs];
		int sx = spriteram[offs+3];
		int code = spriteram[offs+2];
		int attr = spriteram[offs+1];
		int flipx = attr & 0x40;
		int flipy = !(attr & 0x80);
		int color = attr & 0x0f;

		if (attr & 0x10) code += 512;
		if (attr & 0x20) code += 256;

		if (flipscreen)
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = 240 - sx;
			sy = 240 - sy;
		}

		if (sx >= 256-8) sx -= 256;

		drawgfx_transpen(bitmap,&clip,gfx,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

VIDEO_UPDATE( kncljoe )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}
