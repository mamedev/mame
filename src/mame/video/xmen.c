#include "emu.h"
#include "video/konicdev.h"
#include "includes/xmen.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void xmen_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	xmen_state *state = machine->driver_data<xmen_state>();

	/* (color & 0x02) is flip y handled internally by the 052109 */
	if (layer == 0)
		*color = state->layer_colorbase[layer] + ((*color & 0xf0) >> 4);
	else
		*color = state->layer_colorbase[layer] + ((*color & 0x7c) >> 2);
}

/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void xmen_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	xmen_state *state = machine->driver_data<xmen_state>();
	int pri = (*color & 0x00e0) >> 4;	/* ??????? */

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

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( xmen6p )
{
	xmen_state *state = machine->driver_data<xmen_state>();

	k053247_get_ram(state->k053246, &state->k053247_ram);

	state->screen_left  = auto_bitmap_alloc(machine, 64 * 8, 32 * 8, BITMAP_FORMAT_INDEXED16);
	state->screen_right = auto_bitmap_alloc(machine, 64 * 8, 32 * 8, BITMAP_FORMAT_INDEXED16);

	state_save_register_global_bitmap(machine, state->screen_left);
	state_save_register_global_bitmap(machine, state->screen_right);
}


/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( xmen )
{
	xmen_state *state = screen->machine->driver_data<xmen_state>();
	int layer[3], bg_colorbase;

	bg_colorbase = k053251_get_palette_index(state->k053251, K053251_CI4);
	state->sprite_colorbase = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->layer_colorbase[0] = k053251_get_palette_index(state->k053251, K053251_CI3);
	state->layer_colorbase[1] = k053251_get_palette_index(state->k053251, K053251_CI0);
	state->layer_colorbase[2] = k053251_get_palette_index(state->k053251, K053251_CI2);

	k052109_tilemap_update(state->k052109);

	layer[0] = 0;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI3);
	layer[1] = 1;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI0);
	layer[2] = 2;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI2);

	konami_sortlayers3(layer, state->layerpri);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	/* note the '+1' in the background color!!! */
	bitmap_fill(bitmap, cliprect, 16 * bg_colorbase + 1);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, layer[0], 0, 1);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
	k053247_sprites_draw(state->k053246, bitmap, cliprect);
	return 0;
}


VIDEO_UPDATE( xmen6p )
{
	xmen_state *state = screen->machine->driver_data<xmen_state>();
	int x, y;

	if (screen == state->lscreen)
		for(y = 0; y < 32 * 8; y++)
		{
			UINT16* line_dest = BITMAP_ADDR16(bitmap, y, 0);
			UINT16* line_src = BITMAP_ADDR16(state->screen_left, y, 0);

			for (x = 12 * 8; x < 52 * 8; x++)
				line_dest[x] = line_src[x];
		}
	else if (screen == state->rscreen)
		for(y = 0; y < 32 * 8; y++)
		{
			UINT16* line_dest = BITMAP_ADDR16(bitmap, y, 0);
			UINT16* line_src = BITMAP_ADDR16(state->screen_right, y, 0);

			for (x = 12 * 8; x < 52 * 8; x++)
				line_dest[x] = line_src[x];
		}

	return 0;
}

/* my lefts and rights are mixed up in several places.. */
VIDEO_EOF( xmen6p )
{
	xmen_state *state = machine->driver_data<xmen_state>();
	int layer[3], bg_colorbase;
	bitmap_t * renderbitmap;
	rectangle cliprect;
	int offset;

	state->current_frame ^= 0x01;

//  const rectangle *visarea = machine->primary_screen->visible_area();
//  cliprect.min_x = visarea->min_x;
//  cliprect.max_x = visarea->max_x;
//  cliprect.min_y = visarea->min_y;
//  cliprect.max_y = visarea->max_y;

	cliprect.min_x = 0;
	cliprect.max_x = 64 * 8 - 1;
	cliprect.min_y = 2 * 8;
	cliprect.max_y = 30 * 8 - 1;


	if (state->current_frame & 0x01)
	{

		/* copy the desired spritelist to the chip */
		memcpy(state->k053247_ram, state->xmen6p_spriteramright, 0x1000);

		/* we write the entire content of the tileram to the chip to ensure
           everything gets marked as dirty and the desired tilemap is rendered
           this is not very efficient!
           */
		for (offset = 0; offset < (0xc000 / 2); offset++)
		{
//          K052109_lsb_w
			k052109_w(state->k052109, offset, state->xmen6p_tilemapright[offset] & 0x00ff);
		}


		renderbitmap = state->screen_right;
	}
	else
	{
		/* copy the desired spritelist to the chip */
		memcpy(state->k053247_ram, state->xmen6p_spriteramleft, 0x1000);

		/* we write the entire content of the tileram to the chip to ensure
           everything gets marked as dirty and the desired tilemap is rendered

           this is not very efficient!
           */
		for (offset = 0; offset < (0xc000 / 2); offset++)
		{
//          K052109_lsb_w
			k052109_w(state->k052109, offset, state->xmen6p_tilemapleft[offset] & 0x00ff);
		}


		renderbitmap = state->screen_left;
	}


	bg_colorbase = k053251_get_palette_index(state->k053251, K053251_CI4);
	state->sprite_colorbase = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->layer_colorbase[0] = k053251_get_palette_index(state->k053251, K053251_CI3);
	state->layer_colorbase[1] = k053251_get_palette_index(state->k053251, K053251_CI0);
	state->layer_colorbase[2] = k053251_get_palette_index(state->k053251, K053251_CI2);

	k052109_tilemap_update(state->k052109);

	layer[0] = 0;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI3);
	layer[1] = 1;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI0);
	layer[2] = 2;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI2);

	konami_sortlayers3(layer, state->layerpri);

	bitmap_fill(machine->priority_bitmap, &cliprect, 0);
	/* note the '+1' in the background color!!! */
	bitmap_fill(renderbitmap, &cliprect, 16 * bg_colorbase + 1);
	k052109_tilemap_draw(state->k052109, renderbitmap, &cliprect, layer[0], 0, 1);
	k052109_tilemap_draw(state->k052109, renderbitmap, &cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(state->k052109, renderbitmap, &cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
	k053247_sprites_draw(state->k053246, renderbitmap, &cliprect);
}
