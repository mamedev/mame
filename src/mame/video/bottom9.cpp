// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/bottom9.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static const int layer_colorbase[] = { 0 / 16, 0 / 16, 256 / 16 };

K052109_CB_MEMBER(bottom9_state::tile_callback)
{
	*code |= (*color & 0x3f) << 8;
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(bottom9_state::sprite_callback)
{
	enum { sprite_colorbase = 512 / 16 };

	/* bit 4 = priority over zoom (0 = have priority) */
	/* bit 5 = priority over B (1 = have priority) */
	*priority = 0;
	if ( *color & 0x10) *priority |= GFX_PMASK_1;
	if (~*color & 0x20) *priority |= GFX_PMASK_2;

	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(bottom9_state::zoom_callback)
{
	enum { zoom_colorbase = 768 / 16 };

	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8);
	*color = zoom_colorbase + ((*color & 0x3c) >> 2);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 bottom9_state::screen_update_bottom9(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	/* note: FIX layer is not used */
	bitmap.fill(layer_colorbase[1], cliprect);
	screen.priority().fill(0, cliprect);

//  if (m_video_enable)
	{
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	}
	return 0;
}
