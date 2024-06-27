// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, Uki
/******************************************************************************

    Video Hardware for Video System Mahjong series.

    Driver by Takahiro Nogi 2001/02/04 -

******************************************************************************/

#include "emu.h"
#include "fromanc2.h"

/******************************************************************************

  Callbacks for the TileMap code

******************************************************************************/

template<int VRAM, int Layer>
TILE_GET_INFO_MEMBER(fromanc2_state::fromanc2_get_tile_info)
{
	int const tile  = (m_videoram[VRAM][Layer][tile_index] & 0x3fff) | (m_gfxbank[VRAM][Layer] << 14);
	int const color = (m_videoram[VRAM][Layer][tile_index] & 0xc000) >> 14;

	tileinfo.set(Layer, tile, color, 0);
}

template<int VRAM, int Layer>
TILE_GET_INFO_MEMBER(fromanc2_base_state::fromancr_get_tile_info)
{
	int const tile = m_videoram[VRAM][Layer][tile_index] | (m_gfxbank[VRAM][Layer] << 16);

	tileinfo.set(Layer, tile, 0, 0);
}


/******************************************************************************

  Memory handlers

******************************************************************************/

inline void fromanc2_state::fromanc2_dispvram_w( offs_t offset, uint16_t data, uint16_t mem_mask, int vram, int layer )
{
	layer += (offset < 0x1000) ? 0 : 1;

	COMBINE_DATA(&m_videoram[vram][layer][offset & 0x0fff]);
	m_tilemap[vram][layer]->mark_tile_dirty(offset & 0x0fff);
}

void fromanc2_state::fromanc2_videoram_0_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromanc2_dispvram_w(offset, data, mem_mask, 0, 0); }
void fromanc2_state::fromanc2_videoram_1_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromanc2_dispvram_w(offset, data, mem_mask, 0, 2); }
void fromanc2_state::fromanc2_videoram_2_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromanc2_dispvram_w(offset, data, mem_mask, 1, 0); }
void fromanc2_state::fromanc2_videoram_3_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromanc2_dispvram_w(offset, data, mem_mask, 1, 2); }

void fromanc2_state::fromanc2_gfxreg_0_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][0] = -(data - 0x000); break;
		case 0x01:  m_scrolly[0][0] = -(data - 0x000); break;
		case 0x02:  m_scrollx[0][1] = -(data - 0x004); break;
		case 0x03:  m_scrolly[0][1] = -(data - 0x000); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

void fromanc2_state::fromanc2_gfxreg_1_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[1][0] = -(data - 0x1be); break;
		case 0x01:  m_scrolly[1][0] = -(data - 0x1ef); break;
		case 0x02:  m_scrollx[1][1] = -(data - 0x1c2); break;
		case 0x03:  m_scrolly[1][1] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

void fromanc2_state::fromanc2_gfxreg_2_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][2] = -(data - 0x1c0); break;
		case 0x01:  m_scrolly[0][2] = -(data - 0x1ef); break;
		case 0x02:  m_scrollx[0][3] = -(data - 0x1c3); break;
		case 0x03:  m_scrolly[0][3] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

void fromanc2_state::fromanc2_gfxreg_3_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[1][2] = -(data - 0x1bf); break;
		case 0x01:  m_scrolly[1][2] = -(data - 0x1ef); break;
		case 0x02:  m_scrollx[1][3] = -(data - 0x1c3); break;
		case 0x03:  m_scrolly[1][3] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

void fromanc2_state::fromanc2_gfxbank_0_w(uint16_t data)
{
	m_gfxbank[0][0] = (data & 0x000f) >>  0;
	m_gfxbank[0][1] = (data & 0x00f0) >>  4;
	m_gfxbank[0][2] = (data & 0x0f00) >>  8;
	m_gfxbank[0][3] = (data & 0xf000) >> 12;
	m_tilemap[0][0]->mark_all_dirty();
	m_tilemap[0][1]->mark_all_dirty();
	m_tilemap[0][2]->mark_all_dirty();
	m_tilemap[0][3]->mark_all_dirty();
}

void fromanc2_state::fromanc2_gfxbank_1_w(uint16_t data)
{
	m_gfxbank[1][0] = (data & 0x000f) >>  0;
	m_gfxbank[1][1] = (data & 0x00f0) >>  4;
	m_gfxbank[1][2] = (data & 0x0f00) >>  8;
	m_gfxbank[1][3] = (data & 0xf000) >> 12;
	m_tilemap[1][0]->mark_all_dirty();
	m_tilemap[1][1]->mark_all_dirty();
	m_tilemap[1][2]->mark_all_dirty();
	m_tilemap[1][3]->mark_all_dirty();
}


inline void fromanc2_state::fromancr_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask, int layer )
{
	int const vram = (offset < 0x1000) ? 0 : 1;

	COMBINE_DATA(&m_videoram[vram][layer][offset & 0x0fff]);
	m_tilemap[vram][layer]->mark_tile_dirty(offset & 0x0fff);
}

