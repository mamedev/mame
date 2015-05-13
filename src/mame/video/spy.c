// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/spy.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(spy_state::tile_callback)
{
	*flags = (*color & 0x20) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(spy_state::sprite_callback)
{
	/* bit 4 = priority over layer A (0 = have priority) */
	/* bit 5 = priority over layer B (1 = have priority) */
	*priority = 0x00;
	if ( *color & 0x10) *priority |= 0xa;
	if (~*color & 0x20) *priority |= 0xc;

	*color = m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void spy_state::video_start()
{
	m_layer_colorbase[0] = 48;
	m_layer_colorbase[1] = 0;
	m_layer_colorbase[2] = 16;
	m_sprite_colorbase = 32;
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 spy_state::screen_update_spy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);

	if (!m_video_enable)
		bitmap.fill(16 * m_layer_colorbase[0], cliprect);
	else
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	}

	return 0;
}
