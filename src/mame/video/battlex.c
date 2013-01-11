/***************************************************************************

    Video emulation for Omori Battle Cross

***************************************************************************/

#include "emu.h"
#include "includes/battlex.h"


WRITE8_MEMBER(battlex_state::battlex_palette_w)
{
	palette_set_color_rgb(machine(), offset, pal1bit(data >> 0), pal1bit(data >> 2), pal1bit(data >> 1));
}

WRITE8_MEMBER(battlex_state::battlex_scroll_x_lsb_w)
{
	m_scroll_lsb = data;
}

WRITE8_MEMBER(battlex_state::battlex_scroll_x_msb_w)
{
	m_scroll_msb = data;
}

WRITE8_MEMBER(battlex_state::battlex_scroll_starfield_w)
{
}

WRITE8_MEMBER(battlex_state::battlex_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(battlex_state::battlex_flipscreen_w)
{
	m_starfield_enabled = data & 0x10;

	if (flip_screen() != (data >> 7))
	{
		flip_screen_set(data & 0x80);
		machine().tilemap().mark_all_dirty();
	}
}


TILE_GET_INFO_MEMBER(battlex_state::get_bg_tile_info)
{
	int tile = m_videoram[tile_index * 2] | (((m_videoram[tile_index * 2 + 1] & 0x01)) << 8);
	int color = (m_videoram[tile_index * 2 + 1] & 0x0e) >> 1; // high bits unused

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

void battlex_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(battlex_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	battlex_state *state = machine.driver_data<battlex_state>();
	gfx_element *gfx = machine.gfx[1];
	UINT8 *source = state->m_spriteram;
	UINT8 *finish = state->m_spriteram + 0x200;

	while (source < finish)
	{
		int sx = (source[0] & 0x7f) * 2 - (source[0] & 0x80) * 2;
		int sy = source[3];
		int tile = source[2] & 0x7f;
		int color = source[1] & 0x07;   /* bits 3,4,5 also used during explosions */
		int flipy = source[1] & 0x80;
		int flipx = source[1] & 0x40;

		if (state->flip_screen())
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


UINT32 battlex_state::screen_update_battlex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	m_bg_tilemap->set_scrollx(0, m_scroll_lsb | (m_scroll_msb << 8));
	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(machine(), bitmap, cliprect);

	return 0;
}
