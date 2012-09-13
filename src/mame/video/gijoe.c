#include "emu.h"
#include "video/konicdev.h"
#include "includes/gijoe.h"

void gijoe_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	gijoe_state *state = machine.driver_data<gijoe_state>();
	int pri = (*color & 0x03e0) >> 4;

	if (pri <= state->m_layer_pri[3])
		*priority_mask = 0;
	else if (pri >  state->m_layer_pri[3] && pri <= state->m_layer_pri[2])
		*priority_mask = 0xff00;
	else if (pri >  state->m_layer_pri[2] && pri <= state->m_layer_pri[1])
		*priority_mask = 0xff00 | 0xf0f0;
	else if (pri >  state->m_layer_pri[1] && pri <= state->m_layer_pri[0])
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc;
	else
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa;

	*color = state->m_sprite_colorbase | (*color & 0x001f);
}

void gijoe_tile_callback( running_machine &machine, int layer, int *code, int *color, int *flags )
{
	gijoe_state *state = machine.driver_data<gijoe_state>();
	int tile = *code;

	if (tile >= 0xf000 && tile <= 0xf4ff)
	{
		tile &= 0x0fff;
		if (tile < 0x0310)
		{
			state->m_avac_occupancy[layer] |= 0x0f00;
			tile |= state->m_avac_bits[0];
		}
		else if (tile < 0x0470)
		{
			state->m_avac_occupancy[layer] |= 0xf000;
			tile |= state->m_avac_bits[1];
		}
		else
		{
			state->m_avac_occupancy[layer] |= 0x00f0;
			tile |= state->m_avac_bits[2];
		}
		*code = tile;
	}

	*color = (*color >> 2 & 0x0f) | state->m_layer_colorbase[layer];
}

void gijoe_state::video_start()
{
	int i;

	k056832_linemap_enable(m_k056832, 1);

	for (i = 0; i < 4; i++)
	{
		m_avac_occupancy[i] = 0;
		m_avac_bits[i] = 0;
		m_layer_colorbase[i] = 0;
		m_layer_pri[i] = 0;
	}

	m_avac_vrc = 0xffff;

	save_item(NAME(m_avac_vrc));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_avac_occupancy));
	save_item(NAME(m_avac_bits));	// these could possibly be re-created at postload k056832 elements
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layer_pri));
}

SCREEN_UPDATE_IND16( gijoe )
{
	gijoe_state *state = screen.machine().driver_data<gijoe_state>();
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layer[4];
	int vrc_mode, vrc_new, colorbase_new, /*primode,*/ dirty, i;
	int mask = 0;

	// update tile offsets
	k056832_read_avac(state->m_k056832, &vrc_mode, &vrc_new);

	if (vrc_mode)
	{
		for (dirty = 0xf000; dirty; dirty >>= 4)
			if ((state->m_avac_vrc & dirty) != (vrc_new & dirty))
				mask |= dirty;

		state->m_avac_vrc = vrc_new;
		state->m_avac_bits[0] = vrc_new << 4  & 0xf000;
		state->m_avac_bits[1] = vrc_new       & 0xf000;
		state->m_avac_bits[2] = vrc_new << 8  & 0xf000;
		state->m_avac_bits[3] = vrc_new << 12 & 0xf000;
	}
	else
		state->m_avac_bits[3] = state->m_avac_bits[2] = state->m_avac_bits[1] = state->m_avac_bits[0] = 0xf000;

	// update color info and refresh tilemaps
	state->m_sprite_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI0);

	for (i = 0; i < 4; i++)
	{
		dirty = 0;
		colorbase_new = k053251_get_palette_index(state->m_k053251, K053251_CI[i]);
		if (state->m_layer_colorbase[i] != colorbase_new)
		{
			state->m_layer_colorbase[i] = colorbase_new;
			dirty = 1;
		}
		if (state->m_avac_occupancy[i] & mask)
			dirty = 1;

		if (dirty)
		{
			state->m_avac_occupancy[i] = 0;
			k056832_mark_plane_dirty(state->m_k056832, i);
		}
	}

	/*
        Layer A is supposed to be a non-scrolling status display with static X-offset.
        The weird thing is tilemap alignment only follows the 832 standard when 2 is
        written to the layer's X-scroll register otherwise the chip expects totally
        different alignment values.
    */
	if (k056832_read_register(state->m_k056832, 0x14) == 2)
	{
		k056832_set_layer_offs(state->m_k056832, 0,  2, 0);
		k056832_set_layer_offs(state->m_k056832, 1,  4, 0);
		k056832_set_layer_offs(state->m_k056832, 2,  6, 0); // 7?
		k056832_set_layer_offs(state->m_k056832, 3,  8, 0);
	}
	else
	{
		k056832_set_layer_offs(state->m_k056832, 0,  0, 0);
		k056832_set_layer_offs(state->m_k056832, 1,  8, 0);
		k056832_set_layer_offs(state->m_k056832, 2, 14, 0);
		k056832_set_layer_offs(state->m_k056832, 3, 16, 0); // smaller?
	}

	// seems to switch the K053251 between different priority modes, detail unknown
	// primode = k053251_get_priority(state->m_k053251, K053251_CI1);

	layer[0] = 0;
	state->m_layer_pri[0] = 0; // not sure
	layer[1] = 1;
	state->m_layer_pri[1] = k053251_get_priority(state->m_k053251, K053251_CI2);
	layer[2] = 2;
	state->m_layer_pri[2] = k053251_get_priority(state->m_k053251, K053251_CI3);
	layer[3] = 3;
	state->m_layer_pri[3] = k053251_get_priority(state->m_k053251, K053251_CI4);

	konami_sortlayers4(layer, state->m_layer_pri);

	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	screen.machine().priority_bitmap.fill(0, cliprect);

	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layer[0], 0, 1);
	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layer[1], 0, 2);
	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layer[2], 0, 4);
	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layer[3], 0, 8);

	k053247_sprites_draw(state->m_k053246, bitmap, cliprect);
	return 0;
}
