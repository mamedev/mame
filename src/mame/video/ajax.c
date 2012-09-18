/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/ajax.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void ajax_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	ajax_state *state = machine.driver_data<ajax_state>();
	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void ajax_sprite_callback( running_machine &machine, int *code, int *color, int *priority, int *shadow )
{
	/* priority bits:
       4 over zoom (0 = have priority)
       5 over B    (0 = have priority)
       6 over A    (1 = have priority)
       never over F
    */
	ajax_state *state = machine.driver_data<ajax_state>();
	*priority = 0xff00;							/* F = 8 */
	if ( *color & 0x10) *priority |= 0xf0f0;	/* Z = 4 */
	if (~*color & 0x40) *priority |= 0xcccc;	/* A = 2 */
	if ( *color & 0x20) *priority |= 0xaaaa;	/* B = 1 */
	*color = state->m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void ajax_zoom_callback( running_machine &machine, int *code, int *color, int *flags )
{
	ajax_state *state = machine.driver_data<ajax_state>();
	*code |= ((*color & 0x07) << 8);
	*color = state->m_zoom_colorbase + ((*color & 0x08) >> 3);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void ajax_state::video_start()
{

	m_layer_colorbase[0] = 64;
	m_layer_colorbase[1] = 0;
	m_layer_colorbase[2] = 32;
	m_sprite_colorbase = 16;
	m_zoom_colorbase = 6;	/* == 48 since it's 7-bit graphics */
}



/***************************************************************************

    Display Refresh

***************************************************************************/

UINT32 ajax_state::screen_update_ajax(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	k052109_tilemap_update(m_k052109);

	machine().priority_bitmap.fill(0, cliprect);

	bitmap.fill(get_black_pen(machine()), cliprect);
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, 2, 0, 1);
	if (m_priority)
	{
		/* basic layer order is B, zoom, A, F */
		k051316_zoom_draw(m_k051316, bitmap, cliprect, 0, 4);
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 1, 0, 2);
	}
	else
	{
		/* basic layer order is B, A, zoom, F */
		k052109_tilemap_draw(m_k052109, bitmap, cliprect, 1, 0, 2);
		k051316_zoom_draw(m_k051316, bitmap, cliprect, 0, 4);
	}
	k052109_tilemap_draw(m_k052109, bitmap, cliprect, 0, 0, 8);

	k051960_sprites_draw(m_k051960, bitmap, cliprect, -1, -1);
	return 0;
}
