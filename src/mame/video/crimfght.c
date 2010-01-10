#include "emu.h"
#include "video/konicdev.h"
#include "includes/crimfght.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void crimfght_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	crimfght_state *state = (crimfght_state *)machine->driver_data;

	*flags = (*color & 0x20) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x1f) << 8) | (bank << 13);
	*color = state->layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void crimfght_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	/* Weird priority scheme. Why use three bits when two would suffice? */
	/* The PROM allows for mixed priorities, where sprites would have */
	/* priority over text but not on one or both of the other two planes. */
	/* Luckily, this isn't used by the game. */
	crimfght_state *state = (crimfght_state *)machine->driver_data;

	switch (*color & 0x70)
	{
		case 0x10: *priority = 0; break;
		case 0x00: *priority = 1; break;
		case 0x40: *priority = 2; break;
		case 0x20: *priority = 3; break;
		/*   0x60 == 0x20 */
		/*   0x50 priority over F and A, but not over B */
		/*   0x30 priority over F, but not over A and B */
		/*   0x70 == 0x30 */
	}
	/* bit 7 is on in the "Game Over" sprites, meaning unknown */
	/* in Aliens it is the top bit of the code, but that's not needed here */
	*color = state->sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( crimfght )
{
	crimfght_state *state = (crimfght_state *)machine->driver_data;

	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x400);

	state->layer_colorbase[0] = 0;
	state->layer_colorbase[1] = 4;
	state->layer_colorbase[2] = 8;
	state->sprite_colorbase = 16;

	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x400);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( crimfght )
{
	crimfght_state *state = (crimfght_state *)screen->machine->driver_data;

	k052109_tilemap_update(state->k052109);

	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 0);
	k051960_sprites_draw(state->k051960, bitmap, cliprect, 2, 2);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 0, 0);
	k051960_sprites_draw(state->k051960, bitmap, cliprect, 1, 1);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 0);
	k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
	return 0;
}
