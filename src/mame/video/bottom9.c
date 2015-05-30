// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/bottom9.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(bottom9_state::tile_callback)
{
	*code |= (*color & 0x3f) << 8;
	*color = m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(bottom9_state::sprite_callback)
{
	/* bit 4 = priority over zoom (0 = have priority) */
	/* bit 5 = priority over B (1 = have priority) */
	*priority = (*color & 0x30) >> 4;
	*color = m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(bottom9_state::zoom_callback)
{
	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8);
	*color = m_zoom_colorbase + ((*color & 0x3c) >> 2);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void bottom9_state::video_start()
{
	m_layer_colorbase[0] = 0;   /* not used */
	m_layer_colorbase[1] = 0;
	m_layer_colorbase[2] = 16;
	m_sprite_colorbase = 32;
	m_zoom_colorbase = 48;
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 bottom9_state::screen_update_bottom9(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	/* note: FIX layer is not used */
	bitmap.fill(m_layer_colorbase[1], cliprect);
//  if (m_video_enable)
	{
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 1, 1);
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 0);
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);
		/* note that priority 3 is opposite to the basic layer priority! */
		/* (it IS used, but hopefully has no effect) */
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 2, 3);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	}
	return 0;
}
