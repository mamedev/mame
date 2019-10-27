// license:GPL-2.0+
// copyright-holders:David Graves, Jarek Burczynski
#include "emu.h"
#include "includes/darius.h"
#include "screen.h"

/***************************************************************************/

TILE_GET_INFO_MEMBER(darius_state::get_fg_tile_info)
{
	u16 code = (m_fg_ram[tile_index + 0x2000] & 0x7ff);
	u16 attr = m_fg_ram[tile_index];

	SET_TILE_INFO_MEMBER(2,
			code,
			(attr & 0x7f),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

/***************************************************************************/

void darius_state::video_start()
{
	m_gfxdecode->gfx(2)->set_granularity(16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darius_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);

	m_fg_tilemap->set_transparent_pen(0);
}

/***************************************************************************/

void darius_state::fg_layer_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_ram[offset]);
	if (offset < 0x4000)
		m_fg_tilemap->mark_tile_dirty((offset & 0x1fff));
}

/***************************************************************************/

void darius_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs)
{
	static const u32 primask[2] =
	{
		GFX_PMASK_2, // draw sprites with priority 0 which are under the mid layer
		0  // draw sprites with priority 1 which are over the mid layer
	};

	for (int offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		const u32 code = m_spriteram[offs + 2] & 0x1fff;

		if (code)
		{
			u16 data = m_spriteram[offs];
			const int sy = (256 - data) & 0x1ff;

			data = m_spriteram[offs + 1];
			const int sx = data & 0x3ff;

			data = m_spriteram[offs + 2];
			const bool flipx = ((data & 0x4000) >> 14);
			const bool flipy = ((data & 0x8000) >> 15);

			data = m_spriteram[offs + 3];
			const int priority = (data & 0x80) >> 7;  // 0 = low
			const u32 color = (data & 0x7f);

			int curx = sx - x_offs;
			int cury = sy + y_offs;

			if (curx > 900) curx -= 1024;
			if (cury > 400) cury -= 512;

			m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
					code, color,
					flipx, flipy,
					curx, cury,
					screen.priority(), primask[priority], 0);
		}
	}
}


u32 darius_state::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs)
{
	screen.priority().fill(0, cliprect);
	m_pc080sn->tilemap_update();

	// draw bottom layer(always active)
	m_pc080sn->tilemap_draw_offset(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 1, -xoffs, 0);

	// draw middle layer
	m_pc080sn->tilemap_draw_offset(screen, bitmap, cliprect, 1, 0, 2, -xoffs, 0);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen, bitmap, cliprect, xoffs, -8);

	/* top(text) layer is in fixed position */
	m_fg_tilemap->set_scrollx(0, 0 + xoffs);
	m_fg_tilemap->set_scrolly(0, -8);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

u32 darius_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8 * 0); }
u32 darius_state::screen_update_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8 * 1); }
u32 darius_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8 * 2); }
