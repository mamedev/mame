/***************************************************************************

    Video emulation for Omori Battle Cross

***************************************************************************/

#include "emu.h"
#include "includes/battlex.h"

PALETTE_INIT( battlex )
{
	int i, col;

	for (col = 0; col < 8; col++)
	{
		for (i = 0; i < 16; i++)
		{
			int data = i | col;
			int r = pal1bit(data >> 0);
			int b = pal1bit(data >> 1);
			int g = pal1bit(data >> 2);

#if 0
			/* from Tim's shots, bit 3 seems to have no effect (see e.g. Laser Ship on title screen) */
			if (i & 8)
			{
				r /= 2;
				g /= 2;
				b /= 2;
			}
#endif

			palette_set_color(machine, i + 16 * col, MAKE_RGB(r,g,b));
		}
	}
}

WRITE8_HANDLER( battlex_palette_w )
{
	palette_set_color_rgb(space->machine, 16 * 8 + offset, pal1bit(data >> 0), pal1bit(data >> 2), pal1bit(data >> 1));
}

WRITE8_HANDLER( battlex_scroll_x_lsb_w )
{
	battlex_state *state = space->machine->driver_data<battlex_state>();
	state->scroll_lsb = data;
}

WRITE8_HANDLER( battlex_scroll_x_msb_w )
{
	battlex_state *state = space->machine->driver_data<battlex_state>();
	state->scroll_msb = data;
}

WRITE8_HANDLER( battlex_videoram_w )
{
	battlex_state *state = space->machine->driver_data<battlex_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE8_HANDLER( battlex_flipscreen_w )
{
	/* bit 4 is used, but for what? */

	/* bit 7 is flip screen */

	if (flip_screen_get(space->machine) != (data & 0x80))
	{
		flip_screen_set(space->machine, data & 0x80);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	battlex_state *state = machine->driver_data<battlex_state>();
	int tile = state->videoram[tile_index * 2] | (((state->videoram[tile_index * 2 + 1] & 0x01)) << 8);
	int color = (state->videoram[tile_index * 2 + 1] & 0x0e) >> 1;

	SET_TILE_INFO(0, tile, color, 0);
}

VIDEO_START( battlex )
{
	battlex_state *state = machine->driver_data<battlex_state>();

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state_save_register_global(machine, state->scroll_lsb);
	state_save_register_global(machine, state->scroll_msb);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	battlex_state *state = machine->driver_data<battlex_state>();
	const gfx_element *gfx = machine->gfx[1];
	UINT8 *source = state->spriteram;
	UINT8 *finish = state->spriteram + 0x200;

	while (source < finish)
	{
		int sx = (source[0] & 0x7f) * 2 - (source[0] & 0x80) * 2;
		int sy = source[3];
		int tile = source[2] & 0x7f;
		int color = source[1] & 0x07;	/* bits 3,4,5 also used during explosions */
		int flipy = source[1] & 0x80;
		int flipx = source[1] & 0x40;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, gfx, tile, color, flipx, flipy, sx, sy, 0);
		source += 4;
	}

}

VIDEO_UPDATE(battlex)
{
	battlex_state *state = screen->machine->driver_data<battlex_state>();

	tilemap_set_scrollx(state->bg_tilemap, 0, state->scroll_lsb | (state->scroll_msb << 8));
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
