/***************************************************************************

    Mustache Boy
    (c)1987 March Electronics

***************************************************************************/


#include "emu.h"
#include "includes/mustache.h"


void mustache_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
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

		palette_set_color(machine(),i,MAKE_RGB(r,g,b));
	}
}

WRITE8_MEMBER(mustache_state::mustache_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(mustache_state::mustache_video_control_w)
{
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}

	/* tile bank */

	if ((m_control_byte ^ data) & 0x08)
	{
		m_control_byte = data;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(mustache_state::mustache_scroll_w)
{
	m_bg_tilemap->set_scrollx(0, 0x100 - data);
	m_bg_tilemap->set_scrollx(1, 0x100 - data);
	m_bg_tilemap->set_scrollx(2, 0x100 - data);
	m_bg_tilemap->set_scrollx(3, 0x100);
}

TILE_GET_INFO_MEMBER(mustache_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int attr = videoram[2 * tile_index + 1];
	int code = videoram[2 * tile_index] + ((attr & 0x60) << 3) + ((m_control_byte & 0x08) << 7);
	int color = attr & 0x0f;

	SET_TILE_INFO_MEMBER(0, code, color, ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0)   );


}

void mustache_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(mustache_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS_FLIP_X,
		 8, 8, 64, 32);

	m_bg_tilemap->set_scroll_rows(4);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	mustache_state *state = machine.driver_data<mustache_state>();
	rectangle clip = cliprect;
	gfx_element *gfx = machine.gfx[1];
	const rectangle &visarea = machine.primary_screen->visible_area();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0;offs < state->m_spriteram.bytes();offs += 4)
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
			if (state->flip_screen())
				clip.min_y = visarea.min_y + 56;
			else
				clip.max_y = visarea.max_y - 56;

		if (state->flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap,clip,gfx,
				code,
				color,
				state->flip_screen(),state->flip_screen(),
				sx,sy,0);
	}
}

UINT32 mustache_state::screen_update_mustache(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(machine(), bitmap, cliprect);
	return 0;
}
