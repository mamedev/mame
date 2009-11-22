/***************************************************************************

Atari Canyon Bomber video emulation

***************************************************************************/

#include "driver.h"
#include "includes/canyon.h"


WRITE8_HANDLER( canyon_videoram_w )
{
	canyon_state *state = (canyon_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	canyon_state *state = (canyon_state *)machine->driver_data;
	UINT8 code = state->videoram[tile_index];

	SET_TILE_INFO(0, code & 0x3f, code >> 7, 0);
}


VIDEO_START( canyon )
{
	canyon_state *state = (canyon_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}


static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle* cliprect )
{
	canyon_state *state = (canyon_state *)machine->driver_data;
	int i;

	for (i = 0; i < 2; i++)
	{
		int x = state->videoram[0x3d0 + 2 * i + 0x1];
		int y = state->videoram[0x3d0 + 2 * i + 0x8];
		int c = state->videoram[0x3d0 + 2 * i + 0x9];

		drawgfx_transpen(bitmap, cliprect,
			machine->gfx[1],
			c >> 3,
			i,
			!(c & 0x80), 0,
			224 - x,
			240 - y, 0);
	}
}


static void draw_bombs( running_machine *machine, bitmap_t *bitmap, const rectangle* cliprect )
{
	canyon_state *state = (canyon_state *)machine->driver_data;
	int i;

	for (i = 0; i < 2; i++)
	{
		int sx = 254 - state->videoram[0x3d0 + 2 * i + 0x5];
		int sy = 246 - state->videoram[0x3d0 + 2 * i + 0xc];

		rectangle rect;

		rect.min_x = sx;
		rect.min_y = sy;
		rect.max_x = sx + 1;
		rect.max_y = sy + 1;

		if (rect.min_x < cliprect->min_x) rect.min_x = cliprect->min_x;
		if (rect.min_y < cliprect->min_y) rect.min_y = cliprect->min_y;
		if (rect.max_x > cliprect->max_x) rect.max_x = cliprect->max_x;
		if (rect.max_y > cliprect->max_y) rect.max_y = cliprect->max_y;

		bitmap_fill(bitmap, &rect, 1 + 2 * i);
	}
}


VIDEO_UPDATE( canyon )
{
	canyon_state *state = (canyon_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect);

	draw_bombs(screen->machine, bitmap, cliprect);

	/* watchdog is disabled during service mode */
	watchdog_enable(screen->machine, !(input_port_read(screen->machine, "IN2") & 0x10));

	return 0;
}
