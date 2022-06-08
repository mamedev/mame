// license:BSD-3-Clause
// copyright-holders:R. Belmont, Andrew Gardner
/*
    Polygonet Commanders (Konami, 1993)

    Video hardware emulation

    Currently contains: TTL text plane (probably not complete, needs banking? colors?)
    Needs: PSAC2 roz plane, polygons
*/

#include "emu.h"
#include "includes/plygonet.h"
#include "screen.h"


/* TTL text plane */

TILE_GET_INFO_MEMBER(polygonet_state::ttl_get_tile_info)
{
	uint16_t *ttl_vram = (uint16_t *)&m_ttl_vram[0];
	int code = ttl_vram[tile_index] & 0xfff;
	int attr = ttl_vram[tile_index] >> 12;  /* palette in all 4 bits? */

	tileinfo.set(m_ttl_gfx_index, code, attr, 0);
}

TILE_GET_INFO_MEMBER(polygonet_state::roz_get_tile_info)
{
	uint16_t *roz_vram = (uint16_t *)&m_roz_vram[0];
	int code = roz_vram[tile_index] & 0x3ff;
	int attr = (roz_vram[tile_index] >> 12) + 16; /* roz base palette is palette 16 */

	tileinfo.set(0, code, attr, 0);
}

void polygonet_state::ttl_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ttl_vram[offset]);
	m_ttl_tilemap->mark_tile_dirty(offset << 1);
	m_ttl_tilemap->mark_tile_dirty((offset << 1) + 1);
}

void polygonet_state::roz_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_roz_vram[offset]);
	m_roz_tilemap->mark_tile_dirty(offset << 1);
	m_roz_tilemap->mark_tile_dirty((offset << 1) + 1);
}

TILEMAP_MAPPER_MEMBER(polygonet_state::scan_rows)
{
	return row * num_cols + (col ^ 1);
}

TILEMAP_MAPPER_MEMBER(polygonet_state::scan_cols)
{
	return col * num_rows + (row ^ 1);
}

void polygonet_state::video_start()
{
	static const gfx_layout charlayout =
	{
		8, 8,   /* 8x8 */
		4096,   /* # of tiles */
		4,      /* 4bpp */
		{ 0, 1, 2, 3 }, /* plane offsets */
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 }, /* X offsets */
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 }, /* Y offsets */
		8*8*4
	};

	/* find first empty slot to decode gfx */
	for (m_ttl_gfx_index = 0; m_ttl_gfx_index < MAX_GFX_ELEMENTS; m_ttl_gfx_index++)
		if (m_gfxdecode->gfx(m_ttl_gfx_index) == nullptr)
			break;

	assert(m_ttl_gfx_index != MAX_GFX_ELEMENTS);

	/* decode the ttl layer's gfx */
	m_gfxdecode->set_gfx(m_ttl_gfx_index, std::make_unique<gfx_element>(m_palette, charlayout, memregion("gfx1")->base(), 0, m_palette->entries() / 16, 0));

	/* create the tilemap */
	m_ttl_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(polygonet_state::ttl_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(polygonet_state::scan_rows)),  8, 8, 64, 32);

	m_ttl_tilemap->set_transparent_pen(0);

	/* set up the roz t-map too */
	m_roz_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(polygonet_state::roz_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(polygonet_state::scan_cols)), 16, 16, 32, 64);
	m_roz_tilemap->set_transparent_pen(0);

	/* save states */
	save_item(NAME(m_ttl_gfx_index));
}

uint32_t polygonet_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0);
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 0, 0);

	m_ttl_tilemap->draw(screen, bitmap, cliprect, 0, 1<<0);
	return 0;
}
