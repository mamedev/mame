#include "emu.h"
#include "includes/xexex.h"

void xexex_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	xexex_state *state = machine.driver_data<xexex_state>();
	int pri;

	// Xexex doesn't seem to use bit8 and 9 as effect selectors so this should be safe.
	// (pdrawgfx() still needs change to fix Elaine's end-game graphics)
	pri = (*color & 0x3e0) >> 4;

	if (pri <= state->m_layerpri[3])
		*priority_mask = 0;
	else if (pri > state->m_layerpri[3] && pri <= state->m_layerpri[2])
		*priority_mask = 0xff00;
	else if (pri > state->m_layerpri[2] && pri <= state->m_layerpri[1])
		*priority_mask = 0xff00 | 0xf0f0;
	else if (pri > state->m_layerpri[1] && pri <= state->m_layerpri[0])
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc;
	else
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa;

	*color = state->m_sprite_colorbase | (*color & 0x001f);
}

void xexex_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags)
{
	xexex_state *state = machine.driver_data<xexex_state>();
	*color = state->m_layer_colorbase[layer] | (*color >> 2 & 0x0f);
}

void xexex_state::video_start()
{
	assert(m_screen->format() == BITMAP_FORMAT_RGB32);

	m_cur_alpha = 0;

	// Xexex has relative plane offsets of -2,2,4,6 vs. -2,0,2,3 in MW and GX.
	m_k056832->set_layer_offs(0, -2, 16);
	m_k056832->set_layer_offs(1,  2, 16);
	m_k056832->set_layer_offs(2,  4, 16);
	m_k056832->set_layer_offs(3,  6, 16);
}

UINT32 xexex_state::screen_update_xexex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layer[4];
	int bg_colorbase, new_colorbase, plane, alpha;

	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI0);
	bg_colorbase = m_k053251->get_palette_index(K053251_CI1);
	m_layer_colorbase[0] = 0x70;

	for (plane = 1; plane < 4; plane++)
	{
		new_colorbase = m_k053251->get_palette_index(K053251_CI[plane]);
		if (m_layer_colorbase[plane] != new_colorbase)
		{
			m_layer_colorbase[plane] = new_colorbase;
			m_k056832->mark_plane_dirty( plane);
		}
	}

	layer[0] = 1;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI2);
	layer[1] = 2;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI3);
	layer[2] = 3;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI4);
	layer[3] = -1;
	m_layerpri[3] = m_k053251->get_priority(K053251_CI1);

	konami_sortlayers4(layer, m_layerpri);

	m_k054338->update_all_shadows(0, m_palette);
	m_k054338->fill_backcolor(bitmap, 0);

	screen.priority().fill(0, cliprect);

	for (plane = 0; plane < 4; plane++)
	{
		if (layer[plane] < 0)
		{
			m_k053250->draw(bitmap, cliprect, bg_colorbase, 0, screen.priority(), 1 << plane);
		}
		else if (!m_cur_alpha || layer[plane] != 1)
		{
			m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[plane], 0, 1 << plane);
		}
	}

	m_k053246->k053247_sprites_draw( bitmap, cliprect);

	if (m_cur_alpha)
	{
		alpha = m_k054338->set_alpha_level(1);

		if (alpha > 0)
		{
			m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_ALPHA(alpha), 0);
		}
	}

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	return 0;
}
