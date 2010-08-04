#include "emu.h"
#include "includes/blockade.h"

WRITE8_HANDLER( blockade_videoram_w )
{
	blockade_state *state = space->machine->driver_data<blockade_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);

	if (input_port_read(space->machine, "IN3") & 0x80)
	{
		logerror("blockade_videoram_w: scanline %d\n", space->machine->primary_screen->vpos());
		cpu_spinuntil_int(space->cpu);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	blockade_state *state = machine->driver_data<blockade_state>();
	int code = state->videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( blockade )
{
	blockade_state *state = machine->driver_data<blockade_state>();
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

VIDEO_UPDATE( blockade )
{
	blockade_state *state = screen->machine->driver_data<blockade_state>();

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	return 0;
}
