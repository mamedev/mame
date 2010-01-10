/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/chqflag.h"


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void chqflag_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	chqflag_state *state = (chqflag_state *)machine->driver_data;
	*priority = (*color & 0x10) >> 4;
	*color = state->sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void chqflag_zoom_callback_0( running_machine *machine, int *code, int *color, int *flags )
{
	chqflag_state *state = (chqflag_state *)machine->driver_data;
	*code |= ((*color & 0x03) << 8);
	*color = state->zoom_colorbase[0] + ((*color & 0x3c) >> 2);
}

void chqflag_zoom_callback_1( running_machine *machine, int *code, int *color, int *flags )
{
	chqflag_state *state = (chqflag_state *)machine->driver_data;
	*flags = TILE_FLIPYX((*color & 0xc0) >> 6);
	*code |= ((*color & 0x0f) << 8);
	*color = state->zoom_colorbase[1] + ((*color & 0x10) >> 4);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( chqflag )
{
	chqflag_state *state = (chqflag_state *)machine->driver_data;

	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x800);

	state->sprite_colorbase = 0;
	state->zoom_colorbase[0] = 0x10;
	state->zoom_colorbase[1] = 0x02;

	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x800);
}

/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( chqflag )
{
	chqflag_state *state = (chqflag_state *)screen->machine->driver_data;

	bitmap_fill(bitmap, cliprect, 0);

	k051316_zoom_draw(state->k051316_2, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
	k051316_zoom_draw(state->k051316_2, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	k051960_sprites_draw(state->k051960, bitmap, cliprect, 1, 1);
	k051316_zoom_draw(state->k051316_1, bitmap, cliprect, 0, 0);
	return 0;
}
