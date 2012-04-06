/********************************************************************

    Ashita no Joe (Success Joe) [Wave]
    video hardware emulation

*********************************************************************/

#include "emu.h"
#include "includes/ashnojoe.h"


static TILE_GET_INFO( get_joe_tile_info )
{
	ashnojoe_state *state = machine.driver_data<ashnojoe_state>();
	int code = state->m_tileram[tile_index];

	SET_TILE_INFO(
			2,
			code & 0xfff,
			((code >> 12) & 0x0f),
			0);
}

static TILE_GET_INFO( get_joe_tile_info_2 )
{
	ashnojoe_state *state = machine.driver_data<ashnojoe_state>();
	int code = state->m_tileram_2[tile_index * 2];
	int attr = state->m_tileram_2[tile_index * 2 + 1];

	SET_TILE_INFO(
			4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x40,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_3 )
{
	ashnojoe_state *state = machine.driver_data<ashnojoe_state>();
	int code = state->m_tileram_3[tile_index];

	SET_TILE_INFO(
			0,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x10,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_4 )
{
	ashnojoe_state *state = machine.driver_data<ashnojoe_state>();
	int code = state->m_tileram_4[tile_index];

	SET_TILE_INFO(
			1,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x60,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_5 )
{
	ashnojoe_state *state = machine.driver_data<ashnojoe_state>();
	int code = state->m_tileram_5[tile_index * 2];
	int attr = state->m_tileram_5[tile_index * 2 + 1];

	SET_TILE_INFO(
			4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x20,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_6 )
{
	ashnojoe_state *state = machine.driver_data<ashnojoe_state>();
	int code = state->m_tileram_6[tile_index * 2];
	int attr = state->m_tileram_6[tile_index * 2 + 1];

	SET_TILE_INFO(
			3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}


static TILE_GET_INFO( get_joe_tile_info_7 )
{
	ashnojoe_state *state = machine.driver_data<ashnojoe_state>();
	int code = state->m_tileram_7[tile_index * 2];
	int attr = state->m_tileram_7[tile_index * 2 + 1];

	SET_TILE_INFO(
			3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}

WRITE16_MEMBER(ashnojoe_state::ashnojoe_tileram_w)
{

	m_tileram[offset] = data;
	m_joetilemap->mark_tile_dirty(offset);
}


WRITE16_MEMBER(ashnojoe_state::ashnojoe_tileram2_w)
{

	m_tileram_2[offset] = data;
	m_joetilemap2->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(ashnojoe_state::ashnojoe_tileram3_w)
{

	m_tileram_3[offset] = data;
	m_joetilemap3->mark_tile_dirty(offset);
}

WRITE16_MEMBER(ashnojoe_state::ashnojoe_tileram4_w)
{

	m_tileram_4[offset] = data;
	m_joetilemap4->mark_tile_dirty(offset);
}

WRITE16_MEMBER(ashnojoe_state::ashnojoe_tileram5_w)
{

	m_tileram_5[offset] = data;
	m_joetilemap5->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(ashnojoe_state::ashnojoe_tileram6_w)
{

	m_tileram_6[offset] = data;
	m_joetilemap6->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(ashnojoe_state::ashnojoe_tileram7_w)
{

	m_tileram_7[offset] = data;
	m_joetilemap7->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(ashnojoe_state::joe_tilemaps_xscroll_w)
{

	switch( offset )
	{
	case 0:
		m_joetilemap3->set_scrollx(0, data);
		break;
	case 1:
		m_joetilemap5->set_scrollx(0, data);
		break;
	case 2:
		m_joetilemap2->set_scrollx(0, data);
		break;
	case 3:
		m_joetilemap4->set_scrollx(0, data);
		break;
	case 4:
		m_joetilemap6->set_scrollx(0, data);
		m_joetilemap7->set_scrollx(0, data);
		break;
	}
}

WRITE16_MEMBER(ashnojoe_state::joe_tilemaps_yscroll_w)
{

	switch( offset )
	{
	case 0:
		m_joetilemap3->set_scrolly(0, data);
		break;
	case 1:
		m_joetilemap5->set_scrolly(0, data);
		break;
	case 2:
		m_joetilemap2->set_scrolly(0, data);
		break;
	case 3:
		m_joetilemap4->set_scrolly(0, data);
		break;
	case 4:
		m_joetilemap6->set_scrolly(0, data);
		m_joetilemap7->set_scrolly(0, data);
		break;
	}
}

VIDEO_START( ashnojoe )
{
	ashnojoe_state *state = machine.driver_data<ashnojoe_state>();

	state->m_joetilemap = tilemap_create(machine, get_joe_tile_info,  tilemap_scan_rows, 8, 8, 64, 32);
	state->m_joetilemap2 = tilemap_create(machine, get_joe_tile_info_2, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_joetilemap3 = tilemap_create(machine, get_joe_tile_info_3, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_joetilemap4 = tilemap_create(machine, get_joe_tile_info_4, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_joetilemap5 = tilemap_create(machine, get_joe_tile_info_5, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_joetilemap6 = tilemap_create(machine, get_joe_tile_info_6, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_joetilemap7 = tilemap_create(machine, get_joe_tile_info_7, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_joetilemap->set_transparent_pen(15);
	state->m_joetilemap2->set_transparent_pen(15);
	state->m_joetilemap3->set_transparent_pen(15);
	state->m_joetilemap4->set_transparent_pen(15);
	state->m_joetilemap5->set_transparent_pen(15);
}

SCREEN_UPDATE_IND16( ashnojoe )
{
	ashnojoe_state *state = screen.machine().driver_data<ashnojoe_state>();

	//state->m_tilemap_reg[0] & 0x10 // ?? on coin insertion

	flip_screen_set(screen.machine(), state->m_tilemap_reg[0] & 1);

	if(state->m_tilemap_reg[0] & 0x02)
		state->m_joetilemap7->draw(bitmap, cliprect, 0, 0);
	else
		state->m_joetilemap6->draw(bitmap, cliprect, 0, 0);

	state->m_joetilemap4->draw(bitmap, cliprect, 0, 0);
	state->m_joetilemap2->draw(bitmap, cliprect, 0, 0);
	state->m_joetilemap5->draw(bitmap, cliprect, 0, 0);
	state->m_joetilemap3->draw(bitmap, cliprect, 0, 0);
	state->m_joetilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}
