/* Space Bugger - Video Hardware */

#include "emu.h"
#include "includes/sbugger.h"

static TILE_GET_INFO( get_sbugger_tile_info )
{
	sbugger_state *state = machine->driver_data<sbugger_state>();
	int tileno, color;

	tileno = state->videoram[tile_index];
	color = state->videoram_attr[tile_index];

	SET_TILE_INFO(0,tileno,color,0);
}

WRITE8_HANDLER( sbugger_videoram_w )
{
	sbugger_state *state = space->machine->driver_data<sbugger_state>();

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap,offset);
}

WRITE8_HANDLER( sbugger_videoram_attr_w )
{
	sbugger_state *state = space->machine->driver_data<sbugger_state>();

	state->videoram_attr[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap,offset);
}

VIDEO_START(sbugger)
{
	sbugger_state *state = machine->driver_data<sbugger_state>();
	state->tilemap = tilemap_create(machine, get_sbugger_tile_info, tilemap_scan_rows, 8, 16, 64, 16);
}

VIDEO_UPDATE(sbugger)
{
	sbugger_state *state = screen->machine->driver_data<sbugger_state>();
	tilemap_draw(bitmap,cliprect,state->tilemap,0,0);
	return 0;
}

/* not right but so we can see things ok */
PALETTE_INIT(sbugger)
{
	/* just some random colours for now */
	int i;

	for (i = 0;i < 256;i++)
	{
		int r = machine->rand()|0x80;
		int g = machine->rand()|0x80;
		int b = machine->rand()|0x80;
		if (i == 0) r = g = b = 0;

		palette_set_color(machine,i*2+1,MAKE_RGB(r,g,b));
		palette_set_color(machine,i*2,MAKE_RGB(0,0,0));

	}

}
