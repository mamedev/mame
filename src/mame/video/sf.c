// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "includes/sf.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(sf_state::get_bg_tile_info)
{
	UINT8 *base = memregion("gfx5")->base() + 2 * tile_index;
	int attr = base[0x10000];
	int color = base[0];
	int code = (base[0x10000 + 1] << 8) | base[1];
	SET_TILE_INFO_MEMBER(0,
			code,
			color,
			TILE_FLIPYX(attr & 3));
}

TILE_GET_INFO_MEMBER(sf_state::get_fg_tile_info)
{
	UINT8 *base = memregion("gfx5")->base() + 0x20000 + 2 * tile_index;
	int attr = base[0x10000];
	int color = base[0];
	int code = (base[0x10000 + 1] << 8) | base[1];
	SET_TILE_INFO_MEMBER(1,
			code,
			color,
			TILE_FLIPYX(attr & 3));
}

TILE_GET_INFO_MEMBER(sf_state::get_tx_tile_info)
{
	int code = m_videoram[tile_index];
	SET_TILE_INFO_MEMBER(3,
			code & 0x3ff,
			code>>12,
			TILE_FLIPYX((code & 0xc00)>>10));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void sf_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sf_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 2048, 16);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sf_state::get_fg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 2048, 16);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sf_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(3);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(sf_state::videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(sf_state::bg_scroll_w)
{
	COMBINE_DATA(&m_bgscroll);
	m_bg_tilemap->set_scrollx(0, m_bgscroll);
}

WRITE16_MEMBER(sf_state::fg_scroll_w)
{
	COMBINE_DATA(&m_fgscroll);
	m_fg_tilemap->set_scrollx(0, m_fgscroll);
}

WRITE16_MEMBER(sf_state::gfxctrl_w)
{
	/* b0 = reset, or maybe "set anyway" */
	/* b1 = pulsed when control6.b6==0 until it's 1 */
	/* b2 = active when dip 8 (flip) on */
	/* b3 = active character plane */
	/* b4 = unused */
	/* b5 = active background plane */
	/* b6 = active middle plane */
	/* b7 = active sprites */

	if (ACCESSING_BITS_0_7)
	{
		m_active = data & 0xff;
		flip_screen_set(data & 0x04);
		m_tx_tilemap->enable(data & 0x08);
		m_bg_tilemap->enable(data & 0x20);
		m_fg_tilemap->enable(data & 0x40);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

inline int sf_state::invert( int nb )
{
	static const int delta[4] = {0x00, 0x18, 0x18, 0x00};
	return nb ^ delta[(nb >> 3) & 3];
}

void sf_state::draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	int offs;

	for (offs = 0x1000 - 0x20; offs >= 0; offs -= 0x20)
	{
		int c = m_objectram[offs];
		int attr = m_objectram[offs + 1];
		int sy = m_objectram[offs + 2];
		int sx = m_objectram[offs + 3];
		int color = attr & 0x000f;
		int flipx = attr & 0x0100;
		int flipy = attr & 0x0200;

		if (attr & 0x400)   /* large sprite */
		{
			int c1, c2, c3, c4, t;

			if (flip_screen())
			{
				sx = 480 - sx;
				sy = 224 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			c1 = c;
			c2 = c + 1;
			c3 = c + 16;
			c4 = c + 17;

			if (flipx)
			{
				t = c1; c1 = c2; c2 = t;
				t = c3; c3 = c4; c4 = t;
			}
			if (flipy)
			{
				t = c1; c1 = c3; c3 = t;
				t = c2; c2 = c4; c4 = t;
			}

			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c1),
					color,
					flipx,flipy,
					sx,sy, 15);
			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c2),
					color,
					flipx,flipy,
					sx+16,sy, 15);
			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c3),
					color,
					flipx,flipy,
					sx,sy+16, 15);
			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c4),
					color,
					flipx,flipy,
					sx+16,sy+16, 15);
		}
		else
		{
			if (flip_screen())
			{
				sx = 496 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c),
					color,
					flipx,flipy,
					sx,sy, 15);
		}
	}
}


UINT32 sf_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_active & 0x20)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(0, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (m_active & 0x80)
		draw_sprites(bitmap, cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
