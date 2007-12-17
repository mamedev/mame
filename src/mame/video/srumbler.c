/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


UINT8 *srumbler_backgroundram,*srumbler_foregroundram;
static tilemap *bg_tilemap,*fg_tilemap;



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT8 attr = srumbler_foregroundram[2*tile_index];
	SET_TILE_INFO(
			0,
			srumbler_foregroundram[2*tile_index + 1] + ((attr & 0x03) << 8),
			(attr & 0x3c) >> 2,
			(attr & 0x40) ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 attr = srumbler_backgroundram[2*tile_index];
	SET_TILE_INFO(
			1,
			srumbler_backgroundram[2*tile_index + 1] + ((attr & 0x07) << 8),
			(attr & 0xe0) >> 5,
			((attr & 0x08) ? TILE_FLIPY : 0));
	tileinfo->group = (attr & 0x10) >> 4;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( srumbler )
{
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,64,32);
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,    16,16,64,64);

	tilemap_set_transparent_pen(fg_tilemap,3);

	tilemap_set_transmask(bg_tilemap,0,0xffff,0x0000); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(bg_tilemap,1,0x07ff,0xf800); /* split type 1 has pens 0-10 transparent in front half */
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( srumbler_foreground_w )
{
	srumbler_foregroundram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset/2);
}

WRITE8_HANDLER( srumbler_background_w )
{
	srumbler_backgroundram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset/2);
}


WRITE8_HANDLER( srumbler_4009_w )
{
	/* bit 0 flips screen */
	flip_screen_set(data & 1);

	/* bits 4-5 used during attract mode, unknown */

	/* bits 6-7 coin counters */
	coin_counter_w(0,data & 0x40);
	coin_counter_w(1,data & 0x80);
}


WRITE8_HANDLER( srumbler_scroll_w )
{
	static int scroll[4];

	scroll[offset] = data;

	tilemap_set_scrollx(bg_tilemap,0,scroll[0] | (scroll[1] << 8));
	tilemap_set_scrolly(bg_tilemap,0,scroll[2] | (scroll[3] << 8));
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	/* Draw the sprites. */
	for (offs = spriteram_size-4; offs>=0;offs -= 4)
	{
		/* SPRITES
        =====
        Attribute
        0x80 Code MSB
        0x40 Code MSB
        0x20 Code MSB
        0x10 Colour
        0x08 Colour
        0x04 Colour
        0x02 y Flip
        0x01 X MSB
        */


		int code,colour,sx,sy,flipy;
		int attr = buffered_spriteram[offs+1];
		code = buffered_spriteram[offs];
		code += ( (attr&0xe0) << 3 );
		colour = (attr & 0x1c)>>2;
		sy = buffered_spriteram[offs + 2];
		sx = buffered_spriteram[offs + 3] + 0x100 * ( attr & 0x01);
		flipy = attr & 0x02;

		if (flip_screen)
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[2],
				code,
				colour,
				flip_screen,flipy,
				sx, sy,
				cliprect,TRANSPARENCY_PEN,15);
	}
}


VIDEO_UPDATE( srumbler )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	return 0;
}

VIDEO_EOF( srumbler )
{
	buffer_spriteram_w(0,0);
}
