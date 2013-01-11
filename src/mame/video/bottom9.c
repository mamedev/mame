#include "emu.h"
#include "video/konicdev.h"
#include "includes/bottom9.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void bottom9_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	bottom9_state *state = machine.driver_data<bottom9_state>();
	*code |= (*color & 0x3f) << 8;
	*color = state->m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void bottom9_sprite_callback( running_machine &machine, int *code, int *color, int *priority, int *shadow )
{
	/* bit 4 = priority over zoom (0 = have priority) */
	/* bit 5 = priority over B (1 = have priority) */
	bottom9_state *state = machine.driver_data<bottom9_state>();
	*priority = (*color & 0x30) >> 4;
	*color = state->m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void bottom9_zoom_callback( running_machine &machine, int *code, int *color, int *flags )
{
	bottom9_state *state = machine.driver_data<bottom9_state>();
	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8);
	*color = state->m_zoom_colorbase + ((*color & 0x3c) >> 2);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void bottom9_state::video_start()
{

	m_layer_colorbase[0] = 0;   /* not used */
	m_layer_colorbase[1] = 0;
	m_layer_colorbase[2] = 16;
	m_sprite_colorbase = 32;
	m_zoom_colorbase = 48;
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 bottom9_state::screen_update_bottom9(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	k052109_tilemap_update(m_k052109);

	/* note: FIX layer is not used */
	bitmap.fill(m_layer_colorbase[1], cliprect);
//  if (m_video_enable)
	{
		k051960_sprites_draw(m_k051960, bitmap, cliprect, 1, 1);
		k051316_zoom_draw(m_k051316, bitmap, cliprect, 0, 0);
		k051960_sprites_draw(m_k051960, bitmap, cliprect, 0, 0);
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 2, 0, 0);
		/* note that priority 3 is opposite to the basic layer priority! */
		/* (it IS used, but hopefully has no effect) */
		k051960_sprites_draw(m_k051960, bitmap, cliprect, 2, 3);
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 1, 0, 0);
	}
	return 0;
}
