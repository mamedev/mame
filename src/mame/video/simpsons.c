#include "emu.h"
#include "video/konicdev.h"
#include "includes/simpsons.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void simpsons_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	simpsons_state *state = machine.driver_data<simpsons_state>();
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void simpsons_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	simpsons_state *state = machine.driver_data<simpsons_state>();
	int pri = (*color & 0x0f80) >> 6;	/* ??????? */

	if (pri <= state->m_layerpri[2])
		*priority_mask = 0;
	else if (pri > state->m_layerpri[2] && pri <= state->m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->m_layerpri[1] && pri <= state->m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = state->m_sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

  Extra video banking

***************************************************************************/

READ8_MEMBER(simpsons_state::simpsons_k052109_r)
{
	return k052109_r(m_k052109, offset + 0x2000);
}

WRITE8_MEMBER(simpsons_state::simpsons_k052109_w)
{
	k052109_w(m_k052109, offset + 0x2000, data);
}

READ8_MEMBER(simpsons_state::simpsons_k053247_r)
{
	int offs;

	if (offset < 0x1000)
	{
		offs = offset >> 1;

		if (offset & 1)
			return(m_spriteram[offs] & 0xff);
		else
			return(m_spriteram[offs] >> 8);
	}
	else
		return m_xtraram[offset - 0x1000];
}

WRITE8_MEMBER(simpsons_state::simpsons_k053247_w)
{
	int offs;

	if (offset < 0x1000)
	{
		UINT16 *spriteram = m_spriteram;
		offs = offset >> 1;

		if (offset & 1)
			spriteram[offs] = (spriteram[offs] & 0xff00) | data;
		else
			spriteram[offs] = (spriteram[offs] & 0x00ff) | (data << 8);
	}
	else m_xtraram[offset - 0x1000] = data;
}

void simpsons_video_banking( running_machine &machine, int bank )
{
	simpsons_state *state = machine.driver_data<simpsons_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	if (bank & 1)
	{
		space->install_read_bank(0x0000, 0x0fff, "bank5");
		space->install_write_handler(0x0000, 0x0fff, write8_delegate(FUNC(simpsons_state::paletteram_xBBBBBGGGGGRRRRR_be_w), state));
		memory_set_bankptr(machine, "bank5", state->m_generic_paletteram_8);
	}
	else
		space->install_legacy_readwrite_handler(*state->m_k052109, 0x0000, 0x0fff, FUNC(k052109_r), FUNC(k052109_w));

	if (bank & 2)
		space->install_readwrite_handler(0x2000, 0x3fff, read8_delegate(FUNC(simpsons_state::simpsons_k053247_r),state), write8_delegate(FUNC(simpsons_state::simpsons_k053247_w),state));
	else
		space->install_readwrite_handler(0x2000, 0x3fff, read8_delegate(FUNC(simpsons_state::simpsons_k052109_r),state), write8_delegate(FUNC(simpsons_state::simpsons_k052109_w),state));
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( simpsons )
{
	simpsons_state *state = screen.machine().driver_data<simpsons_state>();
	int layer[3], bg_colorbase;

	bg_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI0);
	state->m_sprite_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI1);
	state->m_layer_colorbase[0] = k053251_get_palette_index(state->m_k053251, K053251_CI2);
	state->m_layer_colorbase[1] = k053251_get_palette_index(state->m_k053251, K053251_CI3);
	state->m_layer_colorbase[2] = k053251_get_palette_index(state->m_k053251, K053251_CI4);

	k052109_tilemap_update(state->m_k052109);

	layer[0] = 0;
	state->m_layerpri[0] = k053251_get_priority(state->m_k053251, K053251_CI2);
	layer[1] = 1;
	state->m_layerpri[1] = k053251_get_priority(state->m_k053251, K053251_CI3);
	layer[2] = 2;
	state->m_layerpri[2] = k053251_get_priority(state->m_k053251, K053251_CI4);

	konami_sortlayers3(layer, state->m_layerpri);

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, layer[0], 0, 1);
	k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, layer[2], 0, 4);

	k053247_sprites_draw(state->m_k053246, bitmap, cliprect);
	return 0;
}
