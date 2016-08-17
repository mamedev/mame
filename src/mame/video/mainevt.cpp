// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/mainevt.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(mainevt_state::mainevt_tile_callback)
{
	static const int layer_colorbase[] = { 0 / 16, 128 / 16, 64 / 16 };

	*flags = (*color & 0x02) ? TILE_FLIPX : 0;

	/* priority relative to HALF priority sprites */
	*priority = (layer == 2) ? (*color & 0x20) >> 5 : 0;
	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

K052109_CB_MEMBER(mainevt_state::dv_tile_callback)
{
	static const int layer_colorbase[] = { 0 / 16, 0 / 16, 64 / 16 };

	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x3c) << 7);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(mainevt_state::mainevt_sprite_callback)
{
	enum { sprite_colorbase = 192 / 16 };

	/* bit 5 = priority over layer B (has precedence) */
	/* bit 6 = HALF priority over layer B (used for crowd when you get out of the ring) */
	if (*color & 0x20)
		*priority = 0xff00;
	else if (*color & 0x40)
		*priority = 0xff00 | 0xf0f0;
	else
		*priority = 0xff00 | 0xf0f0 | 0xcccc;
	/* bit 7 is shadow, not used */

	*color = sprite_colorbase + (*color & 0x03);
}

K051960_CB_MEMBER(mainevt_state::dv_sprite_callback)
{
	enum { sprite_colorbase = 128 / 16 };

	/* TODO: the priority/shadow handling (bits 5-7) seems to be quite complex (see PROM) */
	*color = sprite_colorbase + (*color & 0x07);
}


/*****************************************************************************/

UINT32 mainevt_state::screen_update_mainevt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 1, 2); /* low priority part of layer */
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 4); /* high priority part of layer */
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 8);

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}

UINT32 mainevt_state::screen_update_dv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 0);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	return 0;
}
