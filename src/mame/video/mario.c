/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"

#include "includes/mario.h"

static const res_net_decode_info mario_decode_info =
{
	1,		// there may be two proms needed to construct color
	0,		// start at 0
	255,	// end at 255
	//  R,   G,   B
	{   0,   0,   0},		// offsets
	{   5,   2,   0},		// shifts
	{0x07,0x07,0x03}	    // masks
};

static const res_net_info mario_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 2, {  470, 220,   0 } }  // dkong
	}
};

static const res_net_info mario_net_info_std =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 2, {  470, 220,   0 } }  // dkong
	}
};

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mario Bros. has a 512x8 palette PROM; interstingly, bytes 0-255 contain an
  inverted palette, as other Nintendo games like Donkey Kong, while bytes
  256-511 contain a non inverted palette. This was probably done to allow
  connection to both the special Nintendo and a standard monitor.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor -- inverter  -- RED
        -- 470 ohm resistor -- inverter  -- RED
        -- 1  kohm resistor -- inverter  -- RED
        -- 220 ohm resistor -- inverter  -- GREEN
        -- 470 ohm resistor -- inverter  -- GREEN
        -- 1  kohm resistor -- inverter  -- GREEN
        -- 220 ohm resistor -- inverter  -- BLUE
  bit 0 -- 470 ohm resistor -- inverter  -- BLUE

***************************************************************************/
PALETTE_INIT( mario )
{
	rgb_t	*rgb;

	rgb = compute_res_net_all(machine, color_prom, &mario_decode_info, &mario_net_info);
	palette_set_colors(machine, 0, rgb, 256);
	auto_free(machine, rgb);
	rgb = compute_res_net_all(machine, color_prom+256, &mario_decode_info, &mario_net_info_std);
	palette_set_colors(machine, 256, rgb, 256);
	auto_free(machine, rgb);

	palette_normalize_range(machine->palette, 0, 255, 0, 255);
	palette_normalize_range(machine->palette, 256, 511, 0, 255);
}

WRITE8_HANDLER( mario_videoram_w )
{
	mario_state	*state = (mario_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( mario_gfxbank_w )
{
	mario_state	*state = (mario_state *)space->machine->driver_data;

	if (state->gfx_bank != (data & 0x01))
	{
		state->gfx_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( mario_palettebank_w )
{
	mario_state	*state = (mario_state *)space->machine->driver_data;

	if (state->palette_bank != (data & 0x01))
	{
		state->palette_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( mario_scroll_w )
{
	mario_state	*state = (mario_state *)space->machine->driver_data;

	state->gfx_scroll = data + 17;
}

WRITE8_HANDLER( mario_flip_w )
{
	mario_state	*state = (mario_state *)space->machine->driver_data;

	if (state->flip != (data & 0x01))
	{
		state->flip = data & 0x01;
		if (state->flip)
			tilemap_set_flip_all(space->machine, TILEMAP_FLIPX | TILEMAP_FLIPY);
		else
			tilemap_set_flip_all(space->machine, 0);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mario_state	*state = (mario_state *)machine->driver_data;
	int code = state->videoram[tile_index] + 256 * state->gfx_bank;
	int color;

	color =  ((state->videoram[tile_index] >> 2) & 0x38) | 0x40 | (state->palette_bank<<7) | (state->monitor<<8);
	color = color >> 2;
	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( mario )
{
	mario_state	*state = (mario_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	state->gfx_bank = 0;
	state->palette_bank = 0;
	state->gfx_scroll = 0;
	state_save_register_global(machine, state->gfx_bank);
	state_save_register_global(machine, state->palette_bank);
	state_save_register_global(machine, state->gfx_scroll);
	state_save_register_global(machine, state->flip);
}

/*
 * Erratic line at top when scrolling down "Marios Bros" Title
 * confirmed on mametests.org as being present on real PCB as well.
 */

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/* TODO: draw_sprites should adopt the scanline logic from dkong.c
     * The schematics have the same logic for sprite buffering.
     */
	mario_state	*state = (mario_state *)machine->driver_data;
	int offs;

	for (offs = 0;offs < state->spriteram_size;offs += 4)
	{
		if (state->spriteram[offs])
		{
			int x, y;

			// from schematics ....
			y = (state->spriteram[offs] + (state->flip ? 0xF7 : 0xF9) + 1) & 0xFF;
			x = state->spriteram[offs+3];
			// sprite will be drawn if (y + scanline) & 0xF0 = 0xF0
			y = 240 - y; /* logical screen position */

			y = y ^ (state->flip ? 0xFF : 0x00); /* physical screen location */
			x = x ^ (state->flip ? 0xFF : 0x00); /* physical screen location */

			if (state->flip)
			{
				y -= 14;
				x -= 7;
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						state->spriteram[offs + 2],
						(state->spriteram[offs + 1] & 0x0f) + 16 * state->palette_bank + 32 * state->monitor,
						!(state->spriteram[offs + 1] & 0x80),!(state->spriteram[offs + 1] & 0x40),
						x, y,0);
			}
			else
			{
				y += 1;
				x -= 8;
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						state->spriteram[offs + 2],
						(state->spriteram[offs + 1] & 0x0f) + 16 * state->palette_bank + 32 * state->monitor,
						(state->spriteram[offs + 1] & 0x80),(state->spriteram[offs + 1] & 0x40),
						x, y,0);
			}
		}
	}
}

VIDEO_UPDATE( mario )
{
	mario_state	*state = (mario_state *)screen->machine->driver_data;
	int t;

	t = input_port_read(screen->machine, "MONITOR");
	if (t != state->monitor)
	{
		state->monitor = t;
		tilemap_mark_all_tiles_dirty_all(screen->machine);
	}

	tilemap_set_scrollx(state->bg_tilemap, 0, state->flip ? (HTOTAL-HBSTART) : 0);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->gfx_scroll - (state->flip ? 8 : 0));

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);

	return 0;
}
