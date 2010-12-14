/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/flstory.h"

static TILE_GET_INFO( get_tile_info )
{
	flstory_state *state = machine->driver_data<flstory_state>();
	int code = state->videoram[tile_index * 2];
	int attr = state->videoram[tile_index * 2 + 1];
	int tile_number = code + ((attr & 0xc0) << 2) + 0x400 + 0x800 * state->char_bank;
	int flags = TILE_FLIPYX((attr & 0x18) >> 3);
	tileinfo->category = (attr & 0x20) >> 5;
	tileinfo->group = (attr & 0x20) >> 5;
	SET_TILE_INFO(
			0,
			tile_number,
			attr & 0x0f,
			flags);
}

static TILE_GET_INFO( victnine_get_tile_info )
{
	flstory_state *state = machine->driver_data<flstory_state>();
	int code = state->videoram[tile_index * 2];
	int attr = state->videoram[tile_index * 2 + 1];
	int tile_number = ((attr & 0x38) << 5) + code;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(
			0,
			tile_number,
			attr & 0x07,
			flags);
}

static TILE_GET_INFO( get_rumba_tile_info )
{
	flstory_state *state = machine->driver_data<flstory_state>();
	int code = state->videoram[tile_index * 2];
	int attr = state->videoram[tile_index * 2 + 1];
	int tile_number = code + ((attr & 0xc0) << 2) + 0x400 + 0x800 * state->char_bank;
	int col = (attr & 0x0f);

	tileinfo->category = (attr & 0x20) >> 5;
	tileinfo->group = (attr & 0x20) >> 5;
	SET_TILE_INFO(
			0,
			tile_number,
			col,
			0);
}

VIDEO_START( flstory )
{
	flstory_state *state = machine->driver_data<flstory_state>();
	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
//  tilemap_set_transparent_pen(state->bg_tilemap, 15);
	tilemap_set_transmask(state->bg_tilemap, 0, 0x3fff, 0xc000); /* split type 0 has pens 0-13 transparent in front half */
	tilemap_set_transmask(state->bg_tilemap, 1, 0x8000, 0x7fff); /* split type 1 has pen 15 transparent in front half */
	tilemap_set_scroll_cols(state->bg_tilemap, 32);

	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x200);
	machine->generic.paletteram2.u8 = auto_alloc_array(machine, UINT8, 0x200);
	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x200);
	state_save_register_global_pointer(machine, machine->generic.paletteram2.u8, 0x200);
}

VIDEO_START( rumba )
{
	flstory_state *state = machine->driver_data<flstory_state>();
	state->bg_tilemap = tilemap_create(machine, get_rumba_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
//  tilemap_set_transparent_pen(state->bg_tilemap, 15);
	tilemap_set_transmask(state->bg_tilemap, 0, 0x3fff, 0xc000); /* split type 0 has pens 0-13 transparent in front half */
	tilemap_set_transmask(state->bg_tilemap, 1, 0x8000, 0x7fff); /* split type 1 has pen 15 transparent in front half */
	tilemap_set_scroll_cols(state->bg_tilemap, 32);

	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x200);
	machine->generic.paletteram2.u8 = auto_alloc_array(machine, UINT8, 0x200);
	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x200);
	state_save_register_global_pointer(machine, machine->generic.paletteram2.u8, 0x200);
}

VIDEO_START( victnine )
{
	flstory_state *state = machine->driver_data<flstory_state>();
	state->bg_tilemap = tilemap_create(machine, victnine_get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(state->bg_tilemap, 32);

	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x200);
	machine->generic.paletteram2.u8 = auto_alloc_array(machine, UINT8, 0x200);
	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x200);
	state_save_register_global_pointer(machine, machine->generic.paletteram2.u8, 0x200);
}

