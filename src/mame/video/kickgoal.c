/* Kick Goal - video */

#include "driver.h"
#include "includes/kickgoal.h"


WRITE16_HANDLER( kickgoal_fgram_w )
{
	kickgoal_state *state = (kickgoal_state *)space->machine->driver_data;
	state->fgram[offset] = data;
	tilemap_mark_tile_dirty(state->fgtm, offset / 2);
}

WRITE16_HANDLER( kickgoal_bgram_w )
{
	kickgoal_state *state = (kickgoal_state *)space->machine->driver_data;
	state->bgram[offset] = data;
	tilemap_mark_tile_dirty(state->bgtm, offset / 2);
}

WRITE16_HANDLER( kickgoal_bg2ram_w )
{
	kickgoal_state *state = (kickgoal_state *)space->machine->driver_data;
	state->bg2ram[offset] = data;
	tilemap_mark_tile_dirty(state->bg2tm, offset / 2);
}



/* FG */
static TILE_GET_INFO( get_kickgoal_fg_tile_info )
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;
	int tileno = state->fgram[tile_index * 2] & 0x0fff;
	int color = state->fgram[tile_index * 2 + 1] & 0x000f;

	SET_TILE_INFO(0, tileno + 0x7000, color + 0x00, 0);
}

/* BG */
static TILE_GET_INFO( get_kickgoal_bg_tile_info )
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;
	int tileno = state->bgram[tile_index * 2] & 0x0fff;
	int color = state->bgram[tile_index * 2 + 1] & 0x000f;

	SET_TILE_INFO(1, tileno + 0x1000, color + 0x10, 0);
}

/* BG 2 */
static TILE_GET_INFO( get_kickgoal_bg2_tile_info )
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;
	int tileno = state->bg2ram[tile_index * 2] & 0x07ff;
	int color = state->bg2ram[tile_index * 2 + 1] & 0x000f;
	int flipx = state->bg2ram[tile_index * 2 + 1] & 0x0020;

	SET_TILE_INFO(2, tileno + 0x800, color + 0x20, flipx ? TILE_FLIPX : 0);
}


static TILEMAP_MAPPER( tilemap_scan_kicksbg2 )
{
	/* logical (col,row) -> memory offset */
	return col * 8 + (row & 0x7) + ((row & 0x3c) >> 3) * 0x200;
}

static TILEMAP_MAPPER( tilemap_scan_kicksbg )
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}

static TILEMAP_MAPPER( tilemap_scan_kicksfg )
{
	/* logical (col,row) -> memory offset */
	return col * 32 + (row & 0x1f) + ((row & 0x20) >> 5) * 0x800;
}


VIDEO_START( kickgoal )
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;

	state->fgtm = tilemap_create(machine, get_kickgoal_fg_tile_info, tilemap_scan_kicksfg, 8, 16, 64, 64);
	state->bgtm = tilemap_create(machine, get_kickgoal_bg_tile_info, tilemap_scan_kicksbg, 16, 32, 64, 64);
	state->bg2tm = tilemap_create(machine, get_kickgoal_bg2_tile_info, tilemap_scan_kicksbg2, 32, 64, 64, 64);

	tilemap_set_transparent_pen(state->fgtm, 15);
	tilemap_set_transparent_pen(state->bgtm, 15);
}



static void kickgoal_draw_sprites( running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	const gfx_element *gfx = machine->gfx[1];
	int offs;

	for (offs = 0; offs < state->spriteram_size / 2; offs += 4)
	{
		int xpos = spriteram[offs + 3];
		int ypos = spriteram[offs + 0] & 0x00ff;
		int tileno = spriteram[offs + 2] & 0x0fff;
		int flipx = spriteram[offs + 1] & 0x0020;
		int color = spriteram[offs + 1] & 0x000f;

		if (spriteram[offs + 0] & 0x0100)
			break;

		ypos *= 2;
		ypos = 0x200 - ypos;

		drawgfx_transpen(bitmap,cliprect,gfx,
				tileno,
				0x30 + color,
				flipx,0,
				xpos-16+4,ypos-32,15);
	}
}


VIDEO_UPDATE( kickgoal )
{
	kickgoal_state *state = (kickgoal_state *)screen->machine->driver_data;

	/* set scroll */
	tilemap_set_scrollx(state->fgtm, 0, state->scrram[0]);
	tilemap_set_scrolly(state->fgtm, 0, state->scrram[1] * 2);
	tilemap_set_scrollx(state->bgtm, 0, state->scrram[2]);
	tilemap_set_scrolly(state->bgtm, 0, state->scrram[3] * 2);
	tilemap_set_scrollx(state->bg2tm, 0, state->scrram[4]);
	tilemap_set_scrolly(state->bg2tm, 0, state->scrram[5] * 2);

	/* draw */
	tilemap_draw(bitmap, cliprect, state->bg2tm, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bgtm, 0, 0);

	kickgoal_draw_sprites(screen->machine, bitmap, cliprect);

	tilemap_draw(bitmap, cliprect, state->fgtm, 0, 0);

	/*
    popmessage ("Regs %04x %04x %04x %04x %04x %04x %04x %04x",
    state->scrram[0],
    state->scrram[1],
    state->scrram[2],
    state->scrram[3],
    state->scrram[4],
    state->scrram[5],
    state->scrram[6],
    state->scrram[7]);
    */
	return 0;
}

