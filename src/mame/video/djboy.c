/**
 * @file video/djboy.c
 *
 * video hardware for DJ Boy
 */
#include "emu.h"
#include "video/kan_pand.h"
#include "includes/djboy.h"

WRITE8_HANDLER( djboy_scrollx_w )
{
	djboy_state *state = space->machine->driver_data<djboy_state>();
	state->scrollx = data;
}

WRITE8_HANDLER( djboy_scrolly_w )
{
	djboy_state *state = space->machine->driver_data<djboy_state>();
	state->scrolly = data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	djboy_state *state = machine->driver_data<djboy_state>();
	UINT8 attr = state->videoram[tile_index + 0x800];
	int code = state->videoram[tile_index] + (attr & 0xf) * 256;
	int color = attr >> 4;

	if (color & 8)
		code |= 0x1000;

	SET_TILE_INFO(1, code, color, 0);	/* no flip */
}

WRITE8_HANDLER( djboy_videoram_w )
{
	djboy_state *state = space->machine->driver_data<djboy_state>();

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->background, offset & 0x7ff);
}

VIDEO_START( djboy )
{
	djboy_state *state = machine->driver_data<djboy_state>();
	state->background = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
}

WRITE8_HANDLER( djboy_paletteram_w )
{
	djboy_state *state = space->machine->driver_data<djboy_state>();
	int val;

	state->paletteram[offset] = data;
	offset &= ~1;
	val = (state->paletteram[offset] << 8) | state->paletteram[offset + 1];

	palette_set_color_rgb(space->machine, offset / 2, pal4bit(val >> 8), pal4bit(val >> 4), pal4bit(val >> 0));
}

VIDEO_UPDATE( djboy )
{
	/**
     * xx------ msb x
     * --x----- msb y
     * ---x---- flipscreen?
     * ----xxxx ROM bank
     */
	djboy_state *state = screen->machine->driver_data<djboy_state>();
	int scroll;

	scroll = state->scrollx | ((state->videoreg & 0xc0) << 2);
	tilemap_set_scrollx(state->background, 0, scroll - 0x391);

	scroll = state->scrolly | ((state->videoreg & 0x20) << 3);
	tilemap_set_scrolly(state->background, 0, scroll);

	tilemap_draw(bitmap, cliprect, state->background, 0, 0);
	pandora_update(state->pandora, bitmap, cliprect);

	return 0;
}

VIDEO_EOF( djboy )
{
	djboy_state *state = machine->driver_data<djboy_state>();
	pandora_eof(state->pandora);
}
