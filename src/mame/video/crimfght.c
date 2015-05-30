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
	*color = m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(crimfght_state::sprite_callback)
{
	/* Weird priority scheme. Why use three bits when two would suffice? */
	/* The PROM allows for mixed priorities, where sprites would have */
	/* priority over text but not on one or both of the other two planes. */
	/* Luckily, this isn't used by the game. */
	switch (*color & 0x70)
	{
		case 0x10: *priority = 0; break;
		case 0x00: *priority = 1; break;
		case 0x40: *priority = 2; break;
		case 0x20: *priority = 3; break;
		/*   0x60 == 0x20 */
		/*   0x50 priority over F and A, but not over B */
		/*   0x30 priority over F, but not over A and B */
		/*   0x70 == 0x30 */
	}
	/* bit 7 is on in the "Game Over" sprites, meaning unknown */
	/* in Aliens it is the top bit of the code, but that's not needed here */
	*color = m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void crimfght_state::video_start()
{
	m_paletteram.resize(0x400);
	m_palette->basemem().set(m_paletteram, ENDIANNESS_BIG, 2);

	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 4;
	m_layer_colorbase[2] = 8;
	m_sprite_colorbase = 16;

	save_item(NAME(m_paletteram));
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 crimfght_state::screen_update_crimfght(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 0);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 2, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 1, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	return 0;
}
