#include "emu.h"
#include "video/konicdev.h"
#include "includes/asterix.h"


static void reset_spritebank( running_machine &machine )
{
	asterix_state *state = machine.driver_data<asterix_state>();
	k053244_bankselect(state->m_k053244, state->m_spritebank & 7);
	state->m_spritebanks[0] = (state->m_spritebank << 12) & 0x7000;
	state->m_spritebanks[1] = (state->m_spritebank <<  9) & 0x7000;
	state->m_spritebanks[2] = (state->m_spritebank <<  6) & 0x7000;
	state->m_spritebanks[3] = (state->m_spritebank <<  3) & 0x7000;
}

WRITE16_MEMBER(asterix_state::asterix_spritebank_w)
{
	COMBINE_DATA(&m_spritebank);
	reset_spritebank(machine());
}

void asterix_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	asterix_state *state = machine.driver_data<asterix_state>();
	int pri = (*color & 0x00e0) >> 2;
	if (pri <= state->m_layerpri[2])
		*priority_mask = 0;
	else if (pri > state->m_layerpri[2] && pri <= state->m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->m_layerpri[1] && pri <= state->m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;
	*color = state->m_sprite_colorbase | (*color & 0x001f);
	*code = (*code & 0xfff) | state->m_spritebanks[(*code >> 12) & 3];
}


void asterix_tile_callback( running_machine &machine, int layer, int *code, int *color, int *flags )
{
	asterix_state *state = machine.driver_data<asterix_state>();

	*flags = *code & 0x1000 ? TILE_FLIPX : 0;
	*color = (state->m_layer_colorbase[layer] + ((*code & 0xe000) >> 13)) & 0x7f;
	*code = (*code & 0x03ff) | state->m_tilebanks[(*code >> 10) & 3];
}

SCREEN_UPDATE_IND16( asterix )
{
	asterix_state *state = screen.machine().driver_data<asterix_state>();
	static const int K053251_CI[4] = { K053251_CI0, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layer[3], plane, new_colorbase;

	/* Layer offsets are different if horizontally flipped */
	if (k056832_read_register(state->m_k056832, 0x0) & 0x10)
	{
		k056832_set_layer_offs(state->m_k056832, 0, 89 - 176, 0);
		k056832_set_layer_offs(state->m_k056832, 1, 91 - 176, 0);
		k056832_set_layer_offs(state->m_k056832, 2, 89 - 176, 0);
		k056832_set_layer_offs(state->m_k056832, 3, 95 - 176, 0);
	}
	else
	{
		k056832_set_layer_offs(state->m_k056832, 0, 89, 0);
		k056832_set_layer_offs(state->m_k056832, 1, 91, 0);
		k056832_set_layer_offs(state->m_k056832, 2, 89, 0);
		k056832_set_layer_offs(state->m_k056832, 3, 95, 0);
	}


	state->m_tilebanks[0] = (k056832_get_lookup(state->m_k056832, 0) << 10);
	state->m_tilebanks[1] = (k056832_get_lookup(state->m_k056832, 1) << 10);
	state->m_tilebanks[2] = (k056832_get_lookup(state->m_k056832, 2) << 10);
	state->m_tilebanks[3] = (k056832_get_lookup(state->m_k056832, 3) << 10);

	// update color info and refresh tilemaps
	state->m_sprite_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI1);

	for (plane = 0; plane < 4; plane++)
	{
		new_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI[plane]);
		if (state->m_layer_colorbase[plane] != new_colorbase)
		{
			state->m_layer_colorbase[plane] = new_colorbase;
			k056832_mark_plane_dirty(state->m_k056832, plane);
		}
	}

	layer[0] = 0;
	state->m_layerpri[0] = k053251_get_priority(state->m_k053251, K053251_CI0);
	layer[1] = 1;
	state->m_layerpri[1] = k053251_get_priority(state->m_k053251, K053251_CI2);
	layer[2] = 3;
	state->m_layerpri[2] = k053251_get_priority(state->m_k053251, K053251_CI4);

	konami_sortlayers3(layer, state->m_layerpri);

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layer[0], K056832_DRAW_FLAG_MIRROR, 1);
	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layer[1], K056832_DRAW_FLAG_MIRROR, 2);
	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layer[2], K056832_DRAW_FLAG_MIRROR, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows in front of feet */
	k053245_sprites_draw(state->m_k053244, bitmap, cliprect);

	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, 2, K056832_DRAW_FLAG_MIRROR, 0);
	return 0;
}
