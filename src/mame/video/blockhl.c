#include "emu.h"
#include "video/konicdev.h"
#include "includes/blockhl.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void blockhl_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	blockhl_state *state = machine.driver_data<blockhl_state>();
	*code |= ((*color & 0x0f) << 8);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void blockhl_sprite_callback( running_machine &machine, int *code, int *color, int *priority, int *shadow )
{
	blockhl_state *state = machine.driver_data<blockhl_state>();

	if(*color & 0x10)
		*priority = 0xfe; // under K052109_tilemap[0]
	else
		*priority = 0xfc; // under K052109_tilemap[1]

	*color = state->m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void blockhl_state::video_start()
{

	m_generic_paletteram_8.allocate(0x800);

	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 16;
	m_layer_colorbase[2] = 32;
	m_sprite_colorbase = 48;
}

UINT32 blockhl_state::screen_update_blockhl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	machine().priority_bitmap.fill(0, cliprect);

	k052109_tilemap_update(m_k052109);

	k052109_tilemap_draw(m_k052109, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);   // tile 2
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, 1, 0, 1); // tile 1
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, 0, 0, 2); // tile 0

	k051960_sprites_draw(m_k051960, bitmap, cliprect, 0, -1);
	return 0;
}
