/***************************************************************************

 Wild West C.O.W.boys of Moo Mesa
 Bucky O'Hare
 (c) 1992 Konami

 Video hardware emulation.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/moo.h"

void moo_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	moo_state *state = (moo_state *)machine->driver_data;
	int pri = (*color & 0x03e0) >> 4;

	if (pri <= state->layerpri[2])
		*priority_mask = 0;
	else if (pri <= state->layerpri[1])
		*priority_mask = 0xf0;
	else if (pri <= state->layerpri[0])
		*priority_mask = 0xf0|0xcc;
	else
		*priority_mask = 0xf0|0xcc|0xaa;

	*color = state->sprite_colorbase | (*color & 0x001f);
}

void moo_tile_callback( running_machine *machine, int layer, int *code, int *color, int *flags )
{
	moo_state *state = (moo_state *)machine->driver_data;
	*color = state->layer_colorbase[layer] | (*color >> 2 & 0x0f);
}

VIDEO_START(moo)
{
	moo_state *state = (moo_state *)machine->driver_data;

	assert(video_screen_get_format(machine->primary_screen) == BITMAP_FORMAT_RGB32);

	state->alpha_enabled = 0;

	if (!strcmp(machine->gamedrv->name, "bucky") || !strcmp(machine->gamedrv->name, "buckyua") || !strcmp(machine->gamedrv->name, "buckyaa"))
	{
		// Bucky doesn't chain tilemaps
		k056832_set_layer_association(state->k056832, 0);

		k056832_set_layer_offs(state->k056832, 0, -2, 0);
		k056832_set_layer_offs(state->k056832, 1,  2, 0);
		k056832_set_layer_offs(state->k056832, 2,  4, 0);
		k056832_set_layer_offs(state->k056832, 3,  6, 0);
	}
	else
	{
		// other than the intro showing one blank line alignment is good through the game
		k056832_set_layer_offs(state->k056832, 0, -2 + 1, 0);
		k056832_set_layer_offs(state->k056832, 1,  2 + 1, 0);
		k056832_set_layer_offs(state->k056832, 2,  4 + 1, 0);
		k056832_set_layer_offs(state->k056832, 3,  6 + 1, 0);
	}
}

VIDEO_UPDATE(moo)
{
	moo_state *state = (moo_state *)screen->machine->driver_data;
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layers[3];
	int bg_colorbase, new_colorbase, plane, dirty, alpha;

	bg_colorbase = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->sprite_colorbase = k053251_get_palette_index(state->k053251, K053251_CI0);
	state->layer_colorbase[0] = 0x70;

	if (k056832_get_layer_association(state->k056832))
	{
		for (plane = 1; plane < 4; plane++)
		{
			new_colorbase = k053251_get_palette_index(state->k053251, K053251_CI[plane]);
			if (state->layer_colorbase[plane] != new_colorbase)
			{
				state->layer_colorbase[plane] = new_colorbase;
				k056832_mark_plane_dirty(state->k056832, plane);
			}
		}
	}
	else
	{
		for (dirty = 0, plane = 1; plane < 4; plane++)
		{
			new_colorbase = k053251_get_palette_index(state->k053251, K053251_CI[plane]);
			if (state->layer_colorbase[plane] != new_colorbase)
			{
				state->layer_colorbase[plane] = new_colorbase;
				dirty = 1;
			}
		}
		if (dirty)
			k056832_mark_all_tmaps_dirty(state->k056832);
	}

	layers[0] = 1;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI2);
	layers[1] = 2;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI3);
	layers[2] = 3;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI4);

	konami_sortlayers3(layers, state->layerpri);

	k054338_update_all_shadows(state->k054338, 0);
	k054338_fill_backcolor(state->k054338, bitmap, 0);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	if (state->layerpri[0] < k053251_get_priority(state->k053251, K053251_CI1))	/* bucky hides back layer behind background */
		k056832_tilemap_draw(state->k056832, bitmap, cliprect, layers[0], 0, 1);

	k056832_tilemap_draw(state->k056832, bitmap, cliprect, layers[1], 0, 2);

	// Enabling alpha improves fog and fading in Moo but causes other things to disappear.
	// There is probably a control bit somewhere to turn off alpha blending.
	state->alpha_enabled = k054338_register_r(state->k054338, K338_REG_CONTROL) & K338_CTL_MIXPRI; // DUMMY

	alpha = (state->alpha_enabled) ? k054338_set_alpha_level(state->k054338, 1) : 255;

	if (alpha > 0)
		k056832_tilemap_draw(state->k056832, bitmap, cliprect, layers[2], TILEMAP_DRAW_ALPHA(alpha), 4);

	k053247_sprites_draw(state->k053246, bitmap, cliprect);

	k056832_tilemap_draw(state->k056832, bitmap, cliprect, 0, 0, 0);
	return 0;
}
