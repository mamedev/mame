#include "emu.h"
#include "video/konicdev.h"
#include "includes/asterix.h"


static void reset_spritebank( running_machine *machine )
{
	asterix_state *state = (asterix_state *)machine->driver_data;
	k053244_bankselect(state->k053244, state->spritebank & 7);
	state->spritebanks[0] = (state->spritebank << 12) & 0x7000;
	state->spritebanks[1] = (state->spritebank <<  9) & 0x7000;
	state->spritebanks[2] = (state->spritebank <<  6) & 0x7000;
	state->spritebanks[3] = (state->spritebank <<  3) & 0x7000;
}

WRITE16_HANDLER( asterix_spritebank_w )
{
	asterix_state *state = (asterix_state *)space->machine->driver_data;
	COMBINE_DATA(&state->spritebank);
	reset_spritebank(space->machine);
}

void asterix_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	asterix_state *state = (asterix_state *)machine->driver_data;
	int pri = (*color & 0x00e0) >> 2;
	if (pri <= state->layerpri[2])
		*priority_mask = 0;
	else if (pri > state->layerpri[2] && pri <= state->layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->layerpri[1] && pri <= state->layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;
	*color = state->sprite_colorbase | (*color & 0x001f);
	*code = (*code & 0xfff) | state->spritebanks[(*code >> 12) & 3];
}


void asterix_tile_callback( running_machine *machine, int layer, int *code, int *color, int *flags )
{
	asterix_state *state = (asterix_state *)machine->driver_data;

	*flags = *code & 0x1000 ? TILE_FLIPX : 0;
	*color = (state->layer_colorbase[layer] + ((*code & 0xe000) >> 13)) & 0x7f;
	*code = (*code & 0x03ff) | state->tilebanks[(*code >> 10) & 3];
}

VIDEO_UPDATE( asterix )
{
	asterix_state *state = (asterix_state *)screen->machine->driver_data;
	static const int K053251_CI[4] = { K053251_CI0, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layer[3], plane, new_colorbase;

	/* Layer offsets are different if horizontally flipped */
	if (k056832_read_register(state->k056832, 0x0) & 0x10)
	{
		k056832_set_layer_offs(state->k056832, 0, 89 - 176, 0);
		k056832_set_layer_offs(state->k056832, 1, 91 - 176, 0);
		k056832_set_layer_offs(state->k056832, 2, 89 - 176, 0);
		k056832_set_layer_offs(state->k056832, 3, 95 - 176, 0);
	}
	else
	{
		k056832_set_layer_offs(state->k056832, 0, 89, 0);
		k056832_set_layer_offs(state->k056832, 1, 91, 0);
		k056832_set_layer_offs(state->k056832, 2, 89, 0);
		k056832_set_layer_offs(state->k056832, 3, 95, 0);
	}


	state->tilebanks[0] = (k056832_get_lookup(state->k056832, 0) << 10);
	state->tilebanks[1] = (k056832_get_lookup(state->k056832, 1) << 10);
	state->tilebanks[2] = (k056832_get_lookup(state->k056832, 2) << 10);
	state->tilebanks[3] = (k056832_get_lookup(state->k056832, 3) << 10);

	// update color info and refresh tilemaps
	state->sprite_colorbase = k053251_get_palette_index(state->k053251, K053251_CI1);

	for (plane = 0; plane < 4; plane++)
	{
		new_colorbase = k053251_get_palette_index(state->k053251, K053251_CI[plane]);
		if (state->layer_colorbase[plane] != new_colorbase)
		{
			state->layer_colorbase[plane] = new_colorbase;
			k056832_mark_plane_dirty(state->k056832, plane);
		}
	}

	layer[0] = 0;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI0);
	layer[1] = 1;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI2);
	layer[2] = 3;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI4);

	konami_sortlayers3(layer, state->layerpri);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0);

	k056832_tilemap_draw(state->k056832, bitmap, cliprect, layer[0], 0, 1);
	k056832_tilemap_draw(state->k056832, bitmap, cliprect, layer[1], 0, 2);
	k056832_tilemap_draw(state->k056832, bitmap, cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows in front of feet */
	k053245_sprites_draw(state->k053244, bitmap, cliprect);

	k056832_tilemap_draw(state->k056832, bitmap, cliprect, 2, 0, 0);
	return 0;
}
