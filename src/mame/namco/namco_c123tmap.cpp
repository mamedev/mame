// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino

/*
    C123 Tilemaps
    used by
    namcos1.cpp (all games)
    namcos2.cpp (all games)
    namcofl.cpp (all games)
    namconb1.cpp (all games)
*/

#include "emu.h"
#include "namco_c123tmap.h"

GFXDECODE_START( namco_c123tmap_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, gfx_8x8x8_raw, 0, 32 )
GFXDECODE_END

DEFINE_DEVICE_TYPE(NAMCO_C123TMAP, namco_c123tmap_device, "namco_c123tmap", "Namco C123 (4x + 2x Tilemaps)")

namco_c123tmap_device::namco_c123tmap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCO_C123TMAP, tag, owner, clock),
	device_gfx_interface(mconfig, *this, gfxinfo),
	m_color_base(0),
	m_xoffs(0),
	m_yoffs(0),
	m_tmap3_half_height(false),
	m_mask(*this, "mask")
{
	std::fill(std::begin(m_tilemapinfo.control), std::end(m_tilemapinfo.control), 0);
}

void namco_c123tmap_device::device_start()
{
	const int size = m_tmap3_half_height ? 0x8000 : 0x10000;
	m_tilemapinfo.videoram = std::make_unique<uint16_t[]>(size);

	// four scrolling tilemaps
	m_tilemapinfo.tmap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_c123tmap_device::get_tile_info<0x0000>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemapinfo.tmap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_c123tmap_device::get_tile_info<0x1000>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemapinfo.tmap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_c123tmap_device::get_tile_info<0x2000>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	if (m_tmap3_half_height)
	{
		m_tilemapinfo.tmap[3] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_c123tmap_device::get_tile_info<0x3000>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

		// two non-scrolling tilemaps
		m_tilemapinfo.tmap[4] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_c123tmap_device::get_tile_info<0x3808>)), TILEMAP_SCAN_ROWS, 8, 8, 36, 28);
		m_tilemapinfo.tmap[5] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_c123tmap_device::get_tile_info<0x3c08>)), TILEMAP_SCAN_ROWS, 8, 8, 36, 28);
	}
	else
	{
		m_tilemapinfo.tmap[3] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_c123tmap_device::get_tile_info<0x3000>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

		// two non-scrolling tilemaps
		m_tilemapinfo.tmap[4] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_c123tmap_device::get_tile_info<0x4008>)), TILEMAP_SCAN_ROWS, 8, 8, 36, 28);
		m_tilemapinfo.tmap[5] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_c123tmap_device::get_tile_info<0x4408>)), TILEMAP_SCAN_ROWS, 8, 8, 36, 28);
	}

	// define offsets for scrolling
	for (int i = 0; i < 6; i++)
	{
		if (i < 4)
		{
			static const int adj[4] = { 4,2,1,0 };
			const int dx = 44 + adj[i];
			m_tilemapinfo.tmap[i]->set_scrolldx(m_xoffs - dx, m_xoffs + 288 + dx);
			m_tilemapinfo.tmap[i]->set_scrolldy(m_yoffs - 24, m_yoffs + 224 + 24);
		}
		else
		{
			m_tilemapinfo.tmap[i]->set_scrolldx(m_xoffs, m_xoffs);
			m_tilemapinfo.tmap[i]->set_scrolldy(m_yoffs, m_yoffs);
		}
		m_tilemapinfo.tmap[i]->set_palette_offset(m_color_base);
	}

	save_item(NAME(m_tilemapinfo.control));
	save_pointer(NAME(m_tilemapinfo.videoram), size);
}

/**************************************************************************************/

void namco_c123tmap_device::mark_all_dirty(void)
{
	for (int i = 0; i < 6; i++)
	{
		m_tilemapinfo.tmap[i]->mark_all_dirty();
	}
}

template<int Offset>
TILE_GET_INFO_MEMBER(namco_c123tmap_device::get_tile_info)
{
	const uint16_t *vram = &m_tilemapinfo.videoram[Offset];
	int tile, mask;
	m_tilemapinfo.cb(vram[tile_index], tile, mask);
	tileinfo.mask_data = m_mask + mask * 8;
	tileinfo.set(0, tile, 0, 0);
}

void namco_c123tmap_device::init_scroll(int flip) // 8 bit control with external flip screen value
{
	for (int i = 0; i < 4; i++)
	{
		const int offs = i << 2;
		int scrollx = ((m_tilemapinfo.control[offs | 0] & 0xff) << 8) | (m_tilemapinfo.control[offs | 1] & 0xff);
		int scrolly = ((m_tilemapinfo.control[offs | 2] & 0xff) << 8) | (m_tilemapinfo.control[offs | 3] & 0xff);
		if (flip)
		{
			scrollx = -scrollx;
			scrolly = -scrolly;
		}
		m_tilemapinfo.tmap[i]->set_scrollx(0, scrollx);
		m_tilemapinfo.tmap[i]->set_scrolly(0, scrolly);
	}
}

