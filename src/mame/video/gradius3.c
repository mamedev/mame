#include "emu.h"
#include "video/konicdev.h"
#include "includes/gradius3.h"


#define TOTAL_CHARS    0x1000
#define TOTAL_SPRITES  0x4000

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void gradius3_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	gradius3_state *state = machine.driver_data<gradius3_state>();

	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void gradius3_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask, int *shadow )
{
	#define L0 0xaa
	#define L1 0xcc
	#define L2 0xf0
	static const int primask[2][4] =
	{
		{ L0|L2, L0, L0|L2, L0|L1|L2 },
		{ L1|L2, L2, 0,     L0|L1|L2 }
	};
	gradius3_state *state = machine.driver_data<gradius3_state>();
	int pri = ((*color & 0x60) >> 5);

	if (state->m_priority == 0)
		*priority_mask = primask[0][pri];
	else
		*priority_mask = primask[1][pri];

	*code |= (*color & 0x01) << 13;
	*color = state->m_sprite_colorbase + ((*color & 0x1e) >> 1);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void gradius3_postload(running_machine &machine)
{
	int i;

	for (i = 0; i < 0x20000; i += 16)
	{
		gfx_element_mark_dirty(machine.gfx[0], i / 16);
	}
}

VIDEO_START( gradius3 )
{
	gradius3_state *state = machine.driver_data<gradius3_state>();
	int i;

	state->m_layer_colorbase[0] = 0;
	state->m_layer_colorbase[1] = 32;
	state->m_layer_colorbase[2] = 48;
	state->m_sprite_colorbase = 16;

	k052109_set_layer_offsets(state->m_k052109, 2, -2, 0);
	k051960_set_sprite_offsets(state->m_k051960, 2, 0);

	/* re-decode the sprites because the ROMs are connected to the custom IC differently
       from how they are connected to the CPU. */
	for (i = 0; i < TOTAL_SPRITES; i++)
		gfx_element_mark_dirty(machine.gfx[1], i);

	gfx_element_set_source(machine.gfx[0], (UINT8 *)state->m_gfxram);

	machine.save().register_postload(save_prepost_delegate(FUNC(gradius3_postload), &machine));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_MEMBER(gradius3_state::gradius3_gfxrom_r)
{
	UINT8 *gfxdata = machine().region("gfx2")->base();

	return (gfxdata[2 * offset + 1] << 8) | gfxdata[2 * offset];
}

WRITE16_MEMBER(gradius3_state::gradius3_gfxram_w)
{
	int oldword = m_gfxram[offset];

	COMBINE_DATA(&m_gfxram[offset]);

	if (oldword != m_gfxram[offset])
		gfx_element_mark_dirty(machine().gfx[0], offset / 16);
}

/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( gradius3 )
{
	gradius3_state *state = screen.machine().driver_data<gradius3_state>();

	/* TODO: this kludge enforces the char banks. For some reason, they don't work otherwise. */
	k052109_w(state->m_k052109, 0x1d80, 0x10);
	k052109_w(state->m_k052109, 0x1f00, 0x32);

	k052109_tilemap_update(state->m_k052109);

	screen.machine().priority_bitmap.fill(0, cliprect);
	if (state->m_priority == 0)
	{
		k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 2);
		k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, 2, 0, 4);
		k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, 0, 0, 1);
	}
	else
	{
		k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 1);
		k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, 1, 0, 2);
		k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, 2, 0, 4);
	}

	k051960_sprites_draw(state->m_k051960, bitmap, cliprect, -1, -1);
	return 0;
}