void fromanc2_state::fromancr_videoram_0_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromancr_vram_w(offset, data, mem_mask, 1); }
void fromanc2_state::fromancr_videoram_1_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromancr_vram_w(offset, data, mem_mask, 0); }
void fromanc2_state::fromancr_videoram_2_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromancr_vram_w(offset, data, mem_mask, 2); }

void fromanc2_state::fromancr_gfxreg_0_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][0] = -(data - 0x1bf); break;
		case 0x01:  m_scrolly[0][0] = -(data - 0x1ef); break;
		case 0x02:  m_scrollx[1][0] = -(data - 0x1c3); break;
		case 0x03:  m_scrolly[1][0] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

void fromanc2_state::fromancr_gfxreg_1_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][1] = -(data - 0x000); break;
		case 0x01:  m_scrolly[0][1] = -(data - 0x000); break;
		case 0x02:  m_scrollx[1][1] = -(data - 0x004); break;
		case 0x03:  m_scrolly[1][1] = -(data - 0x000); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

void fromanc2_state::fromancr_gfxbank_w( int data )
{
	m_gfxbank[0][0] = (data & 0x0010) >>  4; // BG (1P)
	m_gfxbank[0][1] = (data & 0xf000) >> 12; // FG (1P)
	m_gfxbank[1][0] = (data & 0x0008) >>  3; // BG (2P)
	m_gfxbank[1][1] = (data & 0x0f00) >>  8; // FG (2P)
	m_tilemap[0][0]->mark_all_dirty();
	m_tilemap[0][1]->mark_all_dirty();
	m_tilemap[1][0]->mark_all_dirty();
	m_tilemap[1][1]->mark_all_dirty();
}


inline void fromanc4_state::fromanc4_vram_w( offs_t offset, uint16_t data, uint16_t mem_mask, int layer )
{
	int const vram = (offset < 0x4000) ? 0 : 1;

	COMBINE_DATA(&m_videoram[vram][layer][offset & 0x3fff]);
	m_tilemap[vram][layer]->mark_tile_dirty(offset & 0x3fff);
}

void fromanc4_state::fromanc4_videoram_0_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromanc4_vram_w(offset, data, mem_mask, 2); }
void fromanc4_state::fromanc4_videoram_1_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromanc4_vram_w(offset, data, mem_mask, 1); }
void fromanc4_state::fromanc4_videoram_2_w(offs_t offset, uint16_t data, uint16_t mem_mask){ fromanc4_vram_w(offset, data, mem_mask, 0); }

void fromanc4_state::fromanc4_gfxreg_0_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][2] = -(data - 0xfbb); break;
		case 0x01:  m_scrolly[0][2] = -(data - 0x1e4); break;
		case 0x02:  m_scrollx[1][2] = -(data - 0xfbb); break;
		case 0x03:  m_scrolly[1][2] = -(data - 0x1e4); break;
		case 0x05:
			m_gfxbank[0][2] = (data & 0x000f) >> 0;
			m_gfxbank[1][2] = (data & 0x0f00) >> 8;
			m_tilemap[0][2]->mark_all_dirty();
			m_tilemap[1][2]->mark_all_dirty();
			break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:    break;
	}
}

void fromanc4_state::fromanc4_gfxreg_1_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][1] = -(data - 0xfba); break;
		case 0x01:  m_scrolly[0][1] = -(data - 0x1e4); break;
		case 0x02:  m_scrollx[1][1] = -(data - 0xfba); break;
		case 0x03:  m_scrolly[1][1] = -(data - 0x1e4); break;
		case 0x05:
			m_gfxbank[0][1] = (data & 0x000f) >> 0;
			m_gfxbank[1][1] = (data & 0x0f00) >> 8;
			m_tilemap[0][1]->mark_all_dirty();
			m_tilemap[1][1]->mark_all_dirty();
			break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:    break;
	}
}

void fromanc4_state::fromanc4_gfxreg_2_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][0] = -(data - 0xfbb); break;
		case 0x01:  m_scrolly[0][0] = -(data - 0x1e4); break;
		case 0x02:  m_scrollx[1][0] = -(data - 0xfbb); break;
		case 0x03:  m_scrolly[1][0] = -(data - 0x1e4); break;
		case 0x05:
			m_gfxbank[0][0] = (data & 0x000f) >> 0;
			m_gfxbank[1][0] = (data & 0x0f00) >> 8;
			m_tilemap[0][0]->mark_all_dirty();
			m_tilemap[1][0]->mark_all_dirty();
			break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:    break;
	}
}


/******************************************************************************

  Start the video hardware emulation.

******************************************************************************/

