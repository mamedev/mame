#include "emu.h"
#include "video/konicdev.h"
#include "includes/88games.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void _88games_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	_88games_state *state = machine->driver_data<_88games_state>();

	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = state->layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void _88games_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	_88games_state *state = machine->driver_data<_88games_state>();

	*priority = (*color & 0x20) >> 5;	/* ??? */
	*color = state->sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void _88games_zoom_callback( running_machine *machine, int *code, int *color, int *flags )
{
	_88games_state *state = machine->driver_data<_88games_state>();

	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x07) << 8);
	*color = state->zoom_colorbase + ((*color & 0x38) >> 3) + ((*color & 0x80) >> 4);
}

/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( 88games )
{
	_88games_state *state = screen->machine->driver_data<_88games_state>();

	k052109_tilemap_update(state->k052109);

	if (state->k88games_priority)
	{
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);	// tile 0
		k051960_sprites_draw(state->k051960, bitmap,cliprect, 1, 1);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 0, 0);	// tile 2
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 0);	// tile 1
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
		k051316_zoom_draw(state->k051316, bitmap, cliprect, 0, 0);
	}
	else
	{
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);	// tile 2
		k051316_zoom_draw(state->k051316, bitmap, cliprect, 0, 0);
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 0);	// tile 1
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 1, 1);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 0);	// tile 0
	}

	return 0;
}
