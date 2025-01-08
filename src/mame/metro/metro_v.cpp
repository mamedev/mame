// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

    (legacy metro.cpp, currently contains Blazing Tornado overrides,
     to be moved into its own driver file!)

***************************************************************************/

#include "emu.h"
#include "metro.h"

TILE_GET_INFO_MEMBER(blzntrnd_state::k053936_get_tile_info)
{
	int code = m_k053936_ram[tile_index];

	tileinfo.set(0,
			code & 0x7fff,
			0xe,
			0);
}

TILE_GET_INFO_MEMBER(blzntrnd_state::k053936_gstrik2_get_tile_info)
{
	int code = m_k053936_ram[tile_index];

	tileinfo.set(0,
			(code & 0x7fff) >> 2,
			0xe,
			0);
}

void blzntrnd_state::k053936_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_k053936_ram[offset]);
	m_k053936_tilemap->mark_tile_dirty(offset);
}

TILEMAP_MAPPER_MEMBER(blzntrnd_state::tilemap_scan_gstrik2)
{
	/* logical (col,row) -> memory offset */
	return ((row & 0x40) >> 6) | (col << 1) | ((row & 0x80) << 1) | ((row & 0x3f) << 9);
}

VIDEO_START_MEMBER(blzntrnd_state,blzntrnd)
{
	m_k053936_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(blzntrnd_state::k053936_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 256, 512);
}

VIDEO_START_MEMBER(blzntrnd_state,gstrik2)
{
	m_k053936_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(blzntrnd_state::k053936_gstrik2_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(blzntrnd_state::tilemap_scan_gstrik2)), 16, 16, 128, 256);
}

u32 blzntrnd_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* TODO: bit 5 of reg 7 is off when ROZ is supposed to be disabled
	 * (Blazing Tornado title screen/character select/ending and Grand Striker 2 title/how to play transition)
	 */

	bitmap.fill(m_vdp2->get_background_pen(), cliprect);
	m_k053936->zoom_draw(screen, bitmap, cliprect, m_k053936_tilemap, 0, 0, 1); // FIXME!!!
	m_vdp2->draw_foreground(screen, bitmap, cliprect);

	return 0;
}
