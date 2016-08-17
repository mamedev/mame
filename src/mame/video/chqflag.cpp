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
	enum { sprite_colorbase = 0 };

	*priority = (*color & 0x10) ? 0 : GFX_PMASK_1;
	*color = sprite_colorbase + (*color & 0x0f);
}

/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(chqflag_state::zoom_callback_1)
{
	enum { zoom_colorbase_1 = 256 / 16 };

	*code |= ((*color & 0x03) << 8);
	*color = zoom_colorbase_1 + ((*color & 0x3c) >> 2);
}

K051316_CB_MEMBER(chqflag_state::zoom_callback_2)
{
	enum { zoom_colorbase_2 = 512 / 256 };

	*flags = TILE_FLIPYX((*color & 0xc0) >> 6);
	*code |= ((*color & 0x0f) << 8);
	*color = zoom_colorbase_2 + ((*color & 0x10) >> 4);
}

/***************************************************************************

    Display Refresh

***************************************************************************/

UINT32 chqflag_state::screen_update_chqflag(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_k051316_2->zoom_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_k051316_2->zoom_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	m_k051316_1->zoom_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