WRITE8_HANDLER( flstory_videoram_w )
{
	flstory_state *state = space->machine->driver_data<flstory_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE8_HANDLER( flstory_palette_w )
{
	flstory_state *state = space->machine->driver_data<flstory_state>();
	if (offset & 0x100)
		paletteram_xxxxBBBBGGGGRRRR_split2_w(space, (offset & 0xff) + (state->palette_bank << 8),data);
	else
		paletteram_xxxxBBBBGGGGRRRR_split1_w(space, (offset & 0xff) + (state->palette_bank << 8),data);
}

READ8_HANDLER( flstory_palette_r )
{
	flstory_state *state = space->machine->driver_data<flstory_state>();
	if (offset & 0x100)
		return space->machine->generic.paletteram2.u8[ (offset & 0xff) + (state->palette_bank << 8) ];
	else
		return space->machine->generic.paletteram.u8  [ (offset & 0xff) + (state->palette_bank << 8) ];
}

WRITE8_HANDLER( flstory_gfxctrl_w )
{
	flstory_state *state = space->machine->driver_data<flstory_state>();
	if (state->gfxctrl == data)
		return;
	state->gfxctrl = data;

	state->flipscreen = (~data & 0x01);
	if (state->char_bank != ((data & 0x10) >> 4))
	{
		state->char_bank = (data & 0x10) >> 4;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
	state->palette_bank = (data & 0x20) >> 5;

	flip_screen_set(space->machine, state->flipscreen);

//popmessage("%04x: gfxctrl = %02x\n", cpu_get_pc(space->cpu), data);

}

READ8_HANDLER( victnine_gfxctrl_r )
{
	flstory_state *state = space->machine->driver_data<flstory_state>();
	return state->gfxctrl;
}

WRITE8_HANDLER( victnine_gfxctrl_w )
{
	flstory_state *state = space->machine->driver_data<flstory_state>();
	if (state->gfxctrl == data)
		return;
	state->gfxctrl = data;

	state->palette_bank = (data & 0x20) >> 5;

	if (data & 0x04)
	{
		state->flipscreen = (data & 0x01);
		flip_screen_set(space->machine, state->flipscreen);
	}

//popmessage("%04x: gfxctrl = %02x\n", cpu_get_pc(space->cpu), data);

}

WRITE8_HANDLER( flstory_scrlram_w )
{
	flstory_state *state = space->machine->driver_data<flstory_state>();
	state->scrlram[offset] = data;
	tilemap_set_scrolly(state->bg_tilemap, offset, data);
}


static void flstory_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri )
{
	flstory_state *state = machine->driver_data<flstory_state>();
	int i;

	for (i = 0; i < 0x20; i++)
	{
		int pr = state->spriteram[state->spriteram_size - 1 - i];
		int offs = (pr & 0x1f) * 4;

		if ((pr & 0x80) == pri)
		{
			int code, sx, sy, flipx, flipy;

			code = state->spriteram[offs + 2] + ((state->spriteram[offs + 1] & 0x30) << 4);
			sx = state->spriteram[offs + 3];
			sy = state->spriteram[offs + 0];

			if (state->flipscreen)
			{
				sx = (240 - sx) & 0xff ;
				sy = sy - 1 ;
			}
			else
				sy = 240 - sy - 1 ;

			flipx = ((state->spriteram[offs + 1] & 0x40) >> 6) ^ state->flipscreen;
			flipy = ((state->spriteram[offs + 1] & 0x80) >> 7) ^ state->flipscreen;

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					code,
					state->spriteram[offs + 1] & 0x0f,
					flipx,flipy,
					sx,sy,15);
			/* wrap around */
			if (sx > 240)
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						code,
						state->spriteram[offs + 1] & 0x0f,
						flipx,flipy,
						sx-256,sy,15);
		}
	}
}

VIDEO_UPDATE( flstory )
{
	flstory_state *state = screen->machine->driver_data<flstory_state>();
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0 | TILEMAP_DRAW_LAYER1, 0);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 1 | TILEMAP_DRAW_LAYER1, 0);
	flstory_draw_sprites(screen->machine, bitmap, cliprect, 0x00);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0 | TILEMAP_DRAW_LAYER0, 0);
	flstory_draw_sprites(screen->machine, bitmap, cliprect, 0x80);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 1 | TILEMAP_DRAW_LAYER0, 0);
	return 0;
}

static void victnine_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	flstory_state *state = machine->driver_data<flstory_state>();
	int i;

	for (i = 0; i < 0x20; i++)
	{
		int pr = state->spriteram[state->spriteram_size - 1 - i];
		int offs = (pr & 0x1f) * 4;

		//if ((pr & 0x80) == pri)
		{
			int code, sx, sy, flipx, flipy;

			code = state->spriteram[offs + 2] + ((state->spriteram[offs + 1] & 0x20) << 3);
			sx = state->spriteram[offs + 3];
			sy = state->spriteram[offs + 0];

			if (state->flipscreen)
			{
				sx = (240 - sx + 1) & 0xff ;
				sy = sy + 1 ;
			}
			else
				sy = 240 - sy + 1 ;

			flipx = ((state->spriteram[offs + 1] & 0x40) >> 6) ^ state->flipscreen;
			flipy = ((state->spriteram[offs + 1] & 0x80) >> 7) ^ state->flipscreen;

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					code,
					state->spriteram[offs + 1] & 0x0f,
					flipx,flipy,
					sx,sy,15);
			/* wrap around */
			if (sx > 240)
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						code,
						state->spriteram[offs + 1] & 0x0f,
						flipx,flipy,
						sx-256,sy,15);
		}
	}
}

VIDEO_UPDATE( victnine )
{
	flstory_state *state = screen->machine->driver_data<flstory_state>();
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	victnine_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( rumba )
{
	flstory_state *state = screen->machine->driver_data<flstory_state>();
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0 | TILEMAP_DRAW_LAYER1, 0);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 1 | TILEMAP_DRAW_LAYER1, 0);
	victnine_draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0 | TILEMAP_DRAW_LAYER0, 0);
	victnine_draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 1 | TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
