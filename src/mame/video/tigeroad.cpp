// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#include "emu.h"
#include "includes/tigeroad.h"


WRITE16_MEMBER(tigeroad_state::tigeroad_videoram_w)
{
	UINT16 *videoram = m_videoram;
	COMBINE_DATA(&videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tigeroad_state::tigeroad_videoctrl_w)
{
	int bank;

	if (ACCESSING_BITS_8_15)
	{
		data = (data >> 8) & 0xff;

		/* bit 1 flips screen */

		if (flip_screen() != (data & 0x02))
		{
			flip_screen_set(data & 0x02);
			machine().tilemap().mark_all_dirty();
		}

		/* bit 2 selects bg char bank */

		bank = (data & 0x04) >> 2;

		if (m_bgcharbank != bank)
		{
			m_bgcharbank = bank;
			m_bg_tilemap->mark_all_dirty();
		}

		/* bits 4-5 are coin lockouts */
		if (m_has_coinlock)
		{
			machine().bookkeeping().coin_lockout_w(0, !(data & 0x10));
			machine().bookkeeping().coin_lockout_w(1, !(data & 0x20));
		}

		/* bits 6-7 are coin counters */

		machine().bookkeeping().coin_counter_w(0, data & 0x40);
		machine().bookkeeping().coin_counter_w(1, data & 0x80);
	}
}

WRITE16_MEMBER(tigeroad_state::tigeroad_scroll_w)
{
	int scroll = 0;

	COMBINE_DATA(&scroll);

	switch (offset)
	{
	case 0:
		m_bg_tilemap->set_scrollx(0, scroll);
		break;
	case 1:
		m_bg_tilemap->set_scrolly(0, -scroll - 32 * 8);
		break;
	}
}



TILE_GET_INFO_MEMBER(tigeroad_state::get_bg_tile_info)
{
	UINT8 *tilerom = memregion("bgmap")->base();

	int data = tilerom[tile_index];
	int attr = tilerom[tile_index + 1];
	int code = data + ((attr & 0xc0) << 2) + (m_bgcharbank << 10);
	int color = attr & 0x0f;
	int flags = (attr & 0x20) ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(1, code, color, flags);
	tileinfo.group = (attr & 0x10) ? 1 : 0;
}

TILE_GET_INFO_MEMBER(tigeroad_state::get_fg_tile_info)
{
	UINT16 *videoram = m_videoram;
	int data = videoram[tile_index];
	int attr = data >> 8;
	int code = (data & 0xff) + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5);
	int color = attr & 0x0f;
	int flags = (attr & 0x10) ? TILE_FLIPY : 0;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

TILEMAP_MAPPER_MEMBER(tigeroad_state::tigeroad_tilemap_scan)
{
	/* logical (col,row) -> memory offset */
	return 2 * (col % 8) + 16 * ((127 - row) % 8) + 128 * (col / 8) + 2048 * ((127 - row) / 8);
}

void tigeroad_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tigeroad_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(tigeroad_state::tigeroad_tilemap_scan),this),
			32, 32, 128, 128);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tigeroad_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_bg_tilemap->set_transmask(0, 0xffff, 0);
	m_bg_tilemap->set_transmask(1, 0x1ff, 0xfe00);

	m_fg_tilemap->set_transparent_pen(3);
}

UINT32 tigeroad_state::screen_update_tigeroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_spritegen->draw_sprites(bitmap, cliprect, m_gfxdecode, 2, m_spriteram->buffer(), m_spriteram->bytes(), flip_screen(), 1 );
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	return 0;
}
