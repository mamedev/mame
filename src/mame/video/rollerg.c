#include "emu.h"
#include "video/konicdev.h"
#include "includes/rollerg.h"

/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

void rollerg_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	rollerg_state *state = machine.driver_data<rollerg_state>();
#if 0
	if (machine.input().code_pressed(KEYCODE_Q) && (*color & 0x80)) *color = rand();
	if (machine.input().code_pressed(KEYCODE_W) && (*color & 0x40)) *color = rand();
	if (machine.input().code_pressed(KEYCODE_E) && (*color & 0x20)) *color = rand();
	if (machine.input().code_pressed(KEYCODE_R) && (*color & 0x10)) *color = rand();
#endif
	*priority_mask = (*color & 0x10) ? 0 : 0x02;
	*color = state->m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void rollerg_zoom_callback( running_machine &machine, int *code, int *color, int *flags )
{
	rollerg_state *state = machine.driver_data<rollerg_state>();
	*flags = TILE_FLIPYX((*color & 0xc0) >> 6);
	*code |= ((*color & 0x0f) << 8);
	*color = state->m_zoom_colorbase + ((*color & 0x30) >> 4);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void rollerg_state::video_start()
{
	m_sprite_colorbase = 16;
	m_zoom_colorbase = 0;
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( rollerg )
{
	rollerg_state *state = screen.machine().driver_data<rollerg_state>();
	int bg_colorbase = 16;

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	k051316_zoom_draw(state->m_k051316, bitmap, cliprect, 0, 1);
	k053245_sprites_draw(state->m_k053244, bitmap, cliprect);
	return 0;
}
