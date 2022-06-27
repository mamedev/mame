// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"

#include "crshrace.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(crshrace_state::get_tile_info1)
{
	int code = m_videoram[0][tile_index];

	tileinfo.set(1, (code & 0xfff) + (m_roz_bank << 12), code >> 12, 0);
}

TILE_GET_INFO_MEMBER(crshrace_state::get_tile_info2)
{
	int code = m_videoram[1][tile_index];

	tileinfo.set(0, code, 0, 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


uint32_t crshrace_state::tile_callback(uint32_t code)
{
	return m_spriteram[1]->buffer()[code&0x7fff];
}


void crshrace_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(crshrace_state::get_tile_info1)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(crshrace_state::get_tile_info2)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[0]->set_transparent_pen(0x0f);
	m_tilemap[1]->set_transparent_pen(0xff);

}


/***************************************************************************

  Memory handlers

***************************************************************************/


void crshrace_state::roz_bank_w(offs_t offset, uint8_t data)
{
	if (m_roz_bank != data)
	{
		m_roz_bank = data;
		m_tilemap[0]->mark_all_dirty();
	}
}


void crshrace_state::gfxctrl_w(offs_t offset, uint8_t data)
{
	m_gfxctrl = data;
	m_flipscreen = data & 0x20;
}


/***************************************************************************

  Display refresh

***************************************************************************/

void crshrace_state::draw_bg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
}


void crshrace_state::draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k053936->zoom_draw(screen, bitmap, cliprect, m_tilemap[0], 0, 0, 1);
}


uint32_t crshrace_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_gfxctrl & 0x04)   // display disable?
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	bitmap.fill(0x1ff, cliprect);



	switch (m_gfxctrl & 0xfb)
	{
		case 0x00:  // high score screen
			m_spr->draw_sprites(m_spriteram[0]->buffer(), 0x2000,  screen, bitmap, cliprect);
			draw_bg(screen, bitmap, cliprect);
			draw_fg(screen, bitmap, cliprect);
			break;
		case 0x01:
		case 0x02:
			draw_bg(screen, bitmap, cliprect);
			draw_fg(screen, bitmap, cliprect);
			m_spr->draw_sprites(m_spriteram[0]->buffer(), 0x2000,  screen, bitmap, cliprect);
			break;
		default:
			popmessage("gfxctrl = %02x", m_gfxctrl);
			break;
	}
	return 0;
}
