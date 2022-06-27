// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Barry Rodewald
/*

  Malzak

  Video functions

  SAA 5050 -- Character display
  S2636 (x2) -- Sprites, Sprite->Sprite collisions
  Playfield graphics generator
      (TODO: probably best to switch this to tilemaps one day, figure out banking)

*/


#include "emu.h"
#include "malzak.h"

void malzak_state::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 8 * 8; i++)
	{
		palette.set_pen_color(i * 2 + 0, pal1bit(i >> 3), pal1bit(i >> 4), pal1bit(i >> 5));
		palette.set_pen_color(i * 2 + 1, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}
}

uint8_t malzak_state::videoram_r(offs_t offset)
{
	return m_videoram[offset];
}

void malzak_state::port60_w(uint8_t data)
{
	m_scrollx = data;
//  logerror("I/O: port 0x60 write 0x%02x\n", data);
	m_playfield_tilemap->set_scrollx(0, m_scrollx + 48);
}

void malzak_state::portc0_w(uint8_t data)
{
	m_scrolly = data;
//  logerror("I/O: port 0xc0 write 0x%02x\n", data);
	m_playfield_tilemap->set_scrolly(0, m_scrolly);
}

//TODO: how readback works with this arrangement? Never occurs in-game
void malzak_state::playfield_w(offs_t offset, uint8_t data)
{
	int tile = ((m_scrollx / 16) * 16) + (offset / 16);

//  m_playfield_x[tile] = m_malzak_x / 16;
//  m_playfield_y[tile] = m_malzak_y;
	m_playfield_code[tile] = data;
	m_playfield_tilemap->mark_tile_dirty(tile);
	// POST only, adds to the scrollx base address?
//  if (offset & 0xf)
//      popmessage("GFX: 0x16%02x write 0x%02x\n", offset, data);
}

TILE_GET_INFO_MEMBER(malzak_state::get_tile_info)
{
	u8 code = (m_playfield_code[tile_index] & 0x1f) | (m_playfield_bank << 5);
	u8 color = ((m_playfield_code[tile_index] & 0xe0) >> 5);
	tileinfo.set(0, code, color, 0);
}

void malzak_state::video_start()
{
	m_scrollx = 0;

	int width = m_screen->width();
	int height = m_screen->height();
	m_trom_bitmap = std::make_unique<bitmap_rgb32>(width, height);
	m_playfield_bitmap = std::make_unique<bitmap_rgb32>(width, height);
	m_playfield_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(malzak_state::get_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 16, 16);
}

uint32_t malzak_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	bitmap.fill(rgb_t::black(), cliprect);

	// prepare bitmaps
	m_trom->screen_update(screen, *m_trom_bitmap, cliprect);
	m_playfield_tilemap->draw(screen, *m_playfield_bitmap, cliprect, 0, 0);
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);
	bitmap_ind16 const &s2636_1_bitmap = m_s2636[1]->update(cliprect);

	// Superimpose
	// TODO: update for PAL size
	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		int sy = y / 2;
		for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
		{
			int sx = x / 2;
			int s2636_pix_0 = s2636_0_bitmap.pix(sy, sx);
			int s2636_pix_1 = s2636_1_bitmap.pix(sy, sx);
			rgb_t trom_pix = m_trom_bitmap->pix(y, x);
			rgb_t play_pix = m_playfield_bitmap->pix(sy, sx);

			// SAA5050 > s2636[1] > s2636[0] > playfield
			if (trom_pix != rgb_t::black())
				bitmap.pix(y, x) = trom_pix;
			else if (S2636_IS_PIXEL_DRAWN(s2636_pix_1))
				bitmap.pix(y, x) = palette[S2636_PIXEL_COLOR(s2636_pix_1)];
			else if (S2636_IS_PIXEL_DRAWN(s2636_pix_0))
				bitmap.pix(y, x) = palette[S2636_PIXEL_COLOR(s2636_pix_0)];
			else
				bitmap.pix(y, x) = play_pix;
		}
	}

	return 0;
}
