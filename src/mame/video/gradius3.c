#include "emu.h"
#include "video/konicdev.h"
#include "includes/gradius3.h"


#define TOTAL_CHARS    0x1000
#define TOTAL_SPRITES  0x4000

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void gradius3_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	gradius3_state *state = machine->driver_data<gradius3_state>();

	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*color = state->layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void gradius3_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask, int *shadow )
{
	#define L0 0xaa
	#define L1 0xcc
	#define L2 0xf0
	static const int primask[2][4] =
	{
		{ L0|L2, L0, L0|L2, L0|L1|L2 },
		{ L1|L2, L2, 0,     L0|L1|L2 }
	};
	gradius3_state *state = machine->driver_data<gradius3_state>();
	int pri = ((*color & 0x60) >> 5);

	if (state->priority == 0)
		*priority_mask = primask[0][pri];
	else
		*priority_mask = primask[1][pri];

	*code |= (*color & 0x01) << 13;
	*color = state->sprite_colorbase + ((*color & 0x1e) >> 1);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static STATE_POSTLOAD( gradius3_postload )
{
	int i;

	for (i = 0; i < 0x20000; i += 16)
	{
		gfx_element_mark_dirty(machine->gfx[0], i / 16);
	}
}

VIDEO_START( gradius3 )
{
	gradius3_state *state = machine->driver_data<gradius3_state>();
	int i;

	state->layer_colorbase[0] = 0;
	state->layer_colorbase[1] = 32;
	state->layer_colorbase[2] = 48;
	state->sprite_colorbase = 16;

	k052109_set_layer_offsets(state->k052109, 2, -2, 0);
	k051960_set_sprite_offsets(state->k051960, 2, 0);

	/* re-decode the sprites because the ROMs are connected to the custom IC differently
       from how they are connected to the CPU. */
	for (i = 0; i < TOTAL_SPRITES; i++)
		gfx_element_mark_dirty(machine->gfx[1], i);

	gfx_element_set_source(machine->gfx[0], (UINT8 *)state->gfxram);

	state_save_register_postload(machine, gradius3_postload, NULL);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_HANDLER( gradius3_gfxrom_r )
{
	UINT8 *gfxdata = memory_region(space->machine, "gfx2");

	return (gfxdata[2 * offset + 1] << 8) | gfxdata[2 * offset];
}

WRITE16_HANDLER( gradius3_gfxram_w )
{
	gradius3_state *state = space->machine->driver_data<gradius3_state>();
	int oldword = state->gfxram[offset];

	COMBINE_DATA(&state->gfxram[offset]);

	if (oldword != state->gfxram[offset])
		gfx_element_mark_dirty(space->machine->gfx[0], offset / 16);
}

/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( gradius3 )
{
	gradius3_state *state = screen->machine->driver_data<gradius3_state>();

	/* TODO: this kludge enforces the char banks. For some reason, they don't work otherwise. */
	k052109_w(state->k052109, 0x1d80, 0x10);
	k052109_w(state->k052109, 0x1f00, 0x32);

	k052109_tilemap_update(state->k052109);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	if (state->priority == 0)
	{
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 2);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 0, 4);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 1);
	}
	else
	{
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 1);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 2);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 0, 4);
	}

	k051960_sprites_draw(state->k051960, bitmap, cliprect, -1, -1);
	return 0;
}
