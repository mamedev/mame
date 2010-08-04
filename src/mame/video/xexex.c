#include "emu.h"
#include "video/konicdev.h"
#include "includes/xexex.h"

void xexex_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	xexex_state *state = machine->driver_data<xexex_state>();
	int pri;

	// Xexex doesn't seem to use bit8 and 9 as effect selectors so this should be safe.
	// (pdrawgfx() still needs change to fix Elaine's end-game graphics)
	pri = (*color & 0x3e0) >> 4;

	if (pri <= state->layerpri[3])
		*priority_mask = 0;
	else if (pri > state->layerpri[3] && pri <= state->layerpri[2])
		*priority_mask = 0xff00;
	else if (pri > state->layerpri[2] && pri <= state->layerpri[1])
		*priority_mask = 0xff00 | 0xf0f0;
	else if (pri > state->layerpri[1] && pri <= state->layerpri[0])
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc;
	else
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa;

	*color = state->sprite_colorbase | (*color & 0x001f);
}

void xexex_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags)
{
	xexex_state *state = machine->driver_data<xexex_state>();
	*color = state->layer_colorbase[layer] | (*color >> 2 & 0x0f);
}

VIDEO_START( xexex )
{
	xexex_state *state = machine->driver_data<xexex_state>();

	assert(machine->primary_screen->format() == BITMAP_FORMAT_RGB32);

	state->cur_alpha = 0;

	// Xexex has relative plane offsets of -2,2,4,6 vs. -2,0,2,3 in MW and GX.
	k056832_set_layer_offs(state->k056832, 0, -2, 16);
	k056832_set_layer_offs(state->k056832, 1,  2, 16);
	k056832_set_layer_offs(state->k056832, 2,  4, 16);
	k056832_set_layer_offs(state->k056832, 3,  6, 16);
}

VIDEO_UPDATE( xexex )
{
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	xexex_state *state = screen->machine->driver_data<xexex_state>();
	int layer[4];
	int bg_colorbase, new_colorbase, plane, alpha;

	state->sprite_colorbase = k053251_get_palette_index(state->k053251, K053251_CI0);
	bg_colorbase = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->layer_colorbase[0] = 0x70;

	for (plane = 1; plane < 4; plane++)
	{
		new_colorbase = k053251_get_palette_index(state->k053251, K053251_CI[plane]);
		if (state->layer_colorbase[plane] != new_colorbase)
		{
			state->layer_colorbase[plane] = new_colorbase;
			k056832_mark_plane_dirty(state->k056832, plane);
		}
	}

	layer[0] = 1;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI2);
	layer[1] = 2;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI3);
	layer[2] = 3;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI4);
	layer[3] = -1;
	state->layerpri[3] = k053251_get_priority(state->k053251, K053251_CI1);

	konami_sortlayers4(layer, state->layerpri);

	k054338_update_all_shadows(state->k054338, 0);
	k054338_fill_backcolor(state->k054338, bitmap, 0);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	for (plane = 0; plane < 4; plane++)
	{
		if (layer[plane] < 0)
		{
			k053250_draw(state->k053250, bitmap, cliprect, bg_colorbase, 0, 1 << plane);
		}
		else if (!state->cur_alpha || layer[plane] != 1)
		{
			k056832_tilemap_draw(state->k056832, bitmap, cliprect, layer[plane], 0, 1 << plane);
		}
	}

	k053247_sprites_draw(state->k053246, bitmap, cliprect);

	if (state->cur_alpha)
	{
		alpha = k054338_set_alpha_level(state->k054338, 1);

		if (alpha > 0)
		{
			k056832_tilemap_draw(state->k056832, bitmap, cliprect, 1, TILEMAP_DRAW_ALPHA(alpha), 0);
		}
	}

	k056832_tilemap_draw(state->k056832, bitmap, cliprect, 0, 0, 0);
	return 0;
}