void namco_c123tmap_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int prival, int primask)
{
	for (int i = 0; i < 6; i++)
	{
		// bit 3: disable layer
		if ((m_tilemapinfo.control[0x20 / 2 + i] & 0x7) == pri)
		{
			m_tilemapinfo.tmap[i]->draw(screen, bitmap, cliprect, 0, prival, primask);
		}
	}
}

void namco_c123tmap_device::set_tilemap_videoram(int offset, uint16_t newword)
{
	const int size = (m_tmap3_half_height ? 0x7000 : 0x8000) / 2;
	m_tilemapinfo.videoram[offset] = newword;
	if (offset < size)
	{
		m_tilemapinfo.tmap[offset >> 12]->mark_tile_dirty(offset & 0xfff);
	}
	else if (((offset & 0x7ff / 2) >= 0x10 / 2) && ((offset & 0x7ff / 2) < 0x7f0 / 2))
	{
		const int tile = (offset & 0x7ff / 2) - (0x10 / 2);
		m_tilemapinfo.tmap[((offset >> 10) & 1) + 4]->mark_tile_dirty(tile);
	}
}

// 16 bit handlers
void namco_c123tmap_device::videoram16_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t newword = m_tilemapinfo.videoram[offset];
	COMBINE_DATA(&newword);
	set_tilemap_videoram(offset, newword);
}

uint16_t namco_c123tmap_device::videoram16_r(offs_t offset)
{
	return m_tilemapinfo.videoram[offset];
}

void namco_c123tmap_device::control16_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const uint16_t old = m_tilemapinfo.control[offset];
	data = COMBINE_DATA(&m_tilemapinfo.control[offset]);
	if (old == data)
		return;

	if ((offset >= 0x20 / 2) && (offset <= 0x2a / 2))
	{
		m_tilemapinfo.tmap[offset & 7]->enable(BIT(~data, 3));
	}
	else if ((offset >= 0x30 / 2) && (offset <= 0x3a / 2))
	{
		m_tilemapinfo.tmap[offset & 7]->set_palette_offset(((data & 7) << 8) + m_color_base);
	}
	else if ((offset < 0x20 / 2) && BIT(offset, 0))
	{
		if (offset == 0x02 / 2)
		{
			// all planes are flipped X+Y from D15 of this word
			const int attrs = BIT(data, 15) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
			for (int i = 0; i <= 5; i++)
			{
				m_tilemapinfo.tmap[i]->set_flip(attrs);
			}
		}
		data &= 0x1ff;
		if (BIT(m_tilemapinfo.control[0x02 / 2], 15))
		{
			data = -data;
		}
		if (BIT(offset, 1))
			m_tilemapinfo.tmap[offset >> 2]->set_scrolly(0, data);
		else
			m_tilemapinfo.tmap[offset >> 2]->set_scrollx(0, data);

	}
}

uint16_t namco_c123tmap_device::control16_r(offs_t offset)
{
	return m_tilemapinfo.control[offset];
}

// 8 bit handlers
void namco_c123tmap_device::videoram8_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	uint16_t newword = m_tilemapinfo.videoram[offset >> 1];
	if (BIT(offset, 0))
		newword = (newword & ~mem_mask) | (data & mem_mask);
	else
		newword = (newword & ~(mem_mask << 8)) | ((data & mem_mask) << 8);

	set_tilemap_videoram(offset >> 1, newword);
}

uint8_t namco_c123tmap_device::videoram8_r(offs_t offset)
{
	return m_tilemapinfo.videoram[offset >> 1] >> (BIT(~offset, 0) << 3);
}

void namco_c123tmap_device::control8_w(offs_t offset, uint8_t data)
{
	if ((m_tilemapinfo.control[offset] & 0xff) == data)
		return;

	m_tilemapinfo.control[offset] = data & 0xff;
	if ((offset >= 0x10) && (offset <= 0x15))
	{
		m_tilemapinfo.tmap[offset & 7]->enable(BIT(~data, 3));
	}
	else if ((offset >= 0x18) && (offset <= 0x1d))
	{
		m_tilemapinfo.tmap[offset & 7]->set_palette_offset(((data & 7) << 8) + m_color_base);
	}
}

uint8_t namco_c123tmap_device::control8_r(offs_t offset)
{
	return m_tilemapinfo.control[offset] & 0xff;
}
