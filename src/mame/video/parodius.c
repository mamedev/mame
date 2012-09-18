#include "emu.h"
#include "video/konicdev.h"
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

	bg_colorbase = k053251_get_palette_index(m_k053251, K053251_CI0);
	m_sprite_colorbase   = k053251_get_palette_index(m_k053251, K053251_CI1);
	m_layer_colorbase[0] = k053251_get_palette_index(m_k053251, K053251_CI2);
	m_layer_colorbase[1] = k053251_get_palette_index(m_k053251, K053251_CI4);
	m_layer_colorbase[2] = k053251_get_palette_index(m_k053251, K053251_CI3);

	k052109_tilemap_update(m_k052109);

	layer[0] = 0;
	m_layerpri[0] = k053251_get_priority(m_k053251, K053251_CI2);
	layer[1] = 1;
	m_layerpri[1] = k053251_get_priority(m_k053251, K053251_CI4);
	layer[2] = 2;
	m_layerpri[2] = k053251_get_priority(m_k053251, K053251_CI3);

	konami_sortlayers3(layer, m_layerpri);

	machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, layer[0], 0,1);
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, layer[1], 0,2);
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, layer[2], 0,4);

	k053245_sprites_draw(m_k053245, bitmap, cliprect);
	return 0;
}
