// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/88games.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(_88games_state::tile_callback)
{
	static const int layer_colorbase[] = { 1024 / 16, 0 / 16, 256 / 16 };

	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(_88games_state::sprite_callback)
{
	enum { sprite_colorbase = 512 / 16 };

	*priority = (*color & 0x20) >> 5;   /* ??? */
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(_88games_state::zoom_callback)
{
	enum { zoom_colorbase = 768 / 16 };

	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x07) << 8);
	*color = zoom_colorbase + ((*color & 0x38) >> 3) + ((*color & 0x80) >> 4);
}

/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 _88games_state::screen_update_88games(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	if (m_k88games_priority)
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);   // tile 0
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 1, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0); // tile 2
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0); // tile 1
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);   // tile 2
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 0);
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0); // tile 1
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 1, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0); // tile 0
	}

	return 0;
}
