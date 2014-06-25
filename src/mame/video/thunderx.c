#include "emu.h"
#include "includes/thunderx.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(thunderx_state::tile_callback)
{
	*code |= ((*color & 0x1f) << 8) | (bank << 13);
	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(thunderx_state::sprite_callback)
{
	/* Sprite priority 1 means appear behind background, used only to mask sprites */
	/* in the foreground */
	/* Sprite priority 3 means don't draw (not used) */
	switch (*color & 0x30)
	{
		case 0x00: *priority = 0xf0; break;
		case 0x10: *priority = 0xf0 | 0xcc | 0xaa; break;
		case 0x20: *priority = 0xf0 | 0xcc; break;
		case 0x30: *priority = 0xffff; break;
	}

	*color = m_sprite_colorbase + (*color & 0x0f);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void thunderx_state::video_start()
{
	m_layer_colorbase[0] = 48;
	m_layer_colorbase[1] = 0;
	m_layer_colorbase[2] = 16;
	m_sprite_colorbase = 32;

	m_palette->set_shadow_factor(7.0/8.0);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 thunderx_state::screen_update_scontra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);

	/* The background color is always from layer 1 - but it's always black anyway */
//  bitmap.fill(16 * m_layer_colorbase[1], cliprect);
	if (m_priority)
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 2);
	}
	else
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
	}
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 4);

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}
