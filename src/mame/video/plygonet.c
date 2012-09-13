/*
    Polygonet Commanders (Konami, 1993)

    Video hardware emulation

    Currently contains: TTL text plane (probably not complete, needs banking? colors?)
    Needs: PSAC2 roz plane, polygons
*/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/plygonet.h"

/* TTL text plane */

TILE_GET_INFO_MEMBER(polygonet_state::ttl_get_tile_info)
{
	int attr, code;

	code = m_ttl_vram[tile_index]&0xfff;

	attr = m_ttl_vram[tile_index]>>12;	/* palette in all 4 bits? */

	SET_TILE_INFO_MEMBER(m_ttl_gfx_index, code, attr, 0);
}

TILE_GET_INFO_MEMBER(polygonet_state::roz_get_tile_info)
{
	int attr, code;

	attr = (m_roz_vram[tile_index] >> 12) + 16;	/* roz base palette is palette 16 */
	code = m_roz_vram[tile_index] & 0x3ff;

	SET_TILE_INFO_MEMBER(0, code, attr, 0);
}

READ32_MEMBER(polygonet_state::polygonet_ttl_ram_r)
{
	UINT32 *vram = (UINT32 *)m_ttl_vram;

	return vram[offset];
}

WRITE32_MEMBER(polygonet_state::polygonet_ttl_ram_w)
{
	UINT32 *vram = (UINT32 *)m_ttl_vram;

	COMBINE_DATA(&vram[offset]);

	m_ttl_tilemap->mark_tile_dirty(offset*2);
	m_ttl_tilemap->mark_tile_dirty(offset*2+1);
}

READ32_MEMBER(polygonet_state::polygonet_roz_ram_r)
{
	UINT32 *vram = (UINT32 *)m_roz_vram;

	return vram[offset];
}

WRITE32_MEMBER(polygonet_state::polygonet_roz_ram_w)
{
	UINT32 *vram = (UINT32 *)m_roz_vram;

	COMBINE_DATA(&vram[offset]);

	m_roz_tilemap->mark_tile_dirty(offset*2);
	m_roz_tilemap->mark_tile_dirty(offset*2+1);
}

TILEMAP_MAPPER_MEMBER(polygonet_state::plygonet_scan)
{
	return row * num_cols + (col^1);
}

TILEMAP_MAPPER_MEMBER(polygonet_state::plygonet_scan_cols)
{
	return col * num_rows + (row^1);
}

void polygonet_state::video_start()
{
	static const gfx_layout charlayout =
	{
		8, 8,	/* 8x8 */
		4096,	/* # of tiles */
		4,		/* 4bpp */
		{ 0, 1, 2, 3 },	/* plane offsets */
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },	/* X offsets */
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },	/* Y offsets */
		8*8*4
	};

	/* find first empty slot to decode gfx */
	for (m_ttl_gfx_index = 0; m_ttl_gfx_index < MAX_GFX_ELEMENTS; m_ttl_gfx_index++)
		if (machine().gfx[m_ttl_gfx_index] == 0)
			break;

	assert(m_ttl_gfx_index != MAX_GFX_ELEMENTS);

	/* decode the ttl layer's gfx */
	machine().gfx[m_ttl_gfx_index] = auto_alloc(machine(), gfx_element(machine(), charlayout, machine().root_device().memregion("gfx1")->base(), machine().total_colors() / 16, 0));

	/* create the tilemap */
	m_ttl_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(polygonet_state::ttl_get_tile_info),this), tilemap_mapper_delegate(FUNC(polygonet_state::plygonet_scan),this),  8, 8, 64, 32);

	m_ttl_tilemap->set_transparent_pen(0);

	/* set up the roz t-map too */
	m_roz_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(polygonet_state::roz_get_tile_info),this), tilemap_mapper_delegate(FUNC(polygonet_state::plygonet_scan_cols),this), 16, 16, 32, 64);
	m_roz_tilemap->set_transparent_pen(0);

	/* save states */
	save_item(NAME(m_ttl_gfx_index));
	save_item(NAME(m_ttl_vram));
	save_item(NAME(m_roz_vram));
}

SCREEN_UPDATE_IND16( polygonet )
{
	polygonet_state *state = screen.machine().driver_data<polygonet_state>();
	device_t *k053936 = screen.machine().device("k053936");
	screen.machine().priority_bitmap.fill(0);
	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	k053936_zoom_draw(k053936, bitmap, cliprect, state->m_roz_tilemap, 0, 0, 0);

	state->m_ttl_tilemap->draw(bitmap, cliprect, 0, 1<<0);
	return 0;
}

