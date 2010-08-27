/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/snk6502.h"


#define TOTAL_COLORS(m,gfxn) ((m)->gfx[gfxn]->total_colors * (m)->gfx[gfxn]->color_granularity)
#define COLOR(m,gfxn,offs) ((m)->config->m_gfxdecodeinfo[gfxn].color_codes_start + offs)



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Zarzon has a different PROM layout from the others.

***************************************************************************/
PALETTE_INIT( snk6502 )
{
	snk6502_state *state = machine->driver_data<snk6502_state>();
	int i;

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

		state->palette[i] = MAKE_RGB(r, g, b);

		color_prom++;
	}

	state->backcolor = 0;	/* background color can be changed by the game */

	for (i = 0; i < TOTAL_COLORS(machine,0); i++)
		palette_set_color(machine, COLOR(machine, 0, i), state->palette[i]);

	for (i = 0; i < TOTAL_COLORS(machine,1); i++)
	{
		if (i % 4 == 0)
			palette_set_color(machine, COLOR(machine, 1, i), state->palette[4 * state->backcolor + 0x20]);
		else
			palette_set_color(machine, COLOR(machine, 1, i), state->palette[i + 0x20]);
	}
}

WRITE8_HANDLER( snk6502_videoram_w )
{
	snk6502_state *state = space->machine->driver_data<snk6502_state>();

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( snk6502_videoram2_w )
{
	snk6502_state *state = space->machine->driver_data<snk6502_state>();

	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( snk6502_colorram_w )
{
	snk6502_state *state = space->machine->driver_data<snk6502_state>();

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( snk6502_charram_w )
{
	snk6502_state *state = space->machine->driver_data<snk6502_state>();

	if (state->charram[offset] != data)
	{
		state->charram[offset] = data;
		gfx_element_mark_dirty(space->machine->gfx[0], (offset/8) % 256);
	}
}


WRITE8_HANDLER( snk6502_flipscreen_w )
{
	snk6502_state *state = space->machine->driver_data<snk6502_state>();
	int bank;

	/* bits 0-2 select background color */

	if (state->backcolor != (data & 7))
	{
		int i;

		state->backcolor = data & 7;

		for (i = 0;i < 32;i += 4)
			palette_set_color(space->machine, COLOR(space->machine, 1, i), state->palette[4 * state->backcolor + 0x20]);
	}

	/* bit 3 selects char bank */

	bank = (~data & 0x08) >> 3;

	if (state->charbank != bank)
	{
		state->charbank = bank;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}

	/* bit 7 flips screen */

	if (flip_screen_get(space->machine) != (data & 0x80))
	{
		flip_screen_set(space->machine, data & 0x80);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( snk6502_scrollx_w )
{
	snk6502_state *state = space->machine->driver_data<snk6502_state>();

	tilemap_set_scrollx(state->bg_tilemap, 0, data);
}

WRITE8_HANDLER( snk6502_scrolly_w )
{
	snk6502_state *state = space->machine->driver_data<snk6502_state>();

	tilemap_set_scrolly(state->bg_tilemap, 0, data);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	snk6502_state *state = machine->driver_data<snk6502_state>();
	int code = state->videoram[tile_index] + 256 * state->charbank;
	int color = (state->colorram[tile_index] & 0x38) >> 3;

	SET_TILE_INFO(1, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	snk6502_state *state = machine->driver_data<snk6502_state>();
	int code = state->videoram2[tile_index];
	int color = state->colorram[tile_index] & 0x07;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( snk6502 )
{
	snk6502_state *state = machine->driver_data<snk6502_state>();

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	gfx_element_set_source(machine->gfx[0], state->charram);
}

VIDEO_START( pballoon )
{
	snk6502_state *state = machine->driver_data<snk6502_state>();

	VIDEO_START_CALL( snk6502 );

	tilemap_set_scrolldy(state->bg_tilemap, -24, 0);
	tilemap_set_scrolldy(state->fg_tilemap, -24, 0);
}


VIDEO_UPDATE( snk6502 )
{
	snk6502_state *state = screen->machine->driver_data<snk6502_state>();

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}

/* Satan of Saturn */

PALETTE_INIT( satansat )
{
	snk6502_state *state = machine->driver_data<snk6502_state>();
	int i;

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

		state->palette[i] = MAKE_RGB(r, g, b);

		color_prom++;
	}

	state->backcolor = 0;	/* background color can be changed by the game */

	for (i = 0; i < TOTAL_COLORS(machine,0); i++)
		palette_set_color(machine, COLOR(machine, 0, i), state->palette[4 * (i % 4) + (i / 4)]);

	for (i = 0; i < TOTAL_COLORS(machine,1); i++)
	{
		if (i % 4 == 0)
			palette_set_color(machine, COLOR(machine, 1, i), state->palette[state->backcolor + 0x10]);
		else
			palette_set_color(machine, COLOR(machine, 1, i), state->palette[4 * (i % 4) + (i / 4) + 0x10]);
	}
}

WRITE8_HANDLER( satansat_b002_w )
{
	/* bit 0 flips screen */

	if (flip_screen_get(space->machine) != (data & 0x01))
	{
		flip_screen_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}

	/* bit 1 enables interrups */
	/* it controls only IRQs, not NMIs. Here I am affecting both, which */
	/* is wrong. */

	interrupt_enable_w(space,0,data & 0x02);

	/* other bits unused */
}

WRITE8_HANDLER( satansat_backcolor_w )
{
	snk6502_state *state = space->machine->driver_data<snk6502_state>();

	/* bits 0-1 select background color. Other bits unused. */

	if (state->backcolor != (data & 0x03))
	{
		int i;

		state->backcolor = data & 0x03;

		for (i = 0; i < 16; i += 4)
			palette_set_color(space->machine, COLOR(space->machine, 1, i), state->palette[state->backcolor + 0x10]);
	}
}

static TILE_GET_INFO( satansat_get_bg_tile_info )
{
	snk6502_state *state = machine->driver_data<snk6502_state>();
	int code = state->videoram[tile_index];
	int color = (state->colorram[tile_index] & 0x0c) >> 2;

	SET_TILE_INFO(1, code, color, 0);
}

static TILE_GET_INFO( satansat_get_fg_tile_info )
{
	snk6502_state *state = machine->driver_data<snk6502_state>();
	int code = state->videoram2[tile_index];
	int color = state->colorram[tile_index] & 0x03;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( satansat )
{
	snk6502_state *state = machine->driver_data<snk6502_state>();

	state->bg_tilemap = tilemap_create(machine, satansat_get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, satansat_get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	gfx_element_set_source(machine->gfx[0], state->charram);
}
