/***************************************************************************

    Mustache Boy
    (c)1987 March Electronics

***************************************************************************/


#include "emu.h"
#include "includes/mustache.h"


PALETTE_INIT(mustache)
{
	int i;

	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[i + 256] >> 0) & 0x01;
		bit1 = (color_prom[i + 256] >> 1) & 0x01;
		bit2 = (color_prom[i + 256] >> 2) & 0x01;
		bit3 = (color_prom[i + 256] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[i + 512] >> 0) & 0x01;
		bit1 = (color_prom[i + 512] >> 1) & 0x01;
		bit2 = (color_prom[i + 512] >> 2) & 0x01;
		bit3 = (color_prom[i + 512] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

WRITE8_HANDLER( mustache_videoram_w )
{
	mustache_state *state = space->machine().driver_data<mustache_state>();
	UINT8 *videoram = state->m_videoram;
	videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset / 2);
}

WRITE8_HANDLER (mustache_video_control_w)
{
	mustache_state *state = space->machine().driver_data<mustache_state>();
	if (flip_screen_get(space->machine()) != (data & 0x01))
	{
		flip_screen_set(space->machine(), data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine());
	}

	/* tile bank */

	if ((state->m_control_byte ^ data) & 0x08)
	{
		state->m_control_byte = data;
		tilemap_mark_all_tiles_dirty_all(space->machine());
	}
}

WRITE8_HANDLER( mustache_scroll_w )
{
	mustache_state *state = space->machine().driver_data<mustache_state>();
	tilemap_set_scrollx(state->m_bg_tilemap, 0, 0x100 - data);
	tilemap_set_scrollx(state->m_bg_tilemap, 1, 0x100 - data);
	tilemap_set_scrollx(state->m_bg_tilemap, 2, 0x100 - data);
	tilemap_set_scrollx(state->m_bg_tilemap, 3, 0x100);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mustache_state *state = machine.driver_data<mustache_state>();
	UINT8 *videoram = state->m_videoram;
	int attr = videoram[2 * tile_index + 1];
	int code = videoram[2 * tile_index] + ((attr & 0x60) << 3) + ((state->m_control_byte & 0x08) << 7);
	int color = attr & 0x0f;

	SET_TILE_INFO(0, code, color, ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0)   );


}

VIDEO_START( mustache )
{
	mustache_state *state = machine.driver_data<mustache_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows_flip_x,
		 8, 8, 64, 32);

	tilemap_set_scroll_rows(state->m_bg_tilemap, 4);
}

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	mustache_state *state = machine.driver_data<mustache_state>();
	rectangle clip = *cliprect;
	const gfx_element *gfx = machine.gfx[1];
	const rectangle &visarea = machine.primary_screen->visible_area();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0;offs < state->m_spriteram_size;offs += 4)
	{
		int sy = 240-spriteram[offs];
		int sx = 240-spriteram[offs+3];
		int code = spriteram[offs+2];
		int attr = spriteram[offs+1];
		int color = (attr & 0xe0)>>5;

		if (sy == 240) continue;

		code+=(attr&0x0c)<<6;

		if ((state->m_control_byte & 0xa))
			clip.max_y = visarea.max_y;
		else
			if (flip_screen_get(machine))
				clip.min_y = visarea.min_y + 56;
			else
				clip.max_y = visarea.max_y - 56;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap,&clip,gfx,
				code,
				color,
				flip_screen_get(machine),flip_screen_get(machine),
				sx,sy,0);
	}
}

SCREEN_UPDATE( mustache )
{
	mustache_state *state = screen->machine().driver_data<mustache_state>();
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
