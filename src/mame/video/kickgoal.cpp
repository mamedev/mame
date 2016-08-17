// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Kick Goal - video */

#include "emu.h"
#include "includes/kickgoal.h"


WRITE16_MEMBER(kickgoal_state::kickgoal_fgram_w)
{
	COMBINE_DATA(&m_fgram[offset]);
	m_fgtm->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(kickgoal_state::kickgoal_bgram_w)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bgtm->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(kickgoal_state::kickgoal_bg2ram_w)
{
	COMBINE_DATA(&m_bg2ram[offset]);
	m_bg2tm->mark_tile_dirty(offset / 2);
}

/* FG */
TILE_GET_INFO_MEMBER(kickgoal_state::get_kickgoal_fg_tile_info)
{
	int tileno = m_fgram[tile_index * 2] & 0x0fff;
	int color = m_fgram[tile_index * 2 + 1] & 0x000f;

	SET_TILE_INFO_MEMBER(0, tileno + m_fg_base, color + 0x00, 0);
}

/* BG */
TILE_GET_INFO_MEMBER(kickgoal_state::get_kickgoal_bg_tile_info)
{
	int tileno = m_bgram[tile_index * 2] & m_bg_mask;
	int color = m_bgram[tile_index * 2 + 1] & 0x000f;
	int flipx = m_bgram[tile_index * 2 + 1] & 0x0020;
	int flipy = m_bgram[tile_index * 2 + 1] & 0x0040;

	SET_TILE_INFO_MEMBER(1, tileno + m_bg_base, color + 0x10, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}

/* BG 2 */
TILE_GET_INFO_MEMBER(kickgoal_state::get_kickgoal_bg2_tile_info)
{
	int tileno = m_bg2ram[tile_index * 2] & m_bg2_mask;
	int color = m_bg2ram[tile_index * 2 + 1] & 0x000f;
	int flipx = m_bg2ram[tile_index * 2 + 1] & 0x0020;
	int flipy = m_bg2ram[tile_index * 2 + 1] & 0x0040;

	SET_TILE_INFO_MEMBER(m_bg2_region, tileno + m_bg2_base, color + 0x20, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}


TILEMAP_MAPPER_MEMBER(kickgoal_state::tilemap_scan_kicksfg)
{
	/* logical (col,row) -> memory offset */
	return col * 32 + (row & 0x1f) + ((row & 0x20) >> 5) * 0x800;
}

TILEMAP_MAPPER_MEMBER(kickgoal_state::tilemap_scan_kicksbg)
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}


TILEMAP_MAPPER_MEMBER(kickgoal_state::tilemap_scan_kicksbg2)// 16x16 tiles
{
	/* logical (col,row) -> memory offset */
	return col * 8 + (row & 0x7) + ((row & 0x3c) >> 3) * 0x200;
}


TILEMAP_MAPPER_MEMBER(kickgoal_state::tilemap_scan_actionhwbg2)// 32x32 tiles
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}









void kickgoal_state::kickgoal_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT16 *spriteram = m_spriteram;
	gfx_element *gfx = m_gfxdecode->gfx(1);
	int offs;

	for (offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		int xpos = spriteram[offs + 3];
		int ypos = spriteram[offs + 0] & 0x00ff;
		int tileno = spriteram[offs + 2] & 0x3fff;
		int flipx = spriteram[offs + 1] & 0x0020;
		int color = spriteram[offs + 1] & 0x000f;

		if (spriteram[offs + 0] & 0x0100) break;

		ypos = 0x110 - ypos;

		gfx->transpen(bitmap,cliprect,
				tileno+m_sprbase,
				0x30 + color,
				flipx,0,
				xpos-16+4,ypos-32,15);
	}
}


VIDEO_START_MEMBER(kickgoal_state,kickgoal)
{
	m_sprbase = 0x0000;

	m_fg_base = 0x7000;
	m_bg_base = 0x1000;
	m_bg_mask = 0x0fff;

	m_bg2_region = 2; // 32x32 tile source
	m_bg2_base = 0x2000 / 4;
	m_bg2_mask = (0x2000/4) - 1;

	m_fgtm = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kickgoal_state::get_kickgoal_fg_tile_info),this), tilemap_mapper_delegate(FUNC(kickgoal_state::tilemap_scan_kicksfg),this), 8, 8, 64, 64);
	m_bgtm = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kickgoal_state::get_kickgoal_bg_tile_info),this), tilemap_mapper_delegate(FUNC(kickgoal_state::tilemap_scan_kicksbg),this), 16, 16, 64, 64);
	m_bg2tm = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kickgoal_state::get_kickgoal_bg2_tile_info),this), tilemap_mapper_delegate(FUNC(kickgoal_state::tilemap_scan_kicksbg2),this), 32, 32, 64, 64);

	m_fgtm->set_transparent_pen(15);
	m_bgtm->set_transparent_pen(15);
}

VIDEO_START_MEMBER(kickgoal_state,actionhw)
{
	m_sprbase = 0x4000;
	m_fg_base = 0x7000 * 2;

	m_bg_base = 0x0000;
	m_bg_mask = 0x1fff;

	m_bg2_region = 1; // 16x16 tile source
	m_bg2_base = 0x2000;
	m_bg2_mask = 0x2000 - 1;

	m_fgtm = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kickgoal_state::get_kickgoal_fg_tile_info),this), tilemap_mapper_delegate(FUNC(kickgoal_state::tilemap_scan_kicksfg),this), 8, 8, 64, 64);
	m_bgtm = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kickgoal_state::get_kickgoal_bg_tile_info),this), tilemap_mapper_delegate(FUNC(kickgoal_state::tilemap_scan_kicksbg),this), 16, 16, 64, 64);
	m_bg2tm = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kickgoal_state::get_kickgoal_bg2_tile_info),this), tilemap_mapper_delegate(FUNC(kickgoal_state::tilemap_scan_actionhwbg2),this), 16, 16, 64, 64);

	m_fgtm->set_transparent_pen(15);
	m_bgtm->set_transparent_pen(15);
}



UINT32 kickgoal_state::screen_update_kickgoal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* set scroll */
	m_fgtm->set_scrollx(0, m_scrram[0]);
	m_fgtm->set_scrolly(0, m_scrram[1]);
	m_bgtm->set_scrollx(0, m_scrram[2]);
	m_bgtm->set_scrolly(0, m_scrram[3]);
	m_bg2tm->set_scrollx(0, m_scrram[4]);
	m_bg2tm->set_scrolly(0, m_scrram[5]);

	/* draw */
	m_bg2tm->draw(screen, bitmap, cliprect, 0, 0);
	m_bgtm->draw(screen, bitmap, cliprect, 0, 0);

	kickgoal_draw_sprites(bitmap, cliprect);

	m_fgtm->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
