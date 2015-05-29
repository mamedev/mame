// license:???
// copyright-holders:David Graves, Jarek Burczynski
#include "emu.h"
#include "includes/darius.h"

/***************************************************************************/

TILE_GET_INFO_MEMBER(darius_state::get_fg_tile_info)
{
	UINT16 code = (m_fg_ram[tile_index + 0x2000] & 0x7ff);
	UINT16 attr = m_fg_ram[tile_index];

	SET_TILE_INFO_MEMBER(2,
			code,
			(attr & 0x7f),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

/***************************************************************************/

void darius_state::video_start()
{
	m_gfxdecode->gfx(2)->set_granularity(16);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(darius_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,128,64);

	m_fg_tilemap->set_transparent_pen(0);
}

/***************************************************************************/

WRITE16_MEMBER(darius_state::darius_fg_layer_w)
{
	COMBINE_DATA(&m_fg_ram[offset]);
	if (offset < 0x4000)
		m_fg_tilemap->mark_tile_dirty((offset & 0x1fff));
}

/***************************************************************************/

void darius_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int primask, int x_offs, int y_offs )
{
	UINT16 *spriteram = m_spriteram;
	int offs, curx, cury;
	UINT16 code, data, sx, sy;
	UINT8 flipx, flipy, color, priority;

	for (offs = m_spriteram.bytes() / 2 - 4; offs >= 0; offs -= 4)
	{
		code = spriteram[offs + 2] & 0x1fff;

		if (code)
		{
			data = spriteram[offs];
			sy = (256 - data) & 0x1ff;

			data = spriteram[offs + 1];
			sx = data & 0x3ff;

			data = spriteram[offs + 2];
			flipx = ((data & 0x4000) >> 14);
			flipy = ((data & 0x8000) >> 15);

			data = spriteram[offs + 3];
			priority = (data & 0x80) >> 7;  // 0 = low
			if (priority != primask)
				continue;
			color = (data & 0x7f);

			curx = sx - x_offs;
			cury = sy + y_offs;

			if (curx > 900) curx -= 1024;
			if (cury > 400) cury -= 512;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code, color,
					flipx, flipy,
					curx, cury, 0);
		}
	}
}



UINT32 darius_state::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs)
{
	m_pc080sn->tilemap_update();

	// draw bottom layer(always active)
	m_pc080sn->tilemap_draw_offset(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0, -xoffs, 0);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(bitmap, cliprect, 0, xoffs, -8); // draw sprites with priority 0 which are under the mid layer

	// draw middle layer
	m_pc080sn->tilemap_draw_offset(screen, bitmap, cliprect, 1, 0, 0, -xoffs, 0);

	draw_sprites(bitmap, cliprect, 1, xoffs, -8); // draw sprites with priority 1 which are over the mid layer

	/* top(text) layer is in fixed position */
	m_fg_tilemap->set_scrollx(0, 0 + xoffs);
	m_fg_tilemap->set_scrolly(0, -8);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 darius_state::screen_update_darius_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8 * 0); }
UINT32 darius_state::screen_update_darius_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8 * 1); }
UINT32 darius_state::screen_update_darius_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8 * 2); }
