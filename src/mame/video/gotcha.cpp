// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/gotcha.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(gotcha_state::gotcha_tilemap_scan)
{
	return (col & 0x1f) | (row << 5) | ((col & 0x20) << 5);
}

inline void gotcha_state::get_tile_info( tile_data &tileinfo, int tile_index ,UINT16 *vram, int color_offs)
{
	UINT16 data = vram[tile_index];
	int code = (data & 0x3ff) | (m_gfxbank[(data & 0x0c00) >> 10] << 10);

	SET_TILE_INFO_MEMBER(0, code, (data >> 12) + color_offs, 0);
}

TILE_GET_INFO_MEMBER(gotcha_state::fg_get_tile_info)
{
	get_tile_info(tileinfo, tile_index, m_fgvideoram, 0);
}

TILE_GET_INFO_MEMBER(gotcha_state::bg_get_tile_info)
{
	get_tile_info(tileinfo, tile_index, m_bgvideoram, 16);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void gotcha_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gotcha_state::fg_get_tile_info),this), tilemap_mapper_delegate(FUNC(gotcha_state::gotcha_tilemap_scan),this), 16, 16, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gotcha_state::bg_get_tile_info),this), tilemap_mapper_delegate(FUNC(gotcha_state::gotcha_tilemap_scan),this), 16, 16, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);

	m_fg_tilemap->set_scrolldx(-1, 0);
	m_bg_tilemap->set_scrolldx(-5, 0);
}


WRITE16_MEMBER(gotcha_state::gotcha_fgvideoram_w)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(gotcha_state::gotcha_bgvideoram_w)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(gotcha_state::gotcha_gfxbank_select_w)
{
	if (ACCESSING_BITS_8_15)
		m_banksel = (data & 0x0300) >> 8;
}

WRITE16_MEMBER(gotcha_state::gotcha_gfxbank_w)
{
	if (ACCESSING_BITS_8_15)
	{
		if (m_gfxbank[m_banksel] != ((data & 0x0f00) >> 8))
		{
			m_gfxbank[m_banksel] = (data & 0x0f00) >> 8;
			machine().tilemap().mark_all_dirty();
		}
	}
}

WRITE16_MEMBER(gotcha_state::gotcha_scroll_w)
{
	COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_fg_tilemap->set_scrollx(0, m_scroll[0]); break;
		case 1: m_fg_tilemap->set_scrolly(0, m_scroll[1]); break;
		case 2: m_bg_tilemap->set_scrollx(0, m_scroll[2]); break;
		case 3: m_bg_tilemap->set_scrolly(0, m_scroll[3]); break;
	}
}





UINT32 gotcha_state::screen_update_gotcha(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}
