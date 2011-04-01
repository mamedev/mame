/***************************************************************************

    Video emulation for Omori Battle Cross

***************************************************************************/

#include "emu.h"
#include "includes/battlex.h"


WRITE8_HANDLER( battlex_palette_w )
{
	palette_set_color_rgb(space->machine(), offset, pal1bit(data >> 0), pal1bit(data >> 2), pal1bit(data >> 1));
}

WRITE8_HANDLER( battlex_scroll_x_lsb_w )
{
	battlex_state *state = space->machine().driver_data<battlex_state>();
	state->m_scroll_lsb = data;
}

WRITE8_HANDLER( battlex_scroll_x_msb_w )
{
	battlex_state *state = space->machine().driver_data<battlex_state>();
	state->m_scroll_msb = data;
}

WRITE8_HANDLER( battlex_scroll_starfield_w )
{
}

WRITE8_HANDLER( battlex_videoram_w )
{
	battlex_state *state = space->machine().driver_data<battlex_state>();
	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset / 2);
}

WRITE8_HANDLER( battlex_flipscreen_w )
{
	battlex_state *state = space->machine().driver_data<battlex_state>();
	state->m_starfield_enabled = data & 0x10;

	if (flip_screen_get(space->machine()) != (data >> 7))
	{
		flip_screen_set(space->machine(), data & 0x80);
		tilemap_mark_all_tiles_dirty_all(space->machine());
	}
}


static TILE_GET_INFO( get_bg_tile_info )
{
	battlex_state *state = machine.driver_data<battlex_state>();
	int tile = state->m_videoram[tile_index * 2] | (((state->m_videoram[tile_index * 2 + 1] & 0x01)) << 8);
	int color = (state->m_videoram[tile_index * 2 + 1] & 0x0e) >> 1; // high bits unused

	SET_TILE_INFO(0, tile, color, 0);
}

VIDEO_START( battlex )
{
	battlex_state *state = machine.driver_data<battlex_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
}

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	battlex_state *state = machine.driver_data<battlex_state>();
	const gfx_element *gfx = machine.gfx[1];
	UINT8 *source = state->m_spriteram;
	UINT8 *finish = state->m_spriteram + 0x200;

	while (source < finish)
	{
		int sx = (source[0] & 0x7f) * 2 - (source[0] & 0x80) * 2;
		int sy = source[3];
		int tile = source[2] & 0x7f;
		int color = source[1] & 0x07;	/* bits 3,4,5 also used during explosions */
		int flipy = source[1] & 0x80;
		int flipx = source[1] & 0x40;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, gfx, tile, color, flipx, flipy, sx, sy, 0);
		source += 4;
	}

}


SCREEN_UPDATE(battlex)
{
	battlex_state *state = screen->machine().driver_data<battlex_state>();

	tilemap_set_scrollx(state->m_bg_tilemap, 0, state->m_scroll_lsb | (state->m_scroll_msb << 8));
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect);

	return 0;
}
