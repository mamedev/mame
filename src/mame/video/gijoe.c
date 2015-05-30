// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "includes/gijoe.h"

K053246_CB_MEMBER(gijoe_state::sprite_callback)
{
	int pri = (*color & 0x03e0) >> 4;

	if (pri <= m_layer_pri[3])
		*priority_mask = 0;
	else if (pri >  m_layer_pri[3] && pri <= m_layer_pri[2])
		*priority_mask = 0xff00;
	else if (pri >  m_layer_pri[2] && pri <= m_layer_pri[1])
		*priority_mask = 0xff00 | 0xf0f0;
	else if (pri >  m_layer_pri[1] && pri <= m_layer_pri[0])
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc;
	else
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa;

	*color = m_sprite_colorbase | (*color & 0x001f);
}

K056832_CB_MEMBER(gijoe_state::tile_callback)
{
	int tile = *code;

	if (tile >= 0xf000 && tile <= 0xf4ff)
	{
		tile &= 0x0fff;
		if (tile < 0x0310)
		{
			m_avac_occupancy[layer] |= 0x0f00;
			tile |= m_avac_bits[0];
		}
		else if (tile < 0x0470)
		{
			m_avac_occupancy[layer] |= 0xf000;
			tile |= m_avac_bits[1];
		}
		else
		{
			m_avac_occupancy[layer] |= 0x00f0;
			tile |= m_avac_bits[2];
		}
		*code = tile;
	}

	*color = (*color >> 2 & 0x0f) | m_layer_colorbase[layer];
}

void gijoe_state::video_start()
{
	int i;

	m_k056832->linemap_enable(1);

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
	save_item(NAME(m_avac_bits));   // these could possibly be re-created at postload k056832 elements
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layer_pri));
}

UINT32 gijoe_state::screen_update_gijoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layer[4];
	int vrc_mode, vrc_new, colorbase_new, /*primode,*/ dirty, i;
	int mask = 0;

	// update tile offsets
	m_k056832->read_avac(&vrc_mode, &vrc_new);

	if (vrc_mode)
	{
		for (dirty = 0xf000; dirty; dirty >>= 4)
			if ((m_avac_vrc & dirty) != (vrc_new & dirty))
				mask |= dirty;

		m_avac_vrc = vrc_new;
		m_avac_bits[0] = vrc_new << 4  & 0xf000;
		m_avac_bits[1] = vrc_new       & 0xf000;
		m_avac_bits[2] = vrc_new << 8  & 0xf000;
		m_avac_bits[3] = vrc_new << 12 & 0xf000;
	}
	else
		m_avac_bits[3] = m_avac_bits[2] = m_avac_bits[1] = m_avac_bits[0] = 0xf000;

	// update color info and refresh tilemaps
	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI0);

	for (i = 0; i < 4; i++)
	{
		dirty = 0;
		colorbase_new = m_k053251->get_palette_index(K053251_CI[i]);
		if (m_layer_colorbase[i] != colorbase_new)
		{
			m_layer_colorbase[i] = colorbase_new;
			dirty = 1;
		}
		if (m_avac_occupancy[i] & mask)
			dirty = 1;

		if (dirty)
		{
			m_avac_occupancy[i] = 0;
			m_k056832->mark_plane_dirty( i);
		}
	}

	/*
	    Layer A is supposed to be a non-scrolling status display with static X-offset.
	    The weird thing is tilemap alignment only follows the 832 standard when 2 is
	    written to the layer's X-scroll register otherwise the chip expects totally
	    different alignment values.
	*/
	if (m_k056832->read_register(0x14) == 2)
	{
		m_k056832->set_layer_offs(0,  2, 0);
		m_k056832->set_layer_offs(1,  4, 0);
		m_k056832->set_layer_offs(2,  6, 0); // 7?
		m_k056832->set_layer_offs(3,  8, 0);
	}
	else
	{
		m_k056832->set_layer_offs(0,  0, 0);
		m_k056832->set_layer_offs(1,  8, 0);
		m_k056832->set_layer_offs(2, 14, 0);
		m_k056832->set_layer_offs(3, 16, 0); // smaller?
	}

	// seems to switch the K053251 between different priority modes, detail unknown
	// primode = m_k053251->get_priority(K053251_CI1);

	layer[0] = 0;
	m_layer_pri[0] = 0; // not sure
	layer[1] = 1;
	m_layer_pri[1] = m_k053251->get_priority(K053251_CI2);
	layer[2] = 2;
	m_layer_pri[2] = m_k053251->get_priority(K053251_CI3);
	layer[3] = 3;
	m_layer_pri[3] = m_k053251->get_priority(K053251_CI4);

	konami_sortlayers4(layer, m_layer_pri);

	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

	m_k053246->k053247_sprites_draw( bitmap, cliprect);
	return 0;
}
