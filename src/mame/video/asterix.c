// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "includes/asterix.h"


void asterix_state::reset_spritebank()
{
	m_k053244->bankselect(m_spritebank & 7);
	m_spritebanks[0] = (m_spritebank << 12) & 0x7000;
	m_spritebanks[1] = (m_spritebank <<  9) & 0x7000;
	m_spritebanks[2] = (m_spritebank <<  6) & 0x7000;
	m_spritebanks[3] = (m_spritebank <<  3) & 0x7000;
}

WRITE16_MEMBER(asterix_state::asterix_spritebank_w)
{
	COMBINE_DATA(&m_spritebank);
	reset_spritebank();
}

K05324X_CB_MEMBER(asterix_state::sprite_callback)
{
	int pri = (*color & 0x00e0) >> 2;
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;
	*color = m_sprite_colorbase | (*color & 0x001f);
	*code = (*code & 0xfff) | m_spritebanks[(*code >> 12) & 3];
}


K056832_CB_MEMBER(asterix_state::tile_callback)
{
	*flags = *code & 0x1000 ? TILE_FLIPX : 0;
	*color = (m_layer_colorbase[layer] + ((*code & 0xe000) >> 13)) & 0x7f;
	*code = (*code & 0x03ff) | m_tilebanks[(*code >> 10) & 3];
}

UINT32 asterix_state::screen_update_asterix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const int K053251_CI[4] = { K053251_CI0, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layer[3], plane, new_colorbase;

	/* Layer offsets are different if horizontally flipped */
	if (m_k056832->read_register(0x0) & 0x10)
	{
		m_k056832->set_layer_offs(0, 89 - 176, 0);
		m_k056832->set_layer_offs(1, 91 - 176, 0);
		m_k056832->set_layer_offs(2, 89 - 176, 0);
		m_k056832->set_layer_offs(3, 95 - 176, 0);
	}
	else
	{
		m_k056832->set_layer_offs(0, 89, 0);
		m_k056832->set_layer_offs(1, 91, 0);
		m_k056832->set_layer_offs(2, 89, 0);
		m_k056832->set_layer_offs(3, 95, 0);
	}


	m_tilebanks[0] = (m_k056832->get_lookup(0) << 10);
	m_tilebanks[1] = (m_k056832->get_lookup(1) << 10);
	m_tilebanks[2] = (m_k056832->get_lookup(2) << 10);
	m_tilebanks[3] = (m_k056832->get_lookup(3) << 10);

	// update color info and refresh tilemaps
	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI1);

	for (plane = 0; plane < 4; plane++)
	{
		new_colorbase = m_k053251->get_palette_index(K053251_CI[plane]);
		if (m_layer_colorbase[plane] != new_colorbase)
		{
			m_layer_colorbase[plane] = new_colorbase;
			m_k056832->mark_plane_dirty(plane);
		}
	}

	layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI0);
	layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI2);
	layer[2] = 3;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI4);

	konami_sortlayers3(layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[0], K056832_DRAW_FLAG_MIRROR, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[1], K056832_DRAW_FLAG_MIRROR, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[2], K056832_DRAW_FLAG_MIRROR, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows in front of feet */
	m_k053244->sprites_draw(bitmap, cliprect, screen.priority());

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, K056832_DRAW_FLAG_MIRROR, 0);
	return 0;
}
