#include "emu.h"
#include "video/konicdev.h"
#include "includes/thunderx.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void thunderx_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	thunderx_state *state = machine.driver_data<thunderx_state>();
	*code |= ((*color & 0x1f) << 8) | (bank << 13);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void thunderx_sprite_callback( running_machine &machine, int *code,int *color, int *priority_mask, int *shadow )
{
	thunderx_state *state = machine.driver_data<thunderx_state>();

	/* Sprite priority 1 means appear behind background, used only to mask sprites */
	/* in the foreground */
	/* Sprite priority 3 means don't draw (not used) */
	switch (*color & 0x30)
	{
		case 0x00: *priority_mask = 0xf0; break;
		case 0x10: *priority_mask = 0xf0 | 0xcc | 0xaa; break;
		case 0x20: *priority_mask = 0xf0 | 0xcc; break;
		case 0x30: *priority_mask = 0xffff; break;
	}

	*color = state->m_sprite_colorbase + (*color & 0x0f);
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

	palette_set_shadow_factor(machine(),7.0/8.0);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 thunderx_state::screen_update_scontra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	k052109_tilemap_update(m_k052109);

	machine().priority_bitmap.fill(0, cliprect);

	/* The background color is always from layer 1 - but it's always black anyway */
//  bitmap.fill(16 * m_layer_colorbase[1], cliprect);
	if (m_priority)
	{
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 1);
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 1, 0, 2);
	}
	else
	{
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 1);
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 2, 0, 2);
	}
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, 0, 0, 4);

	k051960_sprites_draw(m_k051960, bitmap, cliprect, -1, -1);
	return 0;
}
