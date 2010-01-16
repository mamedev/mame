#include "emu.h"
#include "includes/taito_l.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg18_tile_info )
{
	taitol_state *state = (taitol_state *)machine->driver_data;
	int attr = state->rambanks[2 * tile_index + 0x8000 + 1];
	int code = state->rambanks[2 * tile_index + 0x8000]
			| ((attr & 0x03) << 8)
			| ((state->bankc[(attr & 0xc) >> 2]) << 10)
			| (state->horshoes_gfxbank << 12);

	SET_TILE_INFO(
			0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

static TILE_GET_INFO( get_bg19_tile_info )
{
	taitol_state *state = (taitol_state *)machine->driver_data;
	int attr = state->rambanks[2 * tile_index + 0x9000 + 1];
	int code = state->rambanks[2 * tile_index + 0x9000]
			| ((attr & 0x03) << 8)
			| ((state->bankc[(attr & 0xc) >> 2]) << 10)
			| (state->horshoes_gfxbank << 12);

	SET_TILE_INFO(
			0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

static TILE_GET_INFO( get_ch1a_tile_info )
{
	taitol_state *state = (taitol_state *)machine->driver_data;
	int attr = state->rambanks[2 * tile_index + 0xa000 + 1];
	int code = state->rambanks[2 * tile_index + 0xa000] | ((attr & 0x01) << 8) | ((attr & 0x04) << 7);

	SET_TILE_INFO(
			2,
			code,
			(attr & 0xf0) >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( taitol )
{
	taitol_state *state = (taitol_state *)machine->driver_data;
	int i;

	state->bg18_tilemap = tilemap_create(machine, get_bg18_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->bg19_tilemap = tilemap_create(machine, get_bg19_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->ch1a_tilemap = tilemap_create(machine, get_ch1a_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->bg18_tilemap, 0);
	tilemap_set_transparent_pen(state->ch1a_tilemap, 0);

	for (i = 0; i < 256; i++)
		palette_set_color(machine, i, MAKE_RGB(0, 0, 0));

	tilemap_set_scrolldx(state->ch1a_tilemap, -8, -8);
	tilemap_set_scrolldx(state->bg18_tilemap, 28, -11);
	tilemap_set_scrolldx(state->bg19_tilemap, 38, -21);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( horshoes_bankg_w )
{
	taitol_state *state = (taitol_state *)space->machine->driver_data;

	if (state->horshoes_gfxbank != data)
	{
		state->horshoes_gfxbank = data;

		tilemap_mark_all_tiles_dirty(state->bg18_tilemap);
		tilemap_mark_all_tiles_dirty(state->bg19_tilemap);
	}
}

WRITE8_HANDLER( taitol_bankc_w )
{
	taitol_state *state = (taitol_state *)space->machine->driver_data;

	if (state->bankc[offset] != data)
	{
		state->bankc[offset] = data;
//      logerror("Bankc %d, %02x (%04x)\n", offset, data, cpu_get_pc(space->cpu));

		tilemap_mark_all_tiles_dirty(state->bg18_tilemap);
		tilemap_mark_all_tiles_dirty(state->bg19_tilemap);
	}
}

READ8_HANDLER( taitol_bankc_r )
{
	taitol_state *state = (taitol_state *)space->machine->driver_data;
	return state->bankc[offset];
}


WRITE8_HANDLER( taitol_control_w )
{
	taitol_state *state = (taitol_state *)space->machine->driver_data;

//  logerror("Control Write %02x (%04x)\n", data, cpu_get_pc(space->cpu));

	state->cur_ctrl = data;
//popmessage("%02x",data);

	/* bit 0 unknown */

	/* bit 1 unknown */

	/* bit 3 controls sprite/tile priority - handled in vh_screenrefresh() */

	/* bit 4 flip screen */
	state->flipscreen = data & 0x10;
	tilemap_set_flip_all(space->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* bit 5 display enable - handled in vh_screenrefresh() */
}

READ8_HANDLER( taitol_control_r )
{
	taitol_state *state = (taitol_state *)space->machine->driver_data;

//  logerror("Control Read %02x (%04x)\n", cur_ctrl, cpu_get_pc(space->cpu));
	return state->cur_ctrl;
}

void taitol_chardef14_m( running_machine *machine, int offset )
{
	gfx_element_mark_dirty(machine->gfx[2], offset / 32 + 0);
}

void taitol_chardef15_m( running_machine *machine, int offset )
{
	gfx_element_mark_dirty(machine->gfx[2], offset / 32 + 128);
}

void taitol_chardef16_m( running_machine *machine, int offset )
{
	gfx_element_mark_dirty(machine->gfx[2], offset / 32 + 256);
}

void taitol_chardef17_m( running_machine *machine, int offset )
{
	gfx_element_mark_dirty(machine->gfx[2], offset / 32 + 384);
}

void taitol_chardef1c_m( running_machine *machine, int offset )
{
	gfx_element_mark_dirty(machine->gfx[2], offset / 32 + 512);
}

void taitol_chardef1d_m( running_machine *machine, int offset )
{
	gfx_element_mark_dirty(machine->gfx[2], offset / 32 + 640);
}

void taitol_chardef1e_m( running_machine *machine, int offset )
{
	gfx_element_mark_dirty(machine->gfx[2], offset / 32 + 768);
}

void taitol_chardef1f_m( running_machine *machine, int offset )
{
	gfx_element_mark_dirty(machine->gfx[2], offset / 32 + 896);
}

void taitol_bg18_m( running_machine *machine, int offset )
{
	taitol_state *state = (taitol_state *)machine->driver_data;
	tilemap_mark_tile_dirty(state->bg18_tilemap, offset / 2);
}

void taitol_bg19_m( running_machine *machine, int offset )
{
	taitol_state *state = (taitol_state *)machine->driver_data;
	tilemap_mark_tile_dirty(state->bg19_tilemap, offset / 2);
}

void taitol_char1a_m( running_machine *machine, int offset )
{
	taitol_state *state = (taitol_state *)machine->driver_data;
	tilemap_mark_tile_dirty(state->ch1a_tilemap, offset / 2);
}

void taitol_obj1b_m( running_machine *machine, int offset )
{
#if 0
	if (offset >= 0x3f0 && offset <= 0x3ff)
	{
		/* scroll, handled in vh-screenrefresh */
	}
#endif
}



/***************************************************************************

  Display refresh

***************************************************************************/

/*
    Sprite format:
    00: xxxxxxxx tile number (low)
    01: xxxxxxxx tile number (high)
    02: ----xxxx color
        ----x--- priority
    03: -------x flip x
        ------x- flip y
    04: xxxxxxxx x position (low)
    05: -------x x position (high)
    06: xxxxxxxx y position
    07: xxxxxxxx unknown / ignored? Seems just garbage in many cases, e.g
                 plgirs2 bullets and raimais big bosses.
*/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	taitol_state *state = (taitol_state *)machine->driver_data;
	int offs;

	/* at spriteram + 0x3f0 and 03f8 are the tilemap control registers; spriteram + 0x3e8 seems to be unused */
	for (offs = 0; offs < TAITOL_SPRITERAM_SIZE - 3 * 8; offs += 8)
	{
		int code, color, sx, sy, flipx, flipy;

		color = state->buff_spriteram[offs + 2] & 0x0f;
		code = state->buff_spriteram[offs] | (state->buff_spriteram[offs + 1] << 8);

		code |= (state->horshoes_gfxbank & 0x03) << 10;

		sx = state->buff_spriteram[offs + 4] | ((state->buff_spriteram[offs + 5] & 1) << 8);
		sy = state->buff_spriteram[offs + 6];
		if (sx >= 320)
			sx -= 512;
		flipx = state->buff_spriteram[offs + 3] & 0x01;
		flipy = state->buff_spriteram[offs + 3] & 0x02;

		if (state->flipscreen)
		{
			sx = 304 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		pdrawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,
				machine->priority_bitmap,
				(color & 0x08) ? 0xaa : 0x00,0);
	}
}


VIDEO_UPDATE( taitol )
{
	taitol_state *state = (taitol_state *)screen->machine->driver_data;
	int dx, dy;

	dx = state->rambanks[0xb3f4] | (state->rambanks[0xb3f5] << 8);
	if (state->flipscreen)
		dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf;
	dy = state->rambanks[0xb3f6];

	tilemap_set_scrollx(state->bg18_tilemap, 0, -dx);
	tilemap_set_scrolly(state->bg18_tilemap, 0, -dy);

	dx = state->rambanks[0xb3fc] | (state->rambanks[0xb3fd] << 8);
	if (state->flipscreen)
		dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf;
	dy = state->rambanks[0xb3fe];

	tilemap_set_scrollx(state->bg19_tilemap, 0, -dx);
	tilemap_set_scrolly(state->bg19_tilemap, 0, -dy);

	if (state->cur_ctrl & 0x20)	/* display enable */
	{
		bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

		tilemap_draw(bitmap, cliprect, state->bg19_tilemap, 0, 0);

		if (state->cur_ctrl & 0x08)	/* sprites always over BG1 */
			tilemap_draw(bitmap, cliprect, state->bg18_tilemap, 0, 0);
		else					/* split priority */
			tilemap_draw(bitmap, cliprect, state->bg18_tilemap,0,1);

		draw_sprites(screen->machine, bitmap, cliprect);

		tilemap_draw(bitmap, cliprect, state->ch1a_tilemap, 0, 0);
	}
	else
		bitmap_fill(bitmap, cliprect, screen->machine->pens[0]);
	return 0;
}



VIDEO_EOF( taitol )
{
	taitol_state *state = (taitol_state *)machine->driver_data;
	UINT8 *spriteram = state->rambanks + 0xb000;

	memcpy(state->buff_spriteram, spriteram, TAITOL_SPRITERAM_SIZE);
}
