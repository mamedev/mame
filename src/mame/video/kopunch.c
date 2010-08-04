#include "emu.h"
#include "includes/kopunch.h"


PALETTE_INIT( kopunch )
{
	int i;

	color_prom += 24;	/* first 24 colors are black */
	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

WRITE8_HANDLER( kopunch_videoram_w )
{
	kopunch_state *state = space->machine->driver_data<kopunch_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( kopunch_videoram2_w )
{
	kopunch_state *state = space->machine->driver_data<kopunch_state>();
	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( kopunch_scroll_x_w )
{
	kopunch_state *state = space->machine->driver_data<kopunch_state>();
	tilemap_set_scrollx(state->bg_tilemap, 0, data);
}

WRITE8_HANDLER( kopunch_scroll_y_w )
{
	kopunch_state *state = space->machine->driver_data<kopunch_state>();
	tilemap_set_scrolly(state->bg_tilemap, 0, data);
}

WRITE8_HANDLER( kopunch_gfxbank_w )
{
	kopunch_state *state = space->machine->driver_data<kopunch_state>();
	if (state->gfxbank != (data & 0x07))
	{
		state->gfxbank = data & 0x07;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	tilemap_set_flip(state->bg_tilemap, (data & 0x08) ? TILEMAP_FLIPY : 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	kopunch_state *state = machine->driver_data<kopunch_state>();
	int code = state->videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	kopunch_state *state = machine->driver_data<kopunch_state>();
	int code = (state->videoram2[tile_index] & 0x7f) + 128 * state->gfxbank;

	SET_TILE_INFO(1, code, 0, 0);
}

VIDEO_START( kopunch )
{
	kopunch_state *state = machine->driver_data<kopunch_state>();
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,  8,  8, 32, 32);
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 16, 16);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	tilemap_set_scrolldx(state->bg_tilemap, 16, 16);
}

VIDEO_UPDATE( kopunch )
{
	kopunch_state *state = screen->machine->driver_data<kopunch_state>();

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	return 0;
}
