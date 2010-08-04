/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "emu.h"
#include "includes/ladyfrog.h"


WRITE8_HANDLER(ladyfrog_spriteram_w)
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();
	state->spriteram[offset] = data;
}

READ8_HANDLER(ladyfrog_spriteram_r)
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();
	return state->spriteram[offset];
}

static TILE_GET_INFO( get_tile_info )
{
	ladyfrog_state *state = machine->driver_data<ladyfrog_state>();
	int pal = state->videoram[tile_index * 2 + 1] & 0x0f;
	int tile = state->videoram[tile_index * 2] + ((state->videoram[tile_index * 2 + 1] & 0xc0) << 2)+ ((state->videoram[tile_index * 2 + 1] & 0x30) << 6);
	SET_TILE_INFO(
			0,
			tile + 0x1000 * state->tilebank,
			pal,TILE_FLIPY
			);
}

WRITE8_HANDLER( ladyfrog_videoram_w )
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset >> 1);
}

READ8_HANDLER( ladyfrog_videoram_r )
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();
	return state->videoram[offset];
}

WRITE8_HANDLER( ladyfrog_palette_w )
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();

	if (offset & 0x100)
		paletteram_xxxxBBBBGGGGRRRR_split2_w(space, (offset & 0xff) + (state->palette_bank << 8), data);
	else
		paletteram_xxxxBBBBGGGGRRRR_split1_w(space, (offset & 0xff) + (state->palette_bank << 8), data);
}

READ8_HANDLER( ladyfrog_palette_r )
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();

	if (offset & 0x100)
		return space->machine->generic.paletteram2.u8[(offset & 0xff) + (state->palette_bank << 8)];
	else
		return space->machine->generic.paletteram.u8[(offset & 0xff) + (state->palette_bank << 8)];
}

WRITE8_HANDLER( ladyfrog_gfxctrl_w )
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();
	state->palette_bank = (data & 0x20) >> 5;
}

WRITE8_HANDLER( ladyfrog_gfxctrl2_w )
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();
	state->tilebank = ((data & 0x18) >> 3) ^ 3;
	tilemap_mark_all_tiles_dirty(state->bg_tilemap);
}


#ifdef UNUSED_FUNCTION
int gfxctrl;

READ8_HANDLER( ladyfrog_gfxctrl_r )
{
	return gfxctrl;
}
#endif

READ8_HANDLER( ladyfrog_scrlram_r )
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();
	return state->scrlram[offset];
}

WRITE8_HANDLER( ladyfrog_scrlram_w )
{
	ladyfrog_state *state = space->machine->driver_data<ladyfrog_state>();

	state->scrlram[offset] = data;
	tilemap_set_scrolly(state->bg_tilemap, offset, data);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	ladyfrog_state *state = machine->driver_data<ladyfrog_state>();
	int i;
	for (i = 0; i < 0x20; i++)
	{
		int pr = state->spriteram[0x9f - i];
		int offs = (pr & 0x1f) * 4;
		{
			int code, sx, sy, flipx, flipy, pal;
			code = state->spriteram[offs + 2] + ((state->spriteram[offs + 1] & 0x10) << 4) + state->spritetilebase;
			pal = state->spriteram[offs + 1] & 0x0f;
			sx = state->spriteram[offs + 3];
			sy = 238 - state->spriteram[offs + 0];
			flipx = ((state->spriteram[offs + 1] & 0x40)>>6);
			flipy = ((state->spriteram[offs + 1] & 0x80)>>7);
			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					code,
					pal,
					flipx,flipy,
					sx,sy,15);

			if (state->spriteram[offs + 3] > 240)
			{
				sx = (state->spriteram[offs + 3] - 256);
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
        				code,
				        pal,
				        flipx,flipy,
					      sx,sy,15);
			}
		}
	}
}

static VIDEO_START( ladyfrog_common )
{
	ladyfrog_state *state = machine->driver_data<ladyfrog_state>();

	state->spriteram = auto_alloc_array(machine, UINT8, 160);
	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x200);
	machine->generic.paletteram2.u8 = auto_alloc_array(machine, UINT8, 0x200);
	tilemap_set_scroll_cols(state->bg_tilemap, 32);
	tilemap_set_scrolldy(state->bg_tilemap, 15, 15);

	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x200);
	state_save_register_global_pointer(machine, machine->generic.paletteram2.u8, 0x200);
	state_save_register_global_pointer(machine, state->spriteram, 160);
}

VIDEO_START( ladyfrog )
{
	ladyfrog_state *state = machine->driver_data<ladyfrog_state>();

	// weird, there are sprite tiles at 0x000 and 0x400, but they don't contain all the sprites!
	state->spritetilebase = 0x800;
	VIDEO_START_CALL(ladyfrog_common);
}

VIDEO_START( toucheme )
{
	ladyfrog_state *state = machine->driver_data<ladyfrog_state>();

	state->spritetilebase = 0x000;
	VIDEO_START_CALL(ladyfrog_common);
}


VIDEO_UPDATE( ladyfrog )
{
	ladyfrog_state *state = screen->machine->driver_data<ladyfrog_state>();

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
