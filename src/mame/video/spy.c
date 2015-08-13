// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/spy.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(spy_state::tile_callback)
{
	static const int layer_colorbase[] = { 768 / 16, 0 / 16, 256 / 16 };

	*flags = (*color & 0x20) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(spy_state::sprite_callback)
{
	enum { sprite_colorbase = 512 / 16 };

	/* bit 4 = priority over layer A (0 = have priority) */
	/* bit 5 = priority over layer B (1 = have priority) */
	*priority = 0x00;
	if ( *color & 0x10) *priority |= GFX_PMASK_1;
	if (~*color & 0x20) *priority |= GFX_PMASK_2;

	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 spy_state::screen_update_spy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);

	if (!m_video_enable)
		bitmap.fill(768, cliprect); // ?
	else
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	}

	return 0;
}
