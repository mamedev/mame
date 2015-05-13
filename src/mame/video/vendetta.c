// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "includes/vendetta.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(vendetta_state::vendetta_tile_callback)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x30) << 6) | ((*color & 0x0c) << 10) | (bank << 14);
	*color = m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

K052109_CB_MEMBER(vendetta_state::esckids_tile_callback)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) <<  9) | (bank << 13);
	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >>  5);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

K053246_CB_MEMBER(vendetta_state::sprite_callback)
{
	int pri = (*color & 0x03e0) >> 4;   /* ??????? */
	if (pri <= m_layerpri[2])
		*priority_mask = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 vendetta_state::screen_update_vendetta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[3];

	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI1);
	m_layer_colorbase[0] = m_k053251->get_palette_index(K053251_CI2);
	m_layer_colorbase[1] = m_k053251->get_palette_index(K053251_CI3);
	m_layer_colorbase[2] = m_k053251->get_palette_index(K053251_CI4);

	m_k052109->tilemap_update();

	layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI2);
	layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI3);
	layer[2] = 2;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI4);

	konami_sortlayers3(layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	m_k053246->k053247_sprites_draw(bitmap, cliprect);
	return 0;
}
