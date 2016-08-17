// license:BSD-3-Clause
// copyright-holders:R. Belmont, Nicola Salmoria
/***************************************************************************

    Lethal Enforcers
     (c) 1992 Konami

    Video hardware emulation.

***************************************************************************/

#include "emu.h"
#include "includes/lethal.h"

K05324X_CB_MEMBER(lethal_state::sprite_callback)
{
	int pri = (*color & 0xfff0);
	*color = *color & 0x000f;
	*color += m_sprite_colorbase;

	/* this isn't ideal.. shouldn't need to hardcode it? not 100% sure about it anyway*/
	if (pri == 0x10)
		*priority = 0xf0; // guys on first level
	else if (pri == 0x90)
		*priority = 0xf0; // car doors
	else if (pri == 0x20)
		*priority = 0xf0 | 0xcc; // people behind glass on 1st level
	else if (pri == 0xa0)
		*priority = 0xf0 | 0xcc; // glass on 1st/2nd level
	else if (pri == 0x40)
		*priority = 0; // blood splats?
	else if (pri == 0x00)
		*priority = 0; // gunshots etc
	else if (pri == 0x30)
		*priority = 0xf0 | 0xcc | 0xaa; // mask sprites (always in a bad colour, used to do special effects i think
	else
	{
		popmessage("unknown pri %04x\n", pri);
		*priority = 0;
	}

	*code = (*code & 0x3fff); // | spritebanks[(*code >> 12) & 3];
}

K056832_CB_MEMBER(lethal_state::tile_callback)
{
	*color = m_layer_colorbase[layer] + ((*color & 0x3c) << 2);
}

void lethal_state::video_start()
{
	// this game uses external linescroll RAM
	m_k056832->SetExtLinescroll();

	// the US and Japanese cabinets apparently use different mirror setups
	if (!strcmp(machine().system().name, "lethalenj"))
	{
		m_k056832->set_layer_offs(0, -195, 0);
		m_k056832->set_layer_offs(1, -193, 0);
		m_k056832->set_layer_offs(2, -191, 0);
		m_k056832->set_layer_offs(3, -189, 0);
	}
	else
	{
		m_k056832->set_layer_offs(0, 188, 0);
		m_k056832->set_layer_offs(1, 190, 0);
		m_k056832->set_layer_offs(2, 192, 0);
		m_k056832->set_layer_offs(3, 194, 0);
	}
}

WRITE8_MEMBER(lethal_state::lethalen_palette_control)
{
	switch (offset)
	{
		case 0: // 40c8 - PCU1 from schematics
			m_layer_colorbase[0] = (data & 0x7) * 1024 / 16;
			m_layer_colorbase[1] = ((data >> 4) & 0x7) * 1024 / 16;
			m_k056832->mark_plane_dirty( 0);
			m_k056832->mark_plane_dirty( 1);
			break;

		case 4: // 40cc - PCU2 from schematics
			m_layer_colorbase[2] = (data & 0x7) * 1024 / 16;
			m_layer_colorbase[3] = ((data >> 4) & 0x7) * 1024 / 16;
			m_k056832->mark_plane_dirty( 2);
			m_k056832->mark_plane_dirty( 3);
			break;

		case 8: // 40d0 - PCU3 from schematics
			m_sprite_colorbase = (data & 0x7) * 1024 / 64;
			m_back_colorbase = ((data >> 4) & 0x7) * 1024 + 1023;
			break;
	}
}

UINT32 lethal_state::screen_update_lethalen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_back_colorbase, cliprect);
	screen.priority().fill(0, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, K056832_DRAW_FLAG_MIRROR, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, K056832_DRAW_FLAG_MIRROR, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, K056832_DRAW_FLAG_MIRROR, 4);

	m_k053244->sprites_draw(bitmap, cliprect, screen.priority());

	// force "A" layer over top of everything
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, K056832_DRAW_FLAG_MIRROR, 0);

	return 0;
}
