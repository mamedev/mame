/***************************************************************************

    Car Jamboree
    Omori Electric CAD (OEC) 1981

***************************************************************************/

#include "driver.h"
#include "includes/carjmbre.h"

PALETTE_INIT( carjmbre )
{
	int i, bit0, bit1, bit2, r, g, b;

	for (i = 0; i < machine->config->total_colors; i++)
	{
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

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( carjmbre_flipscreen_w )
{
	carjmbre_state *state = (carjmbre_state *)space->machine->driver_data;

	state->flipscreen = data ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
	tilemap_set_flip_all(space->machine, state->flipscreen);
}

WRITE8_HANDLER( carjmbre_bgcolor_w )
{
	carjmbre_state *state = (carjmbre_state *)space->machine->driver_data;
	int oldbg, i;

	oldbg = state->bgcolor;

	state->bgcolor &= 0xff00 >> (offset * 8);
	state->bgcolor |= ((~data) & 0xff) << (offset * 8);

	if ( oldbg != state->bgcolor)
	{
		for (i = 0; i < 64; i += 4)
			palette_set_color_rgb(space->machine, i, (state->bgcolor & 0xff) * 0x50,
						(state->bgcolor & 0xff) * 0x50, (state->bgcolor & 0xff)!=0 ? 0x50 : 0);
	}
}

static TILE_GET_INFO( get_carjmbre_tile_info )
{
	carjmbre_state *state = (carjmbre_state *)machine->driver_data;
	UINT32 tile_number = state->videoram[tile_index] & 0xff;
	UINT8 attr  = state->videoram[tile_index + 0x400];
	tile_number += (attr & 0x80) << 1; /* bank */
	SET_TILE_INFO(
			0,
			tile_number,
			(attr & 0x7),
			0);
}

WRITE8_HANDLER( carjmbre_videoram_w )
{
	carjmbre_state *state = (carjmbre_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->cj_tilemap, offset & 0x3ff);
}



VIDEO_START( carjmbre )
{
	carjmbre_state *state = (carjmbre_state *)machine->driver_data;

	state->cj_tilemap = tilemap_create(machine, get_carjmbre_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state_save_register_global(machine, state->flipscreen);
	state_save_register_global(machine, state->bgcolor);
}

VIDEO_UPDATE( carjmbre )
{
	carjmbre_state *state = (carjmbre_state *)screen->machine->driver_data;
	int offs, troffs, sx, sy, flipx, flipy;

	//colorram
	//76543210
	//x------- graphic bank
	//-xxx---- unused
	//----x--- ?? probably colour, only used for ramp and pond
	//-----xxx colour

	tilemap_draw(bitmap, cliprect, state->cj_tilemap, 0, 0);

	//spriteram[offs]
	//+0       y pos
	//+1       sprite number
	//+2
	//76543210
	//x------- flipy
	//-x------ flipx
	//--xx---- unused
	//----x--- ?? probably colour
	//-----xxx colour
	//+3       x pos
	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		//before copying the sprites to spriteram the game reorders the first
		//sprite to last, sprite ordering is incorrect if this isn't undone
		troffs = (offs - 4 + state->spriteram_size) % state->spriteram_size;

		//unused sprites are marked with ypos <= 0x02 (or >= 0xfd if screen flipped)
		if (state->spriteram[troffs] > 0x02 && state->spriteram[troffs] < 0xfd)
		{
			{
				sx = state->spriteram[troffs + 3] - 7;
				sy = 241 - state->spriteram[troffs];
				flipx = (state->spriteram[troffs + 2] & 0x40) >> 6;
				flipy = (state->spriteram[troffs + 2] & 0x80) >> 7;

				if (state->flipscreen)
				{
					sx = (256 + (226 - sx)) % 256;
					sy = 242 - sy;
					flipx = !flipx;
					flipy = !flipy;
				}

				drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1],
						state->spriteram[troffs + 1],
						state->spriteram[troffs + 2] & 0x07,
						flipx,flipy,
						sx,sy,0);
			}
		}
	}
	return 0;
}