/* Holywood Action */

/* FG */
static TILE_GET_INFO( get_actionhw_fg_tile_info )
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;
	int tileno = state->fgram[tile_index * 2] & 0x0fff;
	int color = state->fgram[tile_index * 2 + 1] & 0x000f;

	SET_TILE_INFO(0, tileno + 0x7000 * 2, color + 0x00, 0);
}

/* BG */
static TILE_GET_INFO( get_actionhw_bg_tile_info )
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;
	int tileno = state->bgram[tile_index * 2] & 0x1fff;
	int color = state->bgram[tile_index * 2 + 1] & 0x000f;
	int flipx = state->bgram[tile_index * 2 + 1] & 0x0020;
	int flipy = state->bgram[tile_index * 2 + 1] & 0x0040;

	SET_TILE_INFO(1, tileno + 0x0000, color + 0x10, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}

/* BG 2 */
static TILE_GET_INFO( get_actionhw_bg2_tile_info )
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;
	int tileno = state->bg2ram[tile_index * 2] & 0x1fff;
	int color = state->bg2ram[tile_index * 2 + 1] & 0x000f;
	int flipx = state->bg2ram[tile_index * 2 + 1] & 0x0020;
	int flipy = state->bg2ram[tile_index * 2 + 1] & 0x0040;

	SET_TILE_INFO(1, tileno + 0x2000, color + 0x20, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}


static TILEMAP_MAPPER( tilemap_scan_actionhwbg2 )
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}

static TILEMAP_MAPPER( tilemap_scan_actionhwbg )
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}

static TILEMAP_MAPPER( tilemap_scan_actionhwfg )
{
	/* logical (col,row) -> memory offset */
	return col * 32 + (row & 0x1f) + ((row & 0x20) >> 5) * 0x800;
}


VIDEO_START( actionhw )
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;

	state->fgtm = tilemap_create(machine, get_actionhw_fg_tile_info, tilemap_scan_actionhwfg, 8, 8, 64, 64);
	state->bgtm = tilemap_create(machine, get_actionhw_bg_tile_info, tilemap_scan_actionhwbg, 16, 16, 64, 64);
	state->bg2tm = tilemap_create(machine, get_actionhw_bg2_tile_info, tilemap_scan_actionhwbg2, 16, 16, 64, 64);

	tilemap_set_transparent_pen(state->fgtm, 15);
	tilemap_set_transparent_pen(state->bgtm, 15);
}


static void actionhw_draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	kickgoal_state *state = (kickgoal_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	const gfx_element *gfx = machine->gfx[1];
	int offs;

	for (offs = 0; offs < state->spriteram_size / 2; offs += 4)
	{
		int xpos = spriteram[offs + 3];
		int ypos = spriteram[offs + 0] & 0x00ff;
		int tileno = spriteram[offs + 2] & 0x3fff;
		int flipx = spriteram[offs + 1] & 0x0020;
		int color = spriteram[offs + 1] & 0x000f;

		if (spriteram[offs + 0] & 0x0100) break;

		ypos = 0x110 - ypos;

		drawgfx_transpen(bitmap,cliprect,gfx,
				tileno+0x4000,
				0x30 + color,
				flipx,0,
				xpos-16+4,ypos-32,15);
	}
}


VIDEO_UPDATE( actionhw )
{
	kickgoal_state *state = (kickgoal_state *)screen->machine->driver_data;
	/* set scroll */
	tilemap_set_scrollx(state->fgtm, 0, state->scrram[0]);
	tilemap_set_scrolly(state->fgtm, 0, state->scrram[1]);
	tilemap_set_scrollx(state->bgtm, 0, state->scrram[2]);
	tilemap_set_scrolly(state->bgtm, 0, state->scrram[3]);
	tilemap_set_scrollx(state->bg2tm, 0, state->scrram[4]);
	tilemap_set_scrolly(state->bg2tm, 0, state->scrram[5]);

	/* draw */
	tilemap_draw(bitmap, cliprect, state->bg2tm, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bgtm, 0, 0);

	actionhw_draw_sprites(screen->machine, bitmap, cliprect);

	tilemap_draw(bitmap, cliprect, state->fgtm, 0, 0);
	return 0;
}
