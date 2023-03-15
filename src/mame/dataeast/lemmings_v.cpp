// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Lemmings video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    There are two sets of sprites, the combination of custom chips 52 & 71.
    There is a background pixel layer implemented with discrete logic
    rather than a custom chip and a foreground VRAM tilemap layer that the
    game mostly uses as a pixel layer (the vram format is arranged as
    sequential pixels, rather than sequential characters).

***************************************************************************/

#include "emu.h"
#include "lemmings.h"

#include <algorithm>

/******************************************************************************/

TILE_GET_INFO_MEMBER(lemmings_state::get_tile_info)
{
	uint16_t tile = m_vram_data[tile_index];

	tileinfo.set(2,
			tile&0x7ff,
			(tile>>12)&0xf,
			0);
}

void lemmings_state::video_start()
{
	m_vram_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lemmings_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);

	m_vram_tilemap->set_transparent_pen(0);
	m_bitmap0.fill(0x100);

	m_vram_buffer = make_unique_clear<uint8_t[]>(2048 * 64); // 64 bytes per VRAM character
	m_gfxdecode->gfx(2)->set_source(m_vram_buffer.get());

	m_sprgen[0]->alloc_sprite_bitmap();
	m_sprgen[1]->alloc_sprite_bitmap();

	m_sprite_triple_buffer[0] = make_unique_clear<uint16_t[]>(0x800/2);
	m_sprite_triple_buffer[1] = make_unique_clear<uint16_t[]>(0x800/2);

	save_item(NAME(m_bitmap0));
	save_pointer(NAME(m_vram_buffer), 2048 * 64);
	save_pointer(NAME(m_sprite_triple_buffer[0]), 0x800/2, 0);
	save_pointer(NAME(m_sprite_triple_buffer[1]), 0x800/2, 1);
}

WRITE_LINE_MEMBER(lemmings_state::screen_vblank_lemmings)
{
	// rising edge
	if (state)
	{
		for (int chip = 0; chip < 2; chip++)
			std::copy_n(&m_spriteram[chip]->buffer()[0], 0x800/2, &m_sprite_triple_buffer[chip][0]);
	}
}

/******************************************************************************/

// RAM based
void lemmings_state::lemmings_pixel_0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const old = m_pixel_data[0][offset];
	COMBINE_DATA(&m_pixel_data[0][offset]);
	int const src = m_pixel_data[0][offset];
	if (old == src)
		return;

	int const sy = (offset << 1) >> 11;
	int const sx = (offset << 1) & 0x7ff;

	if (sx > 2047 || sy > 255)
		return;

	m_bitmap0.pix(sy, sx + 0) = ((src >> 8) & 0xf) | 0x100;
	m_bitmap0.pix(sy, sx + 1) = ((src >> 0) & 0xf) | 0x100;
}

// RAM based tiles for the FG tilemap
void lemmings_state::lemmings_pixel_1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int sx, sy, src, tile;

	COMBINE_DATA(&m_pixel_data[1][offset]);
	src = m_pixel_data[1][offset];

	sy = (offset << 1) >> 9;
	sx = (offset << 1) & 0x1ff;

	/* Copy pixel to buffer for easier decoding later */
	tile = ((sx / 8) * 32) + (sy / 8);
	m_gfxdecode->gfx(2)->mark_dirty(tile);
	m_vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 8) & 0xf;

	sx += 1; /* Update both pixels in the word */
	m_vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 0) & 0xf;
}

void lemmings_state::lemmings_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram_data[offset]);
	m_vram_tilemap->mark_tile_dirty(offset);
}


void lemmings_state::lemmings_copy_bitmap(bitmap_rgb32& bitmap, int* xscroll, int* yscroll, const rectangle& cliprect)
{
	pen_t const *const paldata = m_palette->pens();

	for (int y=cliprect.top(); y<cliprect.bottom();y++)
	{
		uint32_t *const dst = &bitmap.pix(y,0);

		for (int x=cliprect.left(); x<cliprect.right();x++)
		{
			uint16_t const src = m_bitmap0.pix((y-*yscroll)&0xff,(x-*xscroll)&0x7ff);

			if (src!=0x100)
				dst[x] = paldata[src];
		}
	}
}

uint32_t lemmings_state::screen_update_lemmings(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x1 = -m_control_data[0];
	int x0 = -m_control_data[2];
	int y = 0;
	rectangle rect(0, 0, cliprect.top(), cliprect.bottom());

	// sprites are flipped relative to tilemaps
	m_sprgen[0]->set_flip_screen(true);
	m_sprgen[1]->set_flip_screen(true);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_sprite_triple_buffer[1].get(), 0x400);
	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_sprite_triple_buffer[0].get(), 0x400);

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x300, 0xff);

	/* Pixel layer can be windowed in hardware (two player mode) */
	if ((m_control_data[6] & 2) == 0)
	{
		lemmings_copy_bitmap(bitmap, &x1, &y, cliprect);
	}
	else
	{
		rect.setx(0, 159);
		lemmings_copy_bitmap(bitmap, &x0, &y, rect);

		rect.setx(160, 319);
		lemmings_copy_bitmap(bitmap, &x1, &y, rect);
	}

	m_sprgen[1]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x200, 0xff);
	m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x300, 0xff);
	m_vram_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen[1]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x200, 0xff);
	return 0;
}
