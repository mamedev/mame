/**
 * @file video/djboy.c
 *
 * video hardware for DJ Boy
 */
#include "emu.h"
#include "video/kan_pand.h"
#include "includes/djboy.h"

WRITE8_MEMBER(djboy_state::djboy_scrollx_w)
{
	m_scrollx = data;
}

WRITE8_MEMBER(djboy_state::djboy_scrolly_w)
{
	m_scrolly = data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	djboy_state *state = machine.driver_data<djboy_state>();
	UINT8 attr = state->m_videoram[tile_index + 0x800];
	int code = state->m_videoram[tile_index] + (attr & 0xf) * 256;
	int color = attr >> 4;

	if (color & 8)
		code |= 0x1000;

	SET_TILE_INFO(1, code, color, 0);	/* no flip */
}

WRITE8_MEMBER(djboy_state::djboy_videoram_w)
{

	m_videoram[offset] = data;
	m_background->mark_tile_dirty(offset & 0x7ff);
}

VIDEO_START( djboy )
{
	djboy_state *state = machine.driver_data<djboy_state>();
	state->m_background = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
}

WRITE8_MEMBER(djboy_state::djboy_paletteram_w)
{
	int val;

	m_paletteram[offset] = data;
	offset &= ~1;
	val = (m_paletteram[offset] << 8) | m_paletteram[offset + 1];

	palette_set_color_rgb(machine(), offset / 2, pal4bit(val >> 8), pal4bit(val >> 4), pal4bit(val >> 0));
}

SCREEN_UPDATE_IND16( djboy )
{
	/**
     * xx------ msb x
     * --x----- msb y
     * ---x---- flipscreen?
     * ----xxxx ROM bank
     */
	djboy_state *state = screen.machine().driver_data<djboy_state>();
	int scroll;

	scroll = state->m_scrollx | ((state->m_videoreg & 0xc0) << 2);
	state->m_background->set_scrollx(0, scroll - 0x391);

	scroll = state->m_scrolly | ((state->m_videoreg & 0x20) << 3);
	state->m_background->set_scrolly(0, scroll);

	state->m_background->draw(bitmap, cliprect, 0, 0);
	pandora_update(state->m_pandora, bitmap, cliprect);

	return 0;
}

SCREEN_VBLANK( djboy )
{
	// rising edge
	if (vblank_on)
	{
		djboy_state *state = screen.machine().driver_data<djboy_state>();
		pandora_eof(state->m_pandora);
	}
}
