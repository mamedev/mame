/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/crbaloon.h"


UINT8 *crbaloon_videoram;
UINT8 *crbaloon_colorram;
UINT8 *crbaloon_spriteram;

static UINT16 crbaloon_collision_address;
static UINT8 crbaloon_collision_address_clear;
static tilemap_t *bg_tilemap;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Crazy Balloon has no PROMs, the color code directly maps to a color:
  all bits are inverted
  bit 3 HALF (intensity)
  bit 2 BLUE
  bit 1 GREEN
  bit 0 RED

***************************************************************************/

PALETTE_INIT( crbaloon )
{
	int i;

	for (i = 0; i < machine->total_colors(); i++)
	{
		UINT8 pen;
		int h, r, g, b;

		if (i & 0x01)
			pen = i >> 1;
		else
			pen = 0x0f;

		h = (~pen & 0x08) ? 0xff : 0x55;
		r = h * ((~pen >> 0) & 1);
		g = h * ((~pen >> 1) & 1);
		b = h * ((~pen >> 2) & 1);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


WRITE8_HANDLER( crbaloon_videoram_w )
{
	crbaloon_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( crbaloon_colorram_w )
{
	crbaloon_colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = crbaloon_videoram[tile_index];
	int color = crbaloon_colorram[tile_index] & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( crbaloon )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows_flip_xy,  8, 8, 32, 32);

	state_save_register_global(machine, crbaloon_collision_address);
	state_save_register_global(machine, crbaloon_collision_address_clear);
}


UINT16 crbaloon_get_collision_address(void)
{
	return crbaloon_collision_address_clear ? 0xffff : crbaloon_collision_address;
}


void crbaloon_set_clear_collision_address(int _crbaloon_collision_address_clear)
{
	crbaloon_collision_address_clear = !_crbaloon_collision_address_clear; /* active LO */
}



static void draw_sprite_and_check_collision(running_machine *machine, bitmap_t *bitmap)
{
	int y;
	UINT8 code = crbaloon_spriteram[0] & 0x0f;
	UINT8 color = crbaloon_spriteram[0] >> 4;
	UINT8 sy = crbaloon_spriteram[2] - 32;

	UINT8 *gfx = machine->region("gfx2")->base() + (code << 7);


	if (flip_screen_get(machine))
		sy += 32;

	/* assume no collision */
    crbaloon_collision_address = 0xffff;

	for (y = 0x1f; y >= 0; y--)
	{
		int x;
		UINT8 data = 0;
		UINT8 sx = crbaloon_spriteram[1];

		for (x = 0x1f; x >= 0; x--)
		{
			int bit;

			if ((x & 0x07) == 0x07)
				/* get next byte to draw, but no drawing in VBLANK */
				data = (sy >= 0xe0) ? 0 : gfx[((x >> 3) << 5) | y];

			bit = data & 0x80;

			/* draw the current pixel, but check collision first */
			if (bit)
			{
				if (*BITMAP_ADDR16(bitmap, sy, sx) & 0x01)
					/* compute the collision address -- the +1 is via observation
                       of the game code, probably wrong for cocktail mode */
					crbaloon_collision_address = ((((sy ^ 0xff) >> 3) << 5) | ((sx ^ 0xff) >> 3)) + 1;

				*BITMAP_ADDR16(bitmap, sy, sx) = (color << 1) | 1;
			}

			sx = sx + 1;
			data = data << 1;
        }

        sy = sy + 1;
	}
}


VIDEO_UPDATE( crbaloon )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	draw_sprite_and_check_collision(screen->machine, bitmap);

	return 0;
}
