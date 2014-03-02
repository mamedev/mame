#include "emu.h"

#include "includes/aliens.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void aliens_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	aliens_state *state = machine.driver_data<aliens_state>();

	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void aliens_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask, int *shadow )
{
	aliens_state *state = machine.driver_data<aliens_state>();

	/* The PROM allows for mixed priorities, where sprites would have */
	/* priority over text but not on one or both of the other two planes. */
	switch (*color & 0x70)
	{
		case 0x10: *priority_mask = 0x00; break;            /* over ABF */
		case 0x00: *priority_mask = 0xf0          ; break;  /* over AB, not F */
		case 0x40: *priority_mask = 0xf0|0xcc     ; break;  /* over A, not BF */
		case 0x20:
		case 0x60: *priority_mask = 0xf0|0xcc|0xaa; break;  /* over -, not ABF */
		case 0x50: *priority_mask =      0xcc     ; break;  /* over AF, not B */
		case 0x30:
		case 0x70: *priority_mask =      0xcc|0xaa; break;  /* over F, not AB */
	}
	*code |= (*color & 0x80) << 6;
	*color = state->m_sprite_colorbase + (*color & 0x0f);
	*shadow = 0;    /* shadows are not used by this game */
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void aliens_state::video_start()
{
	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 4;
	m_layer_colorbase[2] = 8;
	m_sprite_colorbase = 16;
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 aliens_state::screen_update_aliens(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_layer_colorbase[1] * 16, cliprect);

	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 4);

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}
