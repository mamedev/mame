/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/appoooh.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Palette information of appoooh is not known.

  The palette decoder of Bank Panic was used for this driver.
  Because these hardware is similar.

***************************************************************************/

PALETTE_INIT( appoooh )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		UINT8 pen;
		int bit0, bit1, bit2, r, g, b;

		if (i < 0x100)
			/* charset #1 */
			pen = (color_prom[0x020 + (i - 0x000)] & 0x0f) | 0x00;
		else
			/* charset #2 */
			pen = (color_prom[0x120 + (i - 0x100)] & 0x0f) | 0x10;

		/* red component */
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[pen] >> 6) & 0x01;
		bit2 = (color_prom[pen] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

PALETTE_INIT( robowres )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		UINT8 pen = color_prom[0x020 + i] & 0x0f;

		/* red component */
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[pen] >> 6) & 0x01;
		bit2 = (color_prom[pen] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	appoooh_state *state = machine.driver_data<appoooh_state>();
	int code = state->m_fg_videoram[tile_index] + 256 * ((state->m_fg_colorram[tile_index] >> 5) & 7);

	SET_TILE_INFO(
			0,
			code,
			state->m_fg_colorram[tile_index] & 0x0f,
			(state->m_fg_colorram[tile_index] & 0x10 ) ? TILEMAP_FLIPX : 0
	);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	appoooh_state *state = machine.driver_data<appoooh_state>();
	int code = state->m_bg_videoram[tile_index] + 256 * ((state->m_bg_colorram[tile_index] >> 5) & 7);

	SET_TILE_INFO(
			1,
			code,
			state->m_bg_colorram[tile_index] & 0x0f,
			(state->m_bg_colorram[tile_index] & 0x10 ) ? TILEMAP_FLIPX : 0
	);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( appoooh )
{
	appoooh_state *state = machine.driver_data<appoooh_state>();

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->m_fg_tilemap, 0);
	tilemap_set_scrolldy(state->m_fg_tilemap, 8, 8);
	tilemap_set_scrolldy(state->m_bg_tilemap, 8, 8);

	state->save_item(NAME(state->m_scroll_x));
	state->save_item(NAME(state->m_priority));
}

WRITE8_HANDLER( appoooh_scroll_w )
{
	appoooh_state *state = space->machine().driver_data<appoooh_state>();
	state->m_scroll_x = data;
}


WRITE8_HANDLER( appoooh_fg_videoram_w )
{
	appoooh_state *state = space->machine().driver_data<appoooh_state>();
	state->m_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset);
}

WRITE8_HANDLER( appoooh_fg_colorram_w )
{
	appoooh_state *state = space->machine().driver_data<appoooh_state>();
	state->m_fg_colorram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset);
}

