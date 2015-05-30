// license:BSD-3-Clause
// copyright-holders:R. Belmont, Acho A. Tang
/***************************************************************************

 Wild West C.O.W.boys of Moo Mesa
 Bucky O'Hare
 (c) 1992 Konami

 Video hardware emulation.

***************************************************************************/

#include "emu.h"
#include "includes/moo.h"

K053246_CB_MEMBER(moo_state::sprite_callback)
{
	int pri = (*color & 0x03e0) >> 4;

	if (pri <= m_layerpri[2])
		*priority_mask = 0;
	else if (pri <= m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri <= m_layerpri[0])
		*priority_mask = 0xf0|0xcc;
	else
		*priority_mask = 0xf0|0xcc|0xaa;

	*color = m_sprite_colorbase | (*color & 0x001f);
}

K056832_CB_MEMBER(moo_state::tile_callback)
{
	*color = m_layer_colorbase[layer] | (*color >> 2 & 0x0f);
}

VIDEO_START_MEMBER(moo_state,moo)
{
	assert(m_screen->format() == BITMAP_FORMAT_RGB32);

	m_alpha_enabled = 0;
	m_zmask = 0xffff;

	// other than the intro showing one blank line alignment is good through the game
	m_k056832->set_layer_offs(0, -2 + 1, 0);
	m_k056832->set_layer_offs(1,  2 + 1, 0);
	m_k056832->set_layer_offs(2,  4 + 1, 0);
	m_k056832->set_layer_offs(3,  6 + 1, 0);
}

VIDEO_START_MEMBER(moo_state,bucky)
{
	assert(m_screen->format() == BITMAP_FORMAT_RGB32);

	m_alpha_enabled = 0;
	m_zmask = 0x00ff;

	// Bucky doesn't chain tilemaps
	m_k056832->set_layer_association(0);

	m_k056832->set_layer_offs(0, -2, 0);
	m_k056832->set_layer_offs(1,  2, 0);
	m_k056832->set_layer_offs(2,  4, 0);
	m_k056832->set_layer_offs(3,  6, 0);
}

UINT32 moo_state::screen_update_moo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layers[3];
	int new_colorbase, plane, dirty, alpha;

	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI0);
	m_layer_colorbase[0] = 0x70;

	if (m_k056832->get_layer_association())
	{
		for (plane = 1; plane < 4; plane++)
		{
			new_colorbase = m_k053251->get_palette_index(K053251_CI[plane]);
			if (m_layer_colorbase[plane] != new_colorbase)
			{
				m_layer_colorbase[plane] = new_colorbase;
				m_k056832->mark_plane_dirty( plane);
			}
		}
	}
	else
	{
		for (dirty = 0, plane = 1; plane < 4; plane++)
		{
			new_colorbase = m_k053251->get_palette_index(K053251_CI[plane]);
			if (m_layer_colorbase[plane] != new_colorbase)
			{
				m_layer_colorbase[plane] = new_colorbase;
				dirty = 1;
			}
		}
		if (dirty)
			m_k056832->mark_all_tilemaps_dirty();
	}

	layers[0] = 1;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI2);
	layers[1] = 2;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI3);
	layers[2] = 3;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI4);

	konami_sortlayers3(layers, m_layerpri);

	m_k054338->update_all_shadows(0, m_palette);
	m_k054338->fill_solid_bg(bitmap, cliprect);

	screen.priority().fill(0, cliprect);

	if (m_layerpri[0] < m_k053251->get_priority(K053251_CI1))   /* bucky hides back layer behind background */
		m_k056832->tilemap_draw(screen, bitmap, cliprect, layers[0], 0, 1);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, layers[1], 0, 2);

	// Enabling alpha improves fog and fading in Moo but causes other things to disappear.
	// There is probably a control bit somewhere to turn off alpha blending.
	m_alpha_enabled = m_k054338->register_r(K338_REG_CONTROL) & K338_CTL_MIXPRI; // DUMMY

	alpha = (m_alpha_enabled) ? m_k054338->set_alpha_level(1) : 255;

	if (alpha > 0)
		m_k056832->tilemap_draw(screen, bitmap, cliprect, layers[2], TILEMAP_DRAW_ALPHA(alpha), 4);

	m_k053246->k053247_sprites_draw( bitmap, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	return 0;
}
