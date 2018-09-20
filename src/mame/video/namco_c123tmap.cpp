// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino

/*
	C123 Tilemaps
	used by
	namcos2.cpp (all games)
	namcofl.cpp (all games)
	namconb1.cpp (all games)

	should be used by
	namcos1.cpp (all games)

	(TODO: merge with namcos1.cpp implementation)
*/

#include "emu.h"
#include "namco_c123tmap.h"

DEFINE_DEVICE_TYPE(NAMCO_C123TMAP, namco_c123tmap_device, "namco_c123tmap", "Namco C123 (4x + 2x Tilemaps)")

namco_c123tmap_device::namco_c123tmap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCO_C123TMAP, tag, owner, clock),
	m_gfxdecode(*this, finder_base::DUMMY_TAG),
	m_maskregion(*this, finder_base::DUMMY_TAG)
{
}

void namco_c123tmap_device::device_start()
{
	m_tilemapinfo.maskBaseAddr = m_maskregion->base();

	m_tilemapinfo.videoram = std::make_unique<uint16_t[]>(0x10000);

	/* four scrolling tilemaps */
	m_tilemapinfo.tmap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namco_c123tmap_device::get_tile_info<0x0000>), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemapinfo.tmap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namco_c123tmap_device::get_tile_info<0x1000>), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemapinfo.tmap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namco_c123tmap_device::get_tile_info<0x2000>), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemapinfo.tmap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namco_c123tmap_device::get_tile_info<0x3000>), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	/* two non-scrolling tilemaps */
	m_tilemapinfo.tmap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namco_c123tmap_device::get_tile_info<0x4008>), this), TILEMAP_SCAN_ROWS, 8, 8, 36, 28);
	m_tilemapinfo.tmap[5] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namco_c123tmap_device::get_tile_info<0x4408>), this), TILEMAP_SCAN_ROWS, 8, 8, 36, 28);

	/* define offsets for scrolling */
	for (int i = 0; i < 4; i++)
	{
		static const int adj[4] = { 4,2,1,0 };
		int dx = 44 + adj[i];
		m_tilemapinfo.tmap[i]->set_scrolldx(-dx, 288 + dx);
		m_tilemapinfo.tmap[i]->set_scrolldy(-24, 224 + 24);
	}


	save_item(NAME(m_tilemapinfo.control));
	save_pointer(NAME(m_tilemapinfo.videoram), 0x10000);
}

/**************************************************************************************/

void namco_c123tmap_device::mark_all_dirty(void)
{
	for (int i = 0; i < 6; i++)
	{
		m_tilemapinfo.tmap[i]->mark_all_dirty();
	}
} /* namco_tilemap_invalidate */

template<int Offset>
TILE_GET_INFO_MEMBER(namco_c123tmap_device::get_tile_info)
{
	const uint16_t *vram = &m_tilemapinfo.videoram[Offset];
	int tile, mask;
	m_tilemapinfo.cb(vram[tile_index], &tile, &mask);
	tileinfo.mask_data = m_tilemapinfo.maskBaseAddr + mask * 8;
	SET_TILE_INFO_MEMBER(m_tilemapinfo.gfxbank, tile, 0, 0);
} /* get_tile_info */

void namco_c123tmap_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	for (int i = 0; i < 6; i++)
	{
		/* note: priority is only in range 0..7, but Point Blank uses 0xf to hide a layer */
		if ((m_tilemapinfo.control[0x20 / 2 + i] & 0xf) == pri)
		{
			int color = m_tilemapinfo.control[0x30 / 2 + i] & 0x07;
			m_tilemapinfo.tmap[i]->set_palette_offset(color * 256);
			m_tilemapinfo.tmap[i]->draw(screen, bitmap, cliprect, 0, 0);
		}
	}
} /* draw */

void namco_c123tmap_device::set_tilemap_videoram(int offset, uint16_t newword)
{
	m_tilemapinfo.videoram[offset] = newword;
	if (offset < 0x4000)
	{
		m_tilemapinfo.tmap[offset >> 12]->mark_tile_dirty(offset & 0xfff);
	}
	else if (offset >= 0x8010 / 2 && offset < 0x87f0 / 2)
	{ /* fixed plane#1 */
		offset -= 0x8010 / 2;
		m_tilemapinfo.tmap[4]->mark_tile_dirty(offset);
	}
	else if (offset >= 0x8810 / 2 && offset < 0x8ff0 / 2)
	{ /* fixed plane#2 */
		offset -= 0x8810 / 2;
		m_tilemapinfo.tmap[5]->mark_tile_dirty(offset);
	}
} /* SetTilemapVideoram */

WRITE16_MEMBER(namco_c123tmap_device::videoram_w)
{
	uint16_t newword = m_tilemapinfo.videoram[offset];
	COMBINE_DATA(&newword);
	set_tilemap_videoram(offset, newword);
}

READ16_MEMBER(namco_c123tmap_device::videoram_r)
{
	return m_tilemapinfo.videoram[offset];
}

void namco_c123tmap_device::set_tilemap_control(int offset, uint16_t newword)
{
	m_tilemapinfo.control[offset] = newword;
	if (offset < 0x20 / 2)
	{
		if (offset == 0x02 / 2)
		{
			/* all planes are flipped X+Y from D15 of this word */
			int attrs = (newword & 0x8000) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
			int i;
			for (i = 0; i <= 5; i++)
			{
				m_tilemapinfo.tmap[i]->set_flip(attrs);
			}
		}
	}
	newword &= 0x1ff;
	if (m_tilemapinfo.control[0x02 / 2] & 0x8000)
	{
		newword = -newword;
	}
	switch (offset)
	{
	case 0x02 / 2:
		m_tilemapinfo.tmap[0]->set_scrollx(0, newword);
		break;
	case 0x06 / 2:
		m_tilemapinfo.tmap[0]->set_scrolly(0, newword);
		break;
	case 0x0a / 2:
		m_tilemapinfo.tmap[1]->set_scrollx(0, newword);
		break;
	case 0x0e / 2:
		m_tilemapinfo.tmap[1]->set_scrolly(0, newword);
		break;
	case 0x12 / 2:
		m_tilemapinfo.tmap[2]->set_scrollx(0, newword);
		break;
	case 0x16 / 2:
		m_tilemapinfo.tmap[2]->set_scrolly(0, newword);
		break;
	case 0x1a / 2:
		m_tilemapinfo.tmap[3]->set_scrollx(0, newword);
		break;
	case 0x1e / 2:
		m_tilemapinfo.tmap[3]->set_scrolly(0, newword);
		break;
	}
} /* SetTilemapControl */

WRITE16_MEMBER(namco_c123tmap_device::control_w)
{
	uint16_t newword = m_tilemapinfo.control[offset];
	COMBINE_DATA(&newword);
	set_tilemap_control(offset, newword);
}

READ16_MEMBER(namco_c123tmap_device::control_r)
{
	return m_tilemapinfo.control[offset];
}

