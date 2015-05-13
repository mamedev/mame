// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/***************************************************************************

    Mustache Boy
    (c)1987 March Electronics

***************************************************************************/


#include "emu.h"
#include "includes/mustache.h"


PALETTE_INIT_MEMBER(mustache_state, mustache)
{
	const UINT8 *color_prom = memregion("proms")->base();
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

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}

WRITE8_MEMBER(mustache_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(mustache_state::video_control_w)
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

WRITE8_MEMBER(mustache_state::scroll_w)
{
	m_bg_tilemap->set_scrollx(0, 0x100 - data);
	m_bg_tilemap->set_scrollx(1, 0x100 - data);
	m_bg_tilemap->set_scrollx(2, 0x100 - data);
	m_bg_tilemap->set_scrollx(3, 0x100);
}

TILE_GET_INFO_MEMBER(mustache_state::get_bg_tile_info)
{
	int attr = m_videoram[2 * tile_index + 1];
	int code = m_videoram[2 * tile_index] + ((attr & 0x60) << 3) + ((m_control_byte & 0x08) << 7);
	int color = attr & 0x0f;

	SET_TILE_INFO_MEMBER(0, code, color, ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0)   );


}

void mustache_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mustache_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS_FLIP_X,
			8, 8, 64, 32);

	m_bg_tilemap->set_scroll_rows(4);

	save_item(NAME(m_control_byte));
}

void mustache_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	rectangle clip = cliprect;
	gfx_element *gfx = m_gfxdecode->gfx(1);
	const rectangle &visarea = m_screen->visible_area();
	int offs;

	for (offs = 0;offs < m_spriteram.bytes();offs += 4)
	{
		int sy = 240-m_spriteram[offs];
		int sx = 240-m_spriteram[offs+3];
		int code = m_spriteram[offs+2];
		int attr = m_spriteram[offs+1];
		int color = (attr & 0xe0)>>5;

		if (sy == 240) continue;

		code+=(attr&0x0c)<<6;

		if ((m_control_byte & 0xa))
			clip.max_y = visarea.max_y;
		else
			if (flip_screen())
				clip.min_y = visarea.min_y + 56;
			else
				clip.max_y = visarea.max_y - 56;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		gfx->transpen(bitmap,clip,
				code,
				color,
				flip_screen(),flip_screen(),
				sx,sy,0);
	}
}

UINT32 mustache_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
