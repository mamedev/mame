#include "emu.h"
#include "video/konicdev.h"
#include "includes/spy.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void spy_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	spy_state *state = machine.driver_data<spy_state>();
	*flags = (*color & 0x20) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void spy_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask, int *shadow )
{
	spy_state *state = machine.driver_data<spy_state>();

	/* bit 4 = priority over layer A (0 = have priority) */
	/* bit 5 = priority over layer B (1 = have priority) */
	*priority_mask = 0x00;
	if ( *color & 0x10) *priority_mask |= 0xa;
	if (~*color & 0x20) *priority_mask |= 0xc;

	*color = state->m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void spy_state::video_start()
{

	m_layer_colorbase[0] = 48;
	m_layer_colorbase[1] = 0;
	m_layer_colorbase[2] = 16;
	m_sprite_colorbase = 32;
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 spy_state::screen_update_spy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	k052109_tilemap_update(m_k052109);

	machine().priority_bitmap.fill(0, cliprect);

	if (!m_video_enable)
		bitmap.fill(16 * m_layer_colorbase[0], cliprect);
	else
	{
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 1);
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 2, 0, 2);
		k051960_sprites_draw(m_k051960, bitmap, cliprect, -1, -1);
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 0, 0, 0);
	}

	return 0;
}
