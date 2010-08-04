#include "emu.h"
#include "video/konicdev.h"
#include "includes/blockhl.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void blockhl_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	blockhl_state *state = machine->driver_data<blockhl_state>();
	*code |= ((*color & 0x0f) << 8);
	*color = state->layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void blockhl_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	blockhl_state *state = machine->driver_data<blockhl_state>();

	if(*color & 0x10)
		*priority = 0xfe; // under K052109_tilemap[0]
	else
		*priority = 0xfc; // under K052109_tilemap[1]

	*color = state->sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( blockhl )
{
	blockhl_state *state = machine->driver_data<blockhl_state>();

	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x800);

	state->layer_colorbase[0] = 0;
	state->layer_colorbase[1] = 16;
	state->layer_colorbase[2] = 32;
	state->sprite_colorbase = 48;

	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x800);
}

VIDEO_UPDATE( blockhl )
{
	blockhl_state *state = screen->machine->driver_data<blockhl_state>();

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	k052109_tilemap_update(state->k052109);

	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);	// tile 2
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 1);	// tile 1
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 2);	// tile 0

	k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, -1);
	return 0;
}
