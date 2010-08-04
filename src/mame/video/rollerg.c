#include "emu.h"
#include "video/konicdev.h"
#include "includes/rollerg.h"

/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

void rollerg_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	rollerg_state *state = machine->driver_data<rollerg_state>();
#if 0
	if (input_code_pressed(machine, KEYCODE_Q) && (*color & 0x80)) *color = rand();
	if (input_code_pressed(machine, KEYCODE_W) && (*color & 0x40)) *color = rand();
	if (input_code_pressed(machine, KEYCODE_E) && (*color & 0x20)) *color = rand();
	if (input_code_pressed(machine, KEYCODE_R) && (*color & 0x10)) *color = rand();
#endif
	*priority_mask = (*color & 0x10) ? 0 : 0x02;
	*color = state->sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void rollerg_zoom_callback( running_machine *machine, int *code, int *color, int *flags )
{
	rollerg_state *state = machine->driver_data<rollerg_state>();
	*flags = TILE_FLIPYX((*color & 0xc0) >> 6);
	*code |= ((*color & 0x0f) << 8);
	*color = state->zoom_colorbase + ((*color & 0x30) >> 4);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( rollerg )
{
	rollerg_state *state = machine->driver_data<rollerg_state>();
	state->sprite_colorbase = 16;
	state->zoom_colorbase = 0;
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( rollerg )
{
	rollerg_state *state = screen->machine->driver_data<rollerg_state>();
	int bg_colorbase = 16;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 16 * bg_colorbase);
	k051316_zoom_draw(state->k051316, bitmap, cliprect, 0, 1);
	k053245_sprites_draw(state->k053244, bitmap, cliprect);
	return 0;
}
