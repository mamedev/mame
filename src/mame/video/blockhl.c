// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/blockhl.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(blockhl_state::tile_callback)
{
	*code |= ((*color & 0x0f) << 8);
	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(blockhl_state::sprite_callback)
{
	if(*color & 0x10)
		*priority = 0xfe; // under K052109_tilemap[0]
	else
		*priority = 0xfc; // under K052109_tilemap[1]

	*color = m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void blockhl_state::video_start()
{
	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 16;
	m_layer_colorbase[2] = 32;
	m_sprite_colorbase = 48;
}

UINT32 blockhl_state::screen_update_blockhl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_k052109->tilemap_update();

	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);   // tile 2
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 1); // tile 1
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 2); // tile 0

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, -1);
	return 0;
}
