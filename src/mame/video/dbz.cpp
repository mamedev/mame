// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont, Pierpaolo Prazzoli
/*
  Dragonball Z
  (c) 1993 Banpresto
  Dragonball Z 2 Super Battle
  (c) 1994 Banpresto

  Video hardware emulation.
*/


#include "emu.h"
#include "includes/dbz.h"


K056832_CB_MEMBER(dbz_state::tile_callback)
{
	*color = (m_layer_colorbase[layer] << 1) + ((*color & 0x3c) >> 2);
}

K053246_CB_MEMBER(dbz_state::sprite_callback)
{
	int pri = (*color & 0x3c0) >> 5;

	if (pri <= m_layerpri[3])
		*priority_mask = 0xff00;
	else if (pri > m_layerpri[3] && pri <= m_layerpri[2])
		*priority_mask = 0xfff0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority_mask = 0xfffc;
	else
		*priority_mask = 0xfffe;

	*color = (m_sprite_colorbase << 1) + (*color & 0x1f);
}

/* Background Tilemaps */

WRITE16_MEMBER(dbz_state::dbz_bg2_videoram_w)
{
	COMBINE_DATA(&m_bg2_videoram[offset]);
	m_bg2_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(dbz_state::get_dbz_bg2_tile_info)
{
	int tileno, colour, flag;

	tileno = m_bg2_videoram[tile_index * 2 + 1] & 0x7fff;
	colour = (m_bg2_videoram[tile_index * 2] & 0x000f);
	flag = (m_bg2_videoram[tile_index * 2] & 0x0080) ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(0, tileno, colour + (m_layer_colorbase[5] << 1), flag);
}

WRITE16_MEMBER(dbz_state::dbz_bg1_videoram_w)
{
	COMBINE_DATA(&m_bg1_videoram[offset]);
	m_bg1_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(dbz_state::get_dbz_bg1_tile_info)
{
	int tileno, colour, flag;

	tileno = m_bg1_videoram[tile_index * 2 + 1] & 0x7fff;
	colour = (m_bg1_videoram[tile_index * 2] & 0x000f);
	flag = (m_bg1_videoram[tile_index * 2] & 0x0080) ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(1, tileno, colour + (m_layer_colorbase[4] << 1), flag);
}

void dbz_state::video_start()
{
	m_bg1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dbz_state::get_dbz_bg1_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_bg2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(dbz_state::get_dbz_bg2_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);

	m_bg1_tilemap->set_transparent_pen(0);
	m_bg2_tilemap->set_transparent_pen(0);

	if (!strcmp(machine().system().name, "dbz"))
		m_k056832->set_layer_offs(0, -34, -16);
	else
		m_k056832->set_layer_offs(0, -35, -16);

	m_k056832->set_layer_offs(1, -31, -16);
	m_k056832->set_layer_offs(3, -31, -16); //?
}

UINT32 dbz_state::screen_update_dbz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const int K053251_CI[6] = { K053251_CI3, K053251_CI4, K053251_CI4, K053251_CI4, K053251_CI2, K053251_CI1 };
	int layer[5], plane, new_colorbase;

	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI0);

	for (plane = 0; plane < 6; plane++)
	{
		new_colorbase = m_k053251->get_palette_index(K053251_CI[plane]);
		if (m_layer_colorbase[plane] != new_colorbase)
		{
			m_layer_colorbase[plane] = new_colorbase;
			if (plane <= 3)
				m_k056832->mark_plane_dirty( plane);
			else if (plane == 4)
				m_bg1_tilemap->mark_all_dirty();
			else if (plane == 5)
				m_bg2_tilemap->mark_all_dirty();
		}
	}

	//layers priority

	layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI3);
	layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI4);
	layer[2] = 3;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI0);
	layer[3] = 4;
	m_layerpri[3] = m_k053251->get_priority(K053251_CI2);
	layer[4] = 5;
	m_layerpri[4] = m_k053251->get_priority(K053251_CI1);

	konami_sortlayers5(layer, m_layerpri);

	screen.priority().fill(0, cliprect);

	for (plane = 0; plane < 5; plane++)
	{
		int flag, pri;

		if (plane == 0)
		{
			flag = TILEMAP_DRAW_OPAQUE;
			pri = 0;
		}
		else
		{
			flag = 0;
			pri = 1 << (plane - 1);
		}

		if(layer[plane] == 4)
			m_k053936_2->zoom_draw(screen, bitmap, cliprect, m_bg1_tilemap, flag, pri, 1);
		else if(layer[plane] == 5)
			m_k053936_1->zoom_draw(screen, bitmap, cliprect, m_bg2_tilemap, flag, pri, 1);
		else
			m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[plane], flag, pri);
	}

	m_k053246->k053247_sprites_draw( bitmap, cliprect);
	return 0;
}
