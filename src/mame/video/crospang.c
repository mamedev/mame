/*

  Cross Pang
  video hardware emulation

 -- this seems to be the same as the tumblepop bootleg based hardware
    in tumbleb.c


*/

#include "emu.h"
#include "includes/crospang.h"
#include "video/decospr.h"

WRITE16_MEMBER(crospang_state::bestri_tilebank_w)
{

	m_bestri_tilebank = (data>>10) & 0xf;
	//printf("bestri %04x\n", data);

	m_fg_layer->mark_all_dirty();
	m_bg_layer->mark_all_dirty();
}


WRITE16_MEMBER(crospang_state::bestri_bg_scrolly_w)
{

	/* Very Strange */
	int scroll =  (data & 0x3ff) ^ 0x0155;
	m_bg_layer->set_scrolly(0, -scroll + 7);
}

WRITE16_MEMBER(crospang_state::bestri_fg_scrolly_w)
{

	/* Very Strange */
	int scroll = (data & 0x3ff) ^ 0x00ab;
	m_fg_layer->set_scrolly(0, -scroll + 7);
}

WRITE16_MEMBER(crospang_state::bestri_fg_scrollx_w)
{

	// printf("fg_layer x %04x\n",data);
	m_fg_layer->set_scrollx(0, data + 32);
}

WRITE16_MEMBER(crospang_state::bestri_bg_scrollx_w)
{

	// printf("bg_layer x %04x\n",data);
	m_bg_layer->set_scrollx(0, data - 60);
}


WRITE16_MEMBER(crospang_state::crospang_fg_scrolly_w)
{
	m_fg_layer->set_scrolly(0, data + 8);
}

WRITE16_MEMBER(crospang_state::crospang_bg_scrolly_w)
{
	m_bg_layer->set_scrolly(0, data + 8);
}

WRITE16_MEMBER(crospang_state::crospang_fg_scrollx_w)
{
	m_fg_layer->set_scrollx(0, data);
}

WRITE16_MEMBER(crospang_state::crospang_bg_scrollx_w)
{
	m_bg_layer->set_scrollx(0, data + 4);
}


WRITE16_MEMBER(crospang_state::crospang_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(crospang_state::crospang_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	crospang_state *state = machine.driver_data<crospang_state>();
	int data  = state->m_bg_videoram[tile_index];
	int tile  = data & 0xfff;
	int color = (data >> 12) & 0x0f;

	SET_TILE_INFO(1, tile + state->m_bestri_tilebank * 0x1000, color + 0x20, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	crospang_state *state = machine.driver_data<crospang_state>();
	int data  = state->m_fg_videoram[tile_index];
	int tile  = data & 0xfff;
	int color = (data >> 12) & 0x0f;

	SET_TILE_INFO(1, tile + state->m_bestri_tilebank * 0x1000, color + 0x10, 0);
}


VIDEO_START( crospang )
{
	crospang_state *state = machine.driver_data<crospang_state>();
	state->m_bg_layer = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_fg_layer = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_fg_layer->set_transparent_pen(0);
}

SCREEN_UPDATE_IND16( crospang )
{
	crospang_state *state = screen.machine().driver_data<crospang_state>();
	state->m_bg_layer->draw(bitmap, cliprect, 0, 0);
	state->m_fg_layer->draw(bitmap, cliprect, 0, 0);
	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, 0x400);
	return 0;
}
