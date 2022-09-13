// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "simpsons.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(simpsons_state::tile_callback)
{
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

K053246_CB_MEMBER(simpsons_state::sprite_callback)
{
	int pri = (*color & 0x0f80) >> 6;   /* ??????? */

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

  Extra video banking

***************************************************************************/

uint8_t simpsons_state::k052109_r(offs_t offset)
{
	return m_k052109->read(offset + 0x2000);
}

void simpsons_state::k052109_w(offs_t offset, uint8_t data)
{
	m_k052109->write(offset + 0x2000, data);
}

uint8_t simpsons_state::k053247_r(offs_t offset)
{
	int offs = offset >> 1;

	if (offset & 1)
		return(m_spriteram[offs] & 0xff);
	else
		return(m_spriteram[offs] >> 8);
}

void simpsons_state::k053247_w(offs_t offset, uint8_t data)
{
	int offs = offset >> 1;

	if (offset & 1)
		m_spriteram[offs] = (m_spriteram[offs] & 0xff00) | data;
	else
		m_spriteram[offs] = (m_spriteram[offs] & 0x00ff) | (data << 8);
}

void simpsons_state::video_bank_select( int bank )
{
	if(bank & 1)
		m_palette_view.select(0);
	else
		m_palette_view.disable();
	m_video_view.select((bank >> 1) & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t simpsons_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[3], bg_colorbase;

	bg_colorbase = m_k053251->get_palette_index(k053251_device::CI0);
	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI1);
	m_layer_colorbase[0] = m_k053251->get_palette_index(k053251_device::CI2);
	m_layer_colorbase[1] = m_k053251->get_palette_index(k053251_device::CI3);
	m_layer_colorbase[2] = m_k053251->get_palette_index(k053251_device::CI4);

	m_k052109->tilemap_update();

	layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(k053251_device::CI2);
	layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(k053251_device::CI3);
	layer[2] = 2;
	m_layerpri[2] = m_k053251->get_priority(k053251_device::CI4);

	konami_sortlayers3(layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	m_k053246->k053247_sprites_draw(bitmap, cliprect);
	return 0;
}
