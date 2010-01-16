/***************************************************************************

    Lethal Enforcers
     (c) 1992 Konami

    Video hardware emulation.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/lethal.h"

void lethalen_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	int pri = (*color & 0xfff0);
	*color = *color & 0x000f;
	*color += 0x400 / 64; // colourbase?

	/* this isn't ideal.. shouldn't need to hardcode it? not 100% sure about it anyway*/
	if (pri == 0x10)
		*priority_mask = 0xf0; // guys on first level
	else if (pri == 0x90)
		*priority_mask = 0xf0; // car doors
	else if (pri == 0x20)
		*priority_mask = 0xf0 | 0xcc; // people behind glass on 1st level
	else if (pri == 0xa0)
		*priority_mask = 0xf0 | 0xcc; // glass on 1st/2nd level
	else if (pri == 0x40)
		*priority_mask = 0; // blood splats?
	else if (pri == 0x00)
		*priority_mask = 0; // gunshots etc
	else if (pri == 0x30)
		*priority_mask = 0xf0 | 0xcc | 0xaa; // mask sprites (always in a bad colour, used to do special effects i think
	else
	{
		popmessage("unknown pri %04x\n", pri);
		*priority_mask = 0;
	}

	*code = (*code & 0x3fff); // | spritebanks[(*code >> 12) & 3];
}

void lethalen_tile_callback( running_machine *machine, int layer, int *code, int *color, int *flags )
{
	lethal_state *state = (lethal_state *)machine->driver_data;
	*color = state->layer_colorbase[layer] + ((*color & 0x3c) << 2);
}

VIDEO_START(lethalen)
{
	lethal_state *state = (lethal_state *)machine->driver_data;

	// this game uses external linescroll RAM
	k056832_SetExtLinescroll(state->k056832);

	// the US and Japanese cabinets apparently use different mirror setups
	if (!strcmp(machine->gamedrv->name, "lethalenj"))
	{
		k056832_set_layer_offs(state->k056832, 0, -196, 0);
		k056832_set_layer_offs(state->k056832, 1, -194, 0);
		k056832_set_layer_offs(state->k056832, 2, -192, 0);
		k056832_set_layer_offs(state->k056832, 3, -190, 0);
	}
	else
	{ /* fixme */
		k056832_set_layer_offs(state->k056832, 0, 188, 0);
		k056832_set_layer_offs(state->k056832, 1, 190, 0);
		k056832_set_layer_offs(state->k056832, 2, 192, 0);
		k056832_set_layer_offs(state->k056832, 3, 194, 0);
	}

	state->layer_colorbase[0] = 0x00;
	state->layer_colorbase[1] = 0x40;
	state->layer_colorbase[2] = 0x80;
	state->layer_colorbase[3] = 0xc0;
}

WRITE8_HANDLER(lethalen_palette_control)
{
	lethal_state *state = (lethal_state *)space->machine->driver_data;

	switch (offset)
	{
		case 0:	// 40c8 - PCU1 from schematics
			state->layer_colorbase[0] = ((data & 0x7) - 1) * 0x40;
			state->layer_colorbase[1] = (((data >> 4) & 0x7) - 1) * 0x40;
			k056832_mark_plane_dirty(state->k056832, 0);
			k056832_mark_plane_dirty(state->k056832, 1);
			break;

		case 4: // 40cc - PCU2 from schematics
			state->layer_colorbase[2] = ((data & 0x7) - 1) * 0x40;
			state->layer_colorbase[3] = (((data >> 4) & 0x7) - 1) * 0x40;
			k056832_mark_plane_dirty(state->k056832, 2);
			k056832_mark_plane_dirty(state->k056832, 3);
			break;

		case 8:	// 40d0 - PCU3 from schematics
			state->sprite_colorbase = ((data & 0x7) - 1) * 0x40;
			break;
	}
}

VIDEO_UPDATE(lethalen)
{
	lethal_state *state = (lethal_state *)screen->machine->driver_data;

	bitmap_fill(bitmap, cliprect, 7168);
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	k056832_tilemap_draw(state->k056832, bitmap, cliprect, 3, 0, 1);
	k056832_tilemap_draw(state->k056832, bitmap, cliprect, 2, 0, 2);
	k056832_tilemap_draw(state->k056832, bitmap, cliprect, 1, 0, 4);

	k053245_sprites_draw_lethal(state->k053244, bitmap, cliprect);

	// force "A" layer over top of everything
	k056832_tilemap_draw(state->k056832, bitmap, cliprect, 0, 0, 0);

	return 0;
}