WRITE8_HANDLER( appoooh_bg_videoram_w )
{
	appoooh_state *state = space->machine().driver_data<appoooh_state>();
	state->m_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

WRITE8_HANDLER( appoooh_bg_colorram_w )
{
	appoooh_state *state = space->machine().driver_data<appoooh_state>();
	state->m_bg_colorram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

WRITE8_HANDLER( appoooh_out_w )
{
	appoooh_state *state = space->machine().driver_data<appoooh_state>();
	/* bit 0 controls NMI */
	state->m_nmi_mask = data & 1;

	/* bit 1 flip screen */
	flip_screen_set(space->machine(), data & 0x02);

	/* bits 2-3 unknown */

	/* bits 4-5 are playfield/sprite priority */
	/* TODO: understand how this works, currently the only thing I do is draw */
	/* the front layer behind sprites when priority == 0, and invert the sprite */
	/* order when priority == 1 */
	state->m_priority = (data & 0x30) >> 4;

	/* bit 6 ROM bank select */
	{
		UINT8 *RAM = space->machine().region("maincpu")->base();

		memory_set_bankptr(space->machine(), "bank1",&RAM[data&0x40 ? 0x10000 : 0x0a000]);
	}

	/* bit 7 unknown (used) */
}

static void appoooh_draw_sprites( bitmap_t *dest_bmp, const rectangle *cliprect, const gfx_element *gfx, UINT8 *sprite )
{
	int offs;
	int flipy = flip_screen_get(gfx->machine());

	for (offs = 0x20 - 4; offs >= 0; offs -= 4)
	{
		int sy    = 240 - sprite[offs + 0];
		int code  = (sprite[offs + 1] >> 2) + ((sprite[offs + 2] >> 5) & 0x07) * 0x40;
		int color = sprite[offs + 2] & 0x0f;	/* TODO: bit 4 toggles continuously, what is it? */
		int sx    = sprite[offs + 3];
		int flipx = sprite[offs + 1] & 0x01;

		if(sx >= 248)
			sx -= 256;

		if (flipy)
		{
			sx = 239 - sx;
			sy = 239 - sy;
			flipx = !flipx;
		}
		drawgfx_transpen( dest_bmp, cliprect,
				gfx,
				code,
				color,
				flipx,flipy,
				sx, sy, 0);
	}
}

static void robowres_draw_sprites( bitmap_t *dest_bmp, const rectangle *cliprect, const gfx_element *gfx, UINT8 *sprite )
{
	int offs;
	int flipy = flip_screen_get(gfx->machine());

	for (offs = 0x20 - 4; offs >= 0; offs -= 4)
	{
		int sy    = 240 - sprite[offs + 0];
		int code  = 0x200 + (sprite[offs + 1] >> 2) + ((sprite[offs + 2] >> 5) & 0x07) * 0x40;
		int color = sprite[offs + 2] & 0x0f;	/* TODO: bit 4 toggles continuously, what is it? */
		int sx    = sprite[offs + 3];
		int flipx = sprite[offs + 1] & 0x01;

		if(sx >= 248)
			sx -= 256;

		if (flipy)
		{
			sx = 239 - sx;
			sy = 239 - sy;
			flipx = !flipx;
		}
		drawgfx_transpen( dest_bmp, cliprect,
				gfx,
				code,
				color,
				flipx,flipy,
				sx, sy, 0);
	}
}


SCREEN_UPDATE( appoooh )
{
	appoooh_state *state = screen->machine().driver_data<appoooh_state>();
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);

	if (state->m_priority == 0)	/* fg behind sprites */
		tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);

	/* draw sprites */
	if (state->m_priority == 1)
	{
		/* sprite set #1 */
		appoooh_draw_sprites(bitmap, cliprect, screen->machine().gfx[2], state->m_spriteram);
		/* sprite set #2 */
		appoooh_draw_sprites(bitmap, cliprect, screen->machine().gfx[3], state->m_spriteram_2);
	}
	else
	{
		/* sprite set #2 */
		appoooh_draw_sprites(bitmap, cliprect, screen->machine().gfx[3], state->m_spriteram_2);
		/* sprite set #1 */
		appoooh_draw_sprites(bitmap, cliprect, screen->machine().gfx[2], state->m_spriteram);
	}

	if (state->m_priority != 0)	/* fg in front of sprites */
		tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);

	return 0;
}

SCREEN_UPDATE( robowres )
{
	appoooh_state *state = screen->machine().driver_data<appoooh_state>();
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);

	if (state->m_priority == 0)	/* fg behind sprites */
		tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);

	/* draw sprites */
	if (state->m_priority == 1)
	{
		/* sprite set #1 */
		robowres_draw_sprites(bitmap, cliprect, screen->machine().gfx[2], state->m_spriteram);
		/* sprite set #2 */
		robowres_draw_sprites(bitmap, cliprect, screen->machine().gfx[3], state->m_spriteram_2);
	}
	else
	{
		/* sprite set #2 */
		robowres_draw_sprites(bitmap, cliprect, screen->machine().gfx[3], state->m_spriteram_2);
		/* sprite set #1 */
		robowres_draw_sprites(bitmap, cliprect, screen->machine().gfx[2], state->m_spriteram);
	}

	if (state->m_priority != 0)	/* fg in front of sprites */
		tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);

	return 0;
}
