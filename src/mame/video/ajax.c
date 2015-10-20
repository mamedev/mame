// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/ajax.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(ajax_state::tile_callback)
{
	static const int layer_colorbase[] = { 1024 / 16, 0 / 16, 512 / 16 };

	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(ajax_state::sprite_callback)
{
	enum { sprite_colorbase = 256 / 16 };

	/* priority bits:
	   4 over zoom (0 = have priority)
	   5 over B    (0 = have priority)
	   6 over A    (1 = have priority)
	   never over F
	*/
	*priority = 0;
	if ( *color & 0x10) *priority |= GFX_PMASK_4; /* Z = 4 */
	if (~*color & 0x40) *priority |= GFX_PMASK_2; /* A = 2 */
	if ( *color & 0x20) *priority |= GFX_PMASK_1; /* B = 1 */
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(ajax_state::zoom_callback)
{
	enum { zoom_colorbase = 768 / 128 };

	*code |= ((*color & 0x07) << 8);
	*color = zoom_colorbase + ((*color & 0x08) >> 3);
}


/***************************************************************************

    Display Refresh

***************************************************************************/

UINT32 ajax_state::screen_update_ajax(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 1);
	if (m_priority)
	{
		/* basic layer order is B, zoom, A, F */
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 4);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 2);
	}
	else
	{
		/* basic layer order is B, A, zoom, F */
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 2);
		m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 4);
	}
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	return 0;
}
