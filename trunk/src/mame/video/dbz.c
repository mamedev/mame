/*
  Dragonball Z
  (c) 1993 Banpresto
  Dragonball Z 2 Super Battle
  (c) 1994 Banpresto

  Video hardware emulation.
*/


#include "emu.h"
#include "video/konicdev.h"
#include "includes/dbz.h"


void dbz_tile_callback( running_machine &machine, int layer, int *code, int *color, int *flags )
{
	dbz_state *state = machine.driver_data<dbz_state>();
	*color = (state->m_layer_colorbase[layer] << 1) + ((*color & 0x3c) >> 2);
}

void dbz_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	dbz_state *state = machine.driver_data<dbz_state>();
	int pri = (*color & 0x3c0) >> 5;

	if (pri <= state->m_layerpri[3])
		*priority_mask = 0xff00;
	else if (pri > state->m_layerpri[3] && pri <= state->m_layerpri[2])
		*priority_mask = 0xfff0;
	else if (pri > state->m_layerpri[2] && pri <= state->m_layerpri[1])
		*priority_mask = 0xfffc;
	else
		*priority_mask = 0xfffe;

	*color = (state->m_sprite_colorbase << 1) + (*color & 0x1f);
}

/* Background Tilemaps */

WRITE16_MEMBER(dbz_state::dbz_bg2_videoram_w)
{
	COMBINE_DATA(&m_bg2_videoram[offset]);
	m_bg2_tilemap->mark_tile_dirty(offset / 2);
}

static TILE_GET_INFO( get_dbz_bg2_tile_info )
{
	dbz_state *state = machine.driver_data<dbz_state>();
	int tileno, colour, flag;

	tileno = state->m_bg2_videoram[tile_index * 2 + 1] & 0x7fff;
	colour = (state->m_bg2_videoram[tile_index * 2] & 0x000f);
	flag = (state->m_bg2_videoram[tile_index * 2] & 0x0080) ? TILE_FLIPX : 0;

	SET_TILE_INFO(0, tileno, colour + (state->m_layer_colorbase[5] << 1), flag);
}

WRITE16_MEMBER(dbz_state::dbz_bg1_videoram_w)
{
	COMBINE_DATA(&m_bg1_videoram[offset]);
	m_bg1_tilemap->mark_tile_dirty(offset / 2);
}

static TILE_GET_INFO( get_dbz_bg1_tile_info )
{
	dbz_state *state = machine.driver_data<dbz_state>();
	int tileno, colour, flag;

	tileno = state->m_bg1_videoram[tile_index * 2 + 1] & 0x7fff;
	colour = (state->m_bg1_videoram[tile_index * 2] & 0x000f);
	flag = (state->m_bg1_videoram[tile_index * 2] & 0x0080) ? TILE_FLIPX : 0;

	SET_TILE_INFO(1, tileno, colour + (state->m_layer_colorbase[4] << 1), flag);
}

VIDEO_START( dbz )
{
	dbz_state *state = machine.driver_data<dbz_state>();

	state->m_bg1_tilemap = tilemap_create(machine, get_dbz_bg1_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_bg2_tilemap = tilemap_create(machine, get_dbz_bg2_tile_info, tilemap_scan_rows, 16, 16, 64, 32);

	state->m_bg1_tilemap->set_transparent_pen(0);
	state->m_bg2_tilemap->set_transparent_pen(0);

	if (!strcmp(machine.system().name, "dbz"))
		k056832_set_layer_offs(state->m_k056832, 0, -34, -16);
	else
		k056832_set_layer_offs(state->m_k056832, 0, -35, -16);

	k056832_set_layer_offs(state->m_k056832, 1, -31, -16);
	k056832_set_layer_offs(state->m_k056832, 3, -31, -16); //?

	k053247_set_sprite_offs(state->m_k053246, -87, 32);
}

SCREEN_UPDATE_IND16( dbz )
{
	dbz_state *state = screen.machine().driver_data<dbz_state>();
	static const int K053251_CI[6] = { K053251_CI3, K053251_CI4, K053251_CI4, K053251_CI4, K053251_CI2, K053251_CI1 };
	int layer[5], plane, new_colorbase;

	state->m_sprite_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI0);

	for (plane = 0; plane < 6; plane++)
	{
		new_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI[plane]);
		if (state->m_layer_colorbase[plane] != new_colorbase)
		{
			state->m_layer_colorbase[plane] = new_colorbase;
			if (plane <= 3)
				k056832_mark_plane_dirty(state->m_k056832, plane);
			else if (plane == 4)
				state->m_bg1_tilemap->mark_all_dirty();
			else if (plane == 5)
				state->m_bg2_tilemap->mark_all_dirty();
		}
	}

	//layers priority

	layer[0] = 0;
	state->m_layerpri[0] = k053251_get_priority(state->m_k053251, K053251_CI3);
	layer[1] = 1;
	state->m_layerpri[1] = k053251_get_priority(state->m_k053251, K053251_CI4);
	layer[2] = 3;
	state->m_layerpri[2] = k053251_get_priority(state->m_k053251, K053251_CI0);
	layer[3] = 4;
	state->m_layerpri[3] = k053251_get_priority(state->m_k053251, K053251_CI2);
	layer[4] = 5;
	state->m_layerpri[4] = k053251_get_priority(state->m_k053251, K053251_CI1);

	konami_sortlayers5(layer, state->m_layerpri);

	screen.machine().priority_bitmap.fill(0, cliprect);

	for (plane = 0; plane < 5; plane++)
	{
		int flag, pri;

		if (plane == 0)
		{
			flag = TILEMAP_DRAW_OPAQUE;
			pri = 0;
		}
		else
		{
			flag = 0;
			pri = 1 << (plane - 1);
		}

		if(layer[plane] == 4)
			k053936_zoom_draw(state->m_k053936_2, bitmap, cliprect, state->m_bg1_tilemap, flag, pri, 1);
		else if(layer[plane] == 5)
			k053936_zoom_draw(state->m_k053936_1, bitmap, cliprect, state->m_bg2_tilemap, flag, pri, 1);
		else
			k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layer[plane], flag, pri);
	}

	k053247_sprites_draw(state->m_k053246, bitmap, cliprect);
	return 0;
}

