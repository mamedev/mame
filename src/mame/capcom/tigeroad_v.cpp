// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#include "emu.h"
#include "tigeroad.h"


void tigeroad_state::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

void tigeroad_state::videoctrl_w(u8 data)
{
	// bit 1 flips screen
	if (flip_screen() != (data & 0x02))
	{
		flip_screen_set(data & 0x02);
		machine().tilemap().mark_all_dirty();
	}

	// bit 2 selects bg char bank
	u8 bank = (data & 0x04) >> 2;

	if (m_bgcharbank != bank)
	{
		m_bgcharbank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	// bits 4-5 are coin lockouts
	if (m_has_coinlock)
	{
		machine().bookkeeping().coin_lockout_w(0, !(data & 0x10));
		machine().bookkeeping().coin_lockout_w(1, !(data & 0x20));
	}

	// bits 6-7 are coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x40);
	machine().bookkeeping().coin_counter_w(1, data & 0x80);
}

void tigeroad_state::scroll_w(offs_t offset, u16 data, u16 mem_mask)
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
	int data = m_bgmap[tile_index];
	int attr = data >> 8;
	int code = (data & 0xff) + ((attr & 0xc0) << 2) + (m_bgcharbank << 10);
	int color = attr & 0x0f;
	int flags = (attr & 0x20) ? TILE_FLIPX : 0;

	tileinfo.set(1, code, color, flags);
	tileinfo.group = (attr & 0x10) ? 1 : 0;
}

TILE_GET_INFO_MEMBER(tigeroad_state::get_fg_tile_info)
{
	int data = m_videoram[tile_index];
	int attr = data >> 8;
	int code = (data & 0xff) + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5);
	int color = attr & 0x0f;
	int flags = (attr & 0x10) ? TILE_FLIPY : 0;

	tileinfo.set(0, code, color, flags);
}

TILEMAP_MAPPER_MEMBER(tigeroad_state::tigeroad_tilemap_scan)
{
	// logical (col,row) -> memory offset
	return (col % 8) + 8 * ((127 - row) % 8) + 0x40 * (col / 8) + 0x400 * ((127 - row) / 8);
}

void tigeroad_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tigeroad_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(tigeroad_state::tigeroad_tilemap_scan)),
			32, 32, 128, 128);

	m_fg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tigeroad_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_bg_tilemap->set_transmask(0, 0xffff, 0);
	m_bg_tilemap->set_transmask(1, 0x1ff, 0xfe00);

	m_fg_tilemap->set_transparent_pen(3);

	save_item(NAME(m_bgcharbank));
}

u32 tigeroad_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), m_spriteram->bytes(), flip_screen(), true);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	return 0;
}
