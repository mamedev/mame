// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/***************************************************************************

Based on taito_h.cpp

***************************************************************************/

#include "emu.h"
#include "includes/taito_o.h"


void taitoo_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	for (int offs = 0x03f8 / 2; offs >= 0; offs -= 0x008 / 2)
	{
		if (offs <  0x01b0 && priority == 0)    continue;
		if (offs >= 0x01b0 && priority == 1)    continue;

		m_tc0080vco->get_sprite_params(offs, true);

		if (m_tc0080vco->get_sprite_tile_offs())
		{
			m_tc0080vco->draw_single_sprite(bitmap, cliprect);
		}
	}
}


u32 taitoo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tc0080vco->tilemap_update();

	bitmap.fill(0, cliprect);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);

	draw_sprites(bitmap, cliprect, 0);
	draw_sprites(bitmap, cliprect, 1);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);

	return 0;
}
