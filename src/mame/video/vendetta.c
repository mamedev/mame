#include "emu.h"
#include "video/konicdev.h"
#include "includes/vendetta.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void vendetta_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	vendetta_state *state = (vendetta_state *)machine->driver_data;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x30) << 6) | ((*color & 0x0c) << 10) | (bank << 14);
	*color = state->layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

void esckids_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	vendetta_state *state = (vendetta_state *)machine->driver_data;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) <<  9) | (bank << 13);
	*color = state->layer_colorbase[layer] + ((*color & 0xe0) >>  5);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void vendetta_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	vendetta_state *state = (vendetta_state *)machine->driver_data;
	int pri = (*color & 0x03e0) >> 4;	/* ??????? */
	if (pri <= state->layerpri[2])
		*priority_mask = 0;
	else if (pri > state->layerpri[2] && pri <= state->layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->layerpri[1] && pri <= state->layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = state->sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( vendetta )
{
	vendetta_state *state = (vendetta_state *)screen->machine->driver_data;
	int layer[3], bg_colorbase;

	bg_colorbase = k053251_get_palette_index(state->k053251, K053251_CI0);
	state->sprite_colorbase = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->layer_colorbase[0] = k053251_get_palette_index(state->k053251, K053251_CI2);
	state->layer_colorbase[1] = k053251_get_palette_index(state->k053251, K053251_CI3);
	state->layer_colorbase[2] = k053251_get_palette_index(state->k053251, K053251_CI4);

	k052109_tilemap_update(state->k052109);

	layer[0] = 0;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI2);
	layer[1] = 1;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI3);
	layer[2] = 2;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI4);

	konami_sortlayers3(layer, state->layerpri);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, layer[2], 0, 4);

	k053247_sprites_draw(state->k053246, bitmap, cliprect);
	return 0;
}
