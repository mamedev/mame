#include "emu.h"

#include "includes/parodius.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void parodius_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	parodius_state *state = machine.driver_data<parodius_state>();
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

void parodius_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	parodius_state *state = machine.driver_data<parodius_state>();
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= state->m_layerpri[2])
		*priority_mask = 0;
	else if (pri > state->m_layerpri[2] && pri <= state->m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->m_layerpri[1] && pri <= state->m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = state->m_sprite_colorbase + (*color & 0x1f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

UINT32 parodius_state::screen_update_parodius(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[3], bg_colorbase;

	bg_colorbase = m_k053251->get_palette_index(K053251_CI0);
	m_sprite_colorbase   = m_k053251->get_palette_index(K053251_CI1);
	m_layer_colorbase[0] = m_k053251->get_palette_index(K053251_CI2);
	m_layer_colorbase[1] = m_k053251->get_palette_index(K053251_CI4);
	m_layer_colorbase[2] = m_k053251->get_palette_index(K053251_CI3);

	m_k052109->tilemap_update();

	layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI2);
	layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI4);
	layer[2] = 2;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI3);

	konami_sortlayers3(layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[0], 0,1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[1], 0,2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[2], 0,4);

	m_k053245->k053245_sprites_draw(bitmap, cliprect, screen.priority());
	return 0;
}
