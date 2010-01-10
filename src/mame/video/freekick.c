/* Free Kick Video Hardware */

#include "emu.h"
#include "includes/freekick.h"


static TILE_GET_INFO( get_freek_tile_info )
{
	freekick_state *state = (freekick_state *)machine->driver_data;
	int tileno, palno;

	tileno = state->videoram[tile_index] + ((state->videoram[tile_index + 0x400] & 0xe0) << 3);
	palno = state->videoram[tile_index + 0x400] & 0x1f;
	SET_TILE_INFO(0, tileno, palno, 0);
}


VIDEO_START( freekick )
{
	freekick_state *state = (freekick_state *)machine->driver_data;
	state->freek_tilemap = tilemap_create(machine, get_freek_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}


WRITE8_HANDLER( freek_videoram_w )
{
	freekick_state *state = (freekick_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->freek_tilemap, offset & 0x3ff);
}

static void gigas_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	freekick_state *state = (freekick_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int xpos = state->spriteram[offs + 3];
		int ypos = state->spriteram[offs + 2];
		int code = state->spriteram[offs + 0] | ((state->spriteram[offs + 1] & 0x20) << 3);

		int flipx = 0;
		int flipy = 0;
		int color = state->spriteram[offs + 1] & 0x1f;

		if (flip_screen_x_get(machine))
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (flip_screen_y_get(machine))
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				xpos,240-ypos,0);
	}
}


static void pbillrd_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	freekick_state *state = (freekick_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int xpos = state->spriteram[offs + 3];
		int ypos = state->spriteram[offs + 2];
		int code = state->spriteram[offs + 0];

		int flipx = 0;//state->spriteram[offs + 0] & 0x80; //?? unused ?
		int flipy = 0;//state->spriteram[offs + 0] & 0x40;
		int color = state->spriteram[offs + 1] & 0x0f;

		if (flip_screen_x_get(machine))
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (flip_screen_y_get(machine))
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				xpos,240-ypos,0);
	}
}



static void freekick_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	freekick_state *state = (freekick_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int xpos = state->spriteram[offs + 3];
		int ypos = state->spriteram[offs + 0];
		int code = state->spriteram[offs + 1] + ((state->spriteram[offs + 2] & 0x20) << 3);

		int flipx = state->spriteram[offs + 2] & 0x80;	//?? unused ?
		int flipy = state->spriteram[offs + 2] & 0x40;
		int color = state->spriteram[offs + 2] & 0x1f;

		if (flip_screen_x_get(machine))
		{
			xpos = 240 - xpos;
			flipx = !flipx;
		}
		if (flip_screen_y_get(machine))
		{
			ypos = 256 - ypos;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				xpos,248-ypos,0);
	}
}

VIDEO_UPDATE( gigas )
{
	freekick_state *state = (freekick_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->freek_tilemap, 0, 0);
	gigas_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( pbillrd )
{
	freekick_state *state = (freekick_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->freek_tilemap, 0, 0);
	pbillrd_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( freekick )
{
	freekick_state *state = (freekick_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->freek_tilemap, 0, 0);
	freekick_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
