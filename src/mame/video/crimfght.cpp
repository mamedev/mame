// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "emu.h"
#include "includes/crimfght.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(crimfght_state::tile_callback)
{
	*flags = (*color & 0x20) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x1f) << 8) | (bank << 13);
	*color = layer * 4 + ((*color & 0xc0) >> 6);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(crimfght_state::sprite_callback)
{
	enum { sprite_colorbase = 256 / 16 };

	/* The PROM allows for mixed priorities, where sprites would have */
	/* priority over text but not on one or both of the other two planes. */
	switch (*color & 0x70)
	{
		case 0x10: *priority = 0x00; break;            /* over ABF */
		case 0x00: *priority = GFX_PMASK_4          ; break;  /* over AB, not F */
		case 0x40: *priority = GFX_PMASK_4|GFX_PMASK_2     ; break;  /* over A, not BF */
		case 0x20:
		case 0x60: *priority = GFX_PMASK_4|GFX_PMASK_2|GFX_PMASK_1; break;  /* over -, not ABF */
		case 0x50: *priority = GFX_PMASK_2     ; break;  /* over AF, not B */
		case 0x30:
		case 0x70: *priority = GFX_PMASK_2|GFX_PMASK_1; break;  /* over F, not AB */
	}
	/* bit 7 is on in the "Game Over" sprites, meaning unknown */
	/* in Aliens it is the top bit of the code, but that's not needed here */
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 crimfght_state::screen_update_crimfght(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);
	/* The background color is always from layer 1 */
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 0);

	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 4);

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}
