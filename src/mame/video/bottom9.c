#include "emu.h"
#include "video/konicdev.h"
#include "includes/bottom9.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void bottom9_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	bottom9_state *state = machine->driver_data<bottom9_state>();
	*code |= (*color & 0x3f) << 8;
	*color = state->layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void bottom9_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	/* bit 4 = priority over zoom (0 = have priority) */
	/* bit 5 = priority over B (1 = have priority) */
	bottom9_state *state = machine->driver_data<bottom9_state>();
	*priority = (*color & 0x30) >> 4;
	*color = state->sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void bottom9_zoom_callback( running_machine *machine, int *code, int *color, int *flags )
{
	bottom9_state *state = machine->driver_data<bottom9_state>();
	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8);
	*color = state->zoom_colorbase + ((*color & 0x3c) >> 2);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bottom9 )
{
	bottom9_state *state = machine->driver_data<bottom9_state>();

	state->layer_colorbase[0] = 0;	/* not used */
	state->layer_colorbase[1] = 0;
	state->layer_colorbase[2] = 16;
	state->sprite_colorbase = 32;
	state->zoom_colorbase = 48;
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( bottom9 )
{
	bottom9_state *state = screen->machine->driver_data<bottom9_state>();

	k052109_tilemap_update(state->k052109);

	/* note: FIX layer is not used */
	bitmap_fill(bitmap, cliprect, state->layer_colorbase[1]);
//  if (state->video_enable)
	{
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 1, 1);
		k051316_zoom_draw(state->k051316, bitmap, cliprect, 0, 0);
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 0, 0);
		/* note that priority 3 is opposite to the basic layer priority! */
		/* (it IS used, but hopefully has no effect) */
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 2, 3);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 0);
	}
	return 0;
}
