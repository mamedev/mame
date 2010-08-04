/********************************************************************

    Ashita no Joe (Success Joe) [Wave]
    video hardware emulation

*********************************************************************/

#include "emu.h"
#include "includes/ashnojoe.h"


static TILE_GET_INFO( get_joe_tile_info )
{
	ashnojoe_state *state = machine->driver_data<ashnojoe_state>();
	int code = state->tileram[tile_index];

	SET_TILE_INFO(
			2,
			code & 0xfff,
			((code >> 12) & 0x0f),
			0);
}

static TILE_GET_INFO( get_joe_tile_info_2 )
{
	ashnojoe_state *state = machine->driver_data<ashnojoe_state>();
	int code = state->tileram_2[tile_index * 2];
	int attr = state->tileram_2[tile_index * 2 + 1];

	SET_TILE_INFO(
			4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x40,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_3 )
{
	ashnojoe_state *state = machine->driver_data<ashnojoe_state>();
	int code = state->tileram_3[tile_index];

	SET_TILE_INFO(
			0,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x10,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_4 )
{
	ashnojoe_state *state = machine->driver_data<ashnojoe_state>();
	int code = state->tileram_4[tile_index];

	SET_TILE_INFO(
			1,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x60,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_5 )
{
	ashnojoe_state *state = machine->driver_data<ashnojoe_state>();
	int code = state->tileram_5[tile_index * 2];
	int attr = state->tileram_5[tile_index * 2 + 1];

	SET_TILE_INFO(
			4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x20,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_6 )
{
	ashnojoe_state *state = machine->driver_data<ashnojoe_state>();
	int code = state->tileram_6[tile_index * 2];
	int attr = state->tileram_6[tile_index * 2 + 1];

	SET_TILE_INFO(
			3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}


static TILE_GET_INFO( get_joe_tile_info_7 )
{
	ashnojoe_state *state = machine->driver_data<ashnojoe_state>();
	int code = state->tileram_7[tile_index * 2];
	int attr = state->tileram_7[tile_index * 2 + 1];

	SET_TILE_INFO(
			3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}

WRITE16_HANDLER( ashnojoe_tileram_w )
{
	ashnojoe_state *state = space->machine->driver_data<ashnojoe_state>();

	state->tileram[offset] = data;
	tilemap_mark_tile_dirty(state->joetilemap, offset);
}


WRITE16_HANDLER( ashnojoe_tileram2_w )
{
	ashnojoe_state *state = space->machine->driver_data<ashnojoe_state>();

	state->tileram_2[offset] = data;
	tilemap_mark_tile_dirty(state->joetilemap2, offset / 2);
}

WRITE16_HANDLER( ashnojoe_tileram3_w )
{
	ashnojoe_state *state = space->machine->driver_data<ashnojoe_state>();

	state->tileram_3[offset] = data;
	tilemap_mark_tile_dirty(state->joetilemap3, offset);
}

WRITE16_HANDLER( ashnojoe_tileram4_w )
{
	ashnojoe_state *state = space->machine->driver_data<ashnojoe_state>();

	state->tileram_4[offset] = data;
	tilemap_mark_tile_dirty(state->joetilemap4, offset);
}

WRITE16_HANDLER( ashnojoe_tileram5_w )
{
	ashnojoe_state *state = space->machine->driver_data<ashnojoe_state>();

	state->tileram_5[offset] = data;
	tilemap_mark_tile_dirty(state->joetilemap5, offset / 2);
}

WRITE16_HANDLER( ashnojoe_tileram6_w )
{
	ashnojoe_state *state = space->machine->driver_data<ashnojoe_state>();

	state->tileram_6[offset] = data;
	tilemap_mark_tile_dirty(state->joetilemap6, offset / 2);
}

WRITE16_HANDLER( ashnojoe_tileram7_w )
{
	ashnojoe_state *state = space->machine->driver_data<ashnojoe_state>();

	state->tileram_7[offset] = data;
	tilemap_mark_tile_dirty(state->joetilemap7, offset / 2);
}

WRITE16_HANDLER( joe_tilemaps_xscroll_w )
{
	ashnojoe_state *state = space->machine->driver_data<ashnojoe_state>();

	switch( offset )
	{
	case 0:
		tilemap_set_scrollx(state->joetilemap3, 0, data);
		break;
	case 1:
		tilemap_set_scrollx(state->joetilemap5, 0, data);
		break;
	case 2:
		tilemap_set_scrollx(state->joetilemap2, 0, data);
		break;
	case 3:
		tilemap_set_scrollx(state->joetilemap4, 0, data);
		break;
	case 4:
		tilemap_set_scrollx(state->joetilemap6, 0, data);
		tilemap_set_scrollx(state->joetilemap7, 0, data);
		break;
	}
}

WRITE16_HANDLER( joe_tilemaps_yscroll_w )
{
	ashnojoe_state *state = space->machine->driver_data<ashnojoe_state>();

	switch( offset )
	{
	case 0:
		tilemap_set_scrolly(state->joetilemap3, 0, data);
		break;
	case 1:
		tilemap_set_scrolly(state->joetilemap5, 0, data);
		break;
	case 2:
		tilemap_set_scrolly(state->joetilemap2, 0, data);
		break;
	case 3:
		tilemap_set_scrolly(state->joetilemap4, 0, data);
		break;
	case 4:
		tilemap_set_scrolly(state->joetilemap6, 0, data);
		tilemap_set_scrolly(state->joetilemap7, 0, data);
		break;
	}
}

VIDEO_START( ashnojoe )
{
	ashnojoe_state *state = machine->driver_data<ashnojoe_state>();

	state->joetilemap = tilemap_create(machine, get_joe_tile_info,  tilemap_scan_rows, 8, 8, 64, 32);
	state->joetilemap2 = tilemap_create(machine, get_joe_tile_info_2, tilemap_scan_rows, 16, 16, 32, 32);
	state->joetilemap3 = tilemap_create(machine, get_joe_tile_info_3, tilemap_scan_rows, 8, 8, 64, 64);
	state->joetilemap4 = tilemap_create(machine, get_joe_tile_info_4, tilemap_scan_rows, 8, 8, 64, 64);
	state->joetilemap5 = tilemap_create(machine, get_joe_tile_info_5, tilemap_scan_rows, 16, 16, 32, 32);
	state->joetilemap6 = tilemap_create(machine, get_joe_tile_info_6, tilemap_scan_rows, 16, 16, 32, 32);
	state->joetilemap7 = tilemap_create(machine, get_joe_tile_info_7, tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->joetilemap,  15);
	tilemap_set_transparent_pen(state->joetilemap2, 15);
	tilemap_set_transparent_pen(state->joetilemap3, 15);
	tilemap_set_transparent_pen(state->joetilemap4, 15);
	tilemap_set_transparent_pen(state->joetilemap5, 15);
}

VIDEO_UPDATE( ashnojoe )
{
	ashnojoe_state *state = screen->machine->driver_data<ashnojoe_state>();

	//state->tilemap_reg[0] & 0x10 // ?? on coin insertion

	flip_screen_set(screen->machine, state->tilemap_reg[0] & 1);

	if(state->tilemap_reg[0] & 0x02)
		tilemap_draw(bitmap, cliprect, state->joetilemap7, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->joetilemap6, 0, 0);

	tilemap_draw(bitmap, cliprect, state->joetilemap4, 0, 0);
	tilemap_draw(bitmap, cliprect, state->joetilemap2, 0, 0);
	tilemap_draw(bitmap, cliprect, state->joetilemap5, 0, 0);
	tilemap_draw(bitmap, cliprect, state->joetilemap3, 0, 0);
	tilemap_draw(bitmap, cliprect, state->joetilemap, 0, 0);

	return 0;
}
