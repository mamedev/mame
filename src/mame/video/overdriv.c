#include "emu.h"
#include "video/konicdev.h"
#include "includes/overdriv.h"

/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void overdriv_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	overdriv_state *state = machine->driver_data<overdriv_state>();
	int pri = (*color & 0xffe0) >> 5;	/* ??????? */
	if (pri)
		*priority_mask = 0x02;
	else
		*priority_mask = 0x00;

	*color = state->sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void overdriv_zoom_callback_0( running_machine *machine, int *code, int *color, int *flags )
{
	overdriv_state *state = machine->driver_data<overdriv_state>();
	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8);
	*color = state->zoom_colorbase[0] + ((*color & 0x3c) >> 2);
}

void overdriv_zoom_callback_1( running_machine *machine, int *code, int *color, int *flags )
{
	overdriv_state *state = machine->driver_data<overdriv_state>();
	*flags = (*color & 0x40) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8);
	*color = state->zoom_colorbase[1] + ((*color & 0x3c) >> 2);
}


/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( overdriv )
{
	overdriv_state *state = screen->machine->driver_data<overdriv_state>();

	state->sprite_colorbase  = k053251_get_palette_index(state->k053251, K053251_CI0);
	state->road_colorbase[1] = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->road_colorbase[0] = k053251_get_palette_index(state->k053251, K053251_CI2);
	state->zoom_colorbase[1] = k053251_get_palette_index(state->k053251, K053251_CI3);
	state->zoom_colorbase[0] = k053251_get_palette_index(state->k053251, K053251_CI4);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	k051316_zoom_draw(state->k051316_1, bitmap, cliprect, 0, 0);
	k051316_zoom_draw(state->k051316_2, bitmap, cliprect, 0, 1);

	k053247_sprites_draw(state->k053246, bitmap,cliprect);
	return 0;
}
