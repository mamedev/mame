#include "emu.h"
#include "video/konicdev.h"
#include "includes/vendetta.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void vendetta_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	vendetta_state *state = machine.driver_data<vendetta_state>();
	*code |= ((*color & 0x03) << 8) | ((*color & 0x30) << 6) | ((*color & 0x0c) << 10) | (bank << 14);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

void esckids_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	vendetta_state *state = machine.driver_data<vendetta_state>();
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) <<  9) | (bank << 13);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xe0) >>  5);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void vendetta_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	vendetta_state *state = machine.driver_data<vendetta_state>();
	int pri = (*color & 0x03e0) >> 4;	/* ??????? */
	if (pri <= state->m_layerpri[2])
		*priority_mask = 0;
	else if (pri > state->m_layerpri[2] && pri <= state->m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->m_layerpri[1] && pri <= state->m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = state->m_sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 vendetta_state::screen_update_vendetta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[3];

	m_sprite_colorbase = k053251_get_palette_index(m_k053251, K053251_CI1);
	m_layer_colorbase[0] = k053251_get_palette_index(m_k053251, K053251_CI2);
	m_layer_colorbase[1] = k053251_get_palette_index(m_k053251, K053251_CI3);
	m_layer_colorbase[2] = k053251_get_palette_index(m_k053251, K053251_CI4);

	k052109_tilemap_update(m_k052109);

	layer[0] = 0;
	m_layerpri[0] = k053251_get_priority(m_k053251, K053251_CI2);
	layer[1] = 1;
	m_layerpri[1] = k053251_get_priority(m_k053251, K053251_CI3);
	layer[2] = 2;
	m_layerpri[2] = k053251_get_priority(m_k053251, K053251_CI4);

	konami_sortlayers3(layer, m_layerpri);

	machine().priority_bitmap.fill(0, cliprect);
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, layer[2], 0, 4);

	k053247_sprites_draw(m_k053246, bitmap, cliprect);
	return 0;
}