VIDEO_START_MEMBER(fromanc2_state,fromanc2)
{
	m_tilemap[0][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromanc2_get_tile_info<0, 0>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromanc2_get_tile_info<0, 1>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromanc2_get_tile_info<0, 2>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromanc2_get_tile_info<0, 3>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromanc2_get_tile_info<1, 0>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromanc2_get_tile_info<1, 1>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromanc2_get_tile_info<1, 2>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromanc2_get_tile_info<1, 3>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	for (int screen = 0; screen < 2; screen++)
	{
		palette_device &palette = (screen == 0 ? *m_lpalette : *m_rpalette);
		for (int tmap = 0; tmap < 4; tmap++)
		{
			m_videoram[screen][tmap] = std::make_unique<uint16_t[]>((64 * 64));
			m_tilemap[screen][tmap]->set_palette(palette);
			if (tmap != 0) m_tilemap[screen][tmap]->set_transparent_pen(0x000);
		}
	}

	save_pointer(NAME(m_videoram[0][0]), (64 * 64));
	save_pointer(NAME(m_videoram[0][1]), (64 * 64));
	save_pointer(NAME(m_videoram[0][2]), (64 * 64));
	save_pointer(NAME(m_videoram[0][3]), (64 * 64));
	save_pointer(NAME(m_videoram[1][0]), (64 * 64));
	save_pointer(NAME(m_videoram[1][1]), (64 * 64));
	save_pointer(NAME(m_videoram[1][2]), (64 * 64));
	save_pointer(NAME(m_videoram[1][3]), (64 * 64));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_gfxbank));
}

VIDEO_START_MEMBER(fromanc2_state,fromancr)
{
	m_tilemap[0][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromancr_get_tile_info<0, 0>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromancr_get_tile_info<0, 1>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromancr_get_tile_info<0, 2>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][3] = nullptr;
	m_tilemap[1][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromancr_get_tile_info<1, 0>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromancr_get_tile_info<1, 1>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc2_state::fromancr_get_tile_info<1, 2>))), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][3] = nullptr;

	for (int screen = 0; screen < 2; screen++)
	{
		palette_device &palette = (screen == 0 ? *m_lpalette : *m_rpalette);
		for (int tmap = 0; tmap < 3; tmap++)
		{
			m_videoram[screen][tmap] = std::make_unique<uint16_t[]>((64 * 64));
			m_tilemap[screen][tmap]->set_palette(palette);
			if (tmap != 0) m_tilemap[screen][tmap]->set_transparent_pen(0x0ff);
		}
	}

	save_pointer(NAME(m_videoram[0][0]), (64 * 64));
	save_pointer(NAME(m_videoram[0][1]), (64 * 64));
	save_pointer(NAME(m_videoram[0][2]), (64 * 64));
	save_pointer(NAME(m_videoram[1][0]), (64 * 64));
	save_pointer(NAME(m_videoram[1][1]), (64 * 64));
	save_pointer(NAME(m_videoram[1][2]), (64 * 64));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_gfxbank));

	for (int i = 0; i < 2; i++)
	{
		std::fill(std::begin(m_scrollx[i]), std::end(m_scrollx[i]), 0 );
		std::fill(std::begin(m_scrolly[i]), std::end(m_scrolly[i]), 0 );
	}
}

void fromanc4_state::video_start()
{
	m_tilemap[0][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc4_state::fromancr_get_tile_info<0, 0>))), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[0][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc4_state::fromancr_get_tile_info<0, 1>))), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[0][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc4_state::fromancr_get_tile_info<0, 2>))), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[0][3] = nullptr;
	m_tilemap[1][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc4_state::fromancr_get_tile_info<1, 0>))), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[1][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc4_state::fromancr_get_tile_info<1, 1>))), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[1][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, NAME((&fromanc4_state::fromancr_get_tile_info<1, 2>))), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[1][3] = nullptr;

	for (int screen = 0; screen < 2; screen++)
	{
		palette_device &palette = (screen == 0 ? *m_lpalette : *m_rpalette);
		for (int tmap = 0; tmap < 3; tmap++)
		{
			m_videoram[screen][tmap] = std::make_unique<uint16_t[]>((256 * 64));
			m_tilemap[screen][tmap]->set_palette(palette);
			if (tmap != 0) m_tilemap[screen][tmap]->set_transparent_pen(0x000);
		}
	}

	save_pointer(NAME(m_videoram[0][0]), (256 * 64));
	save_pointer(NAME(m_videoram[0][1]), (256 * 64));
	save_pointer(NAME(m_videoram[0][2]), (256 * 64));
	save_pointer(NAME(m_videoram[1][0]), (256 * 64));
	save_pointer(NAME(m_videoram[1][1]), (256 * 64));
	save_pointer(NAME(m_videoram[1][2]), (256 * 64));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_gfxbank));
}

/******************************************************************************

  Display refresh

******************************************************************************/

uint32_t fromanc2_base_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 4; i++)
	{
		if (m_tilemap[0][i])
		{
			m_tilemap[0][i]->set_scrollx(0, -m_scrollx[0][i]);
			m_tilemap[0][i]->set_scrolly(0, -m_scrolly[0][i]);
			m_tilemap[0][i]->draw(screen, bitmap, cliprect, 0, 0);
		}
	}

	return 0;
}

uint32_t fromanc2_base_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 4; i++)
	{
		if (m_tilemap[1][i])
		{
			m_tilemap[1][i]->set_scrollx(0, -m_scrollx[1][i]);
			m_tilemap[1][i]->set_scrolly(0, -m_scrolly[1][i]);
			m_tilemap[1][i]->draw(screen, bitmap, cliprect, 0, 0);
		}
	}

	return 0;
}
