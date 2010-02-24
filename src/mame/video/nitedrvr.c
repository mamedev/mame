/***************************************************************************

    Atari Night Driver hardware

***************************************************************************/

#include "emu.h"
#include "includes/nitedrvr.h"

WRITE8_HANDLER( nitedrvr_videoram_w )
{
	nitedrvr_state *state = (nitedrvr_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( nitedrvr_hvc_w )
{
	nitedrvr_state *state = (nitedrvr_state *)space->machine->driver_data;

	state->hvc[offset & 0x3f] = data;

	if ((offset & 0x30) == 0x30)
		watchdog_reset_w(space, 0, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	nitedrvr_state *state = (nitedrvr_state *)machine->driver_data;
	int code = state->videoram[tile_index] & 0x3f;

	SET_TILE_INFO(0, code, 0, 0);
}



VIDEO_START( nitedrvr )
{
	nitedrvr_state *state = (nitedrvr_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_box( bitmap_t *bitmap, int bx, int by, int ex, int ey )
{
	int x, y;

	for (y = by; y < ey; y++)
	{
		for (x = bx; x < ex; x++)
			if ((y < 256) && (x < 256))
				*BITMAP_ADDR16(bitmap, y, x) = 1;
	}

	return;
}

static void draw_roadway( running_machine *machine, bitmap_t *bitmap )
{
	nitedrvr_state *state = (nitedrvr_state *)machine->driver_data;
	int roadway;

	for (roadway = 0; roadway < 16; roadway++)
	{
		int bx, by, ex, ey;

		bx = state->hvc[roadway];
		by = state->hvc[roadway + 16];
		ex = bx + ((state->hvc[roadway + 32] & 0xf0) >> 4);
		ey = by + (16 - (state->hvc[roadway + 32] & 0x0f));

		draw_box(bitmap, bx, by, ex, ey);
	}
}

VIDEO_UPDATE( nitedrvr )
{
	nitedrvr_state *state = (nitedrvr_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_roadway(screen->machine, bitmap);
	return 0;
}
