// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Manuel Abadia
#include "emu.h"
#include "includes/thunderx.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static const int layer_colorbase[] = { 768 / 16, 0 / 16, 256 / 16 };

K052109_CB_MEMBER(thunderx_state::tile_callback)
{
	*code |= ((*color & 0x1f) << 8) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

K052109_CB_MEMBER(thunderx_state::gbusters_tile_callback)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x0d) << 8) | ((*color & 0x10) << 5) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(thunderx_state::sprite_callback)
{
	enum { sprite_colorbase = 512 / 16 };

	/* Sprite priority 1 means appear behind background, used only to mask sprites */
	/* in the foreground */
	/* Sprite priority 3 means don't draw (not used) */
	switch (*color & 0x30)
	{
		case 0x00: *priority = 0; break;
		case 0x10: *priority = GFX_PMASK_2 | GFX_PMASK_1; break;
		case 0x20: *priority = GFX_PMASK_2; break;
		case 0x30: *priority = 0xffff; break;
	}

	*color = sprite_colorbase + (*color & 0x0f);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void thunderx_state::video_start()
{
	m_palette->set_shadow_factor(7.0/8.0);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 thunderx_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);

	/* The background color is always from layer 1 */
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 0);

	int bg = m_priority ? 2 : 1;
	int fg = m_priority ? 1 : 2;

	m_k052109->tilemap_draw(screen, bitmap, cliprect, bg, 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, fg, 0, 2);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	return 0;
}
