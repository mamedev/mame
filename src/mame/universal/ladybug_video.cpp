// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  ladybug/sraider tile/sprite hardware

***************************************************************************/

#include "emu.h"
#include "ladybug_video.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(LADYBUG_VIDEO,  ladybug_video_device,  "ladybug_video",  "Lady Bug/Space Raider video")


ladybug_video_device::ladybug_video_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LADYBUG_VIDEO, tag, owner, clock)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}

void ladybug_video_device::device_start()
{
	m_spr_ram = std::make_unique<u8 []>(0x0400);
	m_bg_ram = std::make_unique<u8 []>(0x0800);
	std::fill_n(m_spr_ram.get(), 0x0400, 0);
	std::fill_n(m_bg_ram.get(), 0x0800, 0);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ladybug_video_device::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_rows(32);
	m_bg_tilemap->set_transparent_pen(0);

	save_pointer(NAME(m_spr_ram), 0x0400);
	save_pointer(NAME(m_bg_ram), 0x0800);
}


void ladybug_video_device::bg_w(offs_t offset, u8 data)
{
	m_bg_ram[offset & 0x07ff] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x03ff);
}

TILE_GET_INFO_MEMBER(ladybug_video_device::get_bg_tile_info)
{
	int const code = m_bg_ram[tile_index] + (BIT(m_bg_ram[0x0400 | tile_index], 3) << 8);
	int const color = m_bg_ram[0x0400 | tile_index] & 0x07;

	tileinfo.set(0, code, color, 0);
}

void ladybug_video_device::draw(screen_device &screen, bitmap_ind16 &bitmap, rectangle const &cliprect, bool flip)
{
	// TODO: confirm whether sraider hardware actually does this - not used by the game
	for (unsigned offs = 0; offs < 32; ++offs)
	{
		int const scroll = m_bg_ram[((offs & 0x03) << 5) | (offs >> 2)];
		m_bg_tilemap->set_scrollx(offs, flip ? -scroll : scroll);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0);
	draw_sprites(bitmap, cliprect);
}

void ladybug_video_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x400 - (0x40 << 1); (0x40 << 1) <= offs; offs -= 0x40)
	{
		// find last valid sprite of current block
		int i = 0;
		while ((0x40 > i) && m_spr_ram[offs + i])
			i += 4;

		while (0 < i)
		{
			i -= 4;

			/*
			 abccdddd eeeeeeee fffghhhh iiiiiiii

			 a: enable?
			 b: size (0 = 8x8, 1 = 16x16)
			 c: flip
			 d: fine-y (coarse-y is from offset)
			 e: sprite code (shift right 2 bits for 16x16 sprites)
			 f: unknown
			 g: sprite bank
			 h: color
			 i: x position
			*/

			if (m_spr_ram[offs + i] & 0x80)
			{
				bool const big(m_spr_ram[offs + i] & 0x40);
				bool const xflip(m_spr_ram[offs + i] & 0x20);
				bool const yflip(m_spr_ram[offs + i] & 0x10);
				int const code(m_spr_ram[offs + i + 1] | (BIT(m_spr_ram[offs + i + 2], 4) << 8));
				int const color(m_spr_ram[offs + i + 2] & 0x0f);
				int const xpos(m_spr_ram[offs + i + 3]);
				int const ypos((offs >> 2) | (m_spr_ram[offs + i] & 0x0f));

				if (big) // 16x16
					m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code >> 2, color, xflip, yflip, xpos, ypos - 8, 0);
				else // 8x8
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, color, xflip, yflip, xpos, ypos, 0);
			}
		}
	}
}
