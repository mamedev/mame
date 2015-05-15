// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Manuel Abadia
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/chqflag.h"


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(chqflag_state::sprite_callback)
{
	*priority = (*color & 0x10) >> 4;
	*color = m_sprite_colorbase + (*color & 0x0f);
}

/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(chqflag_state::zoom_callback_1)
{
	*code |= ((*color & 0x03) << 8);
	*color = m_zoom_colorbase[0] + ((*color & 0x3c) >> 2);
}

K051316_CB_MEMBER(chqflag_state::zoom_callback_2)
{
	*flags = TILE_FLIPYX((*color & 0xc0) >> 6);
	*code |= ((*color & 0x0f) << 8);
	*color = m_zoom_colorbase[1] + ((*color & 0x10) >> 4);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void chqflag_state::video_start()
{
	m_sprite_colorbase = 0;
	m_zoom_colorbase[0] = 0x10;
	m_zoom_colorbase[1] = 0x02;
}

/***************************************************************************

    Display Refresh

***************************************************************************/

UINT32 chqflag_state::screen_update_chqflag(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_k051316_2->zoom_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	m_k051316_2->zoom_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 1, 1);
	m_k051316_1->zoom_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
